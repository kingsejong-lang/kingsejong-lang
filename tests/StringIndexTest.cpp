/**
 * @file StringIndexTest.cpp
 * @brief 문자열 글자 접근(s[i], s[a부터 b까지])과 문자코드·코드문자 테스트
 *
 * 글자는 UTF-8 코드 포인트 단위로 센다. 평가기와 VM 이 같은 결과를 내는지 본다.
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "semantic/SemanticAnalyzer.h"
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"
#include "common/UTF8Utils.h"

using namespace kingsejong;

namespace {

/// 평가기로 돌리고 전역 변수 '기록' 을 문자열로 돌려준다
std::string runEvaluator(const std::string& source)
{
    evaluator::Builtin::registerAllBuiltins();
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty()) << (parser.errors().empty() ? "" : parser.errors()[0]);
    auto env = std::make_shared<evaluator::Environment>();
    // 테스트가 env 를 쥐고 있어 평가기 소멸자가 함수-환경 순환 참조를 끊지 못하므로 직접 비운다 (예외로 끝나도)
    struct ClearOnExit
    {
        std::shared_ptr<evaluator::Environment> env;
        ~ClearOnExit() { env->clear(); }
    } clearOnExit{env};
    {
        evaluator::Evaluator evaluator(env);
        evaluator.evalProgram(program.get());
    }
    return env->get("기록").toString();
}

/// VM 으로 돌리고 전역 변수 '기록' 을 문자열로 돌려준다
std::string runVm(const std::string& source)
{
    evaluator::Builtin::registerAllBuiltins();
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty());
    bytecode::Chunk chunk;
    bytecode::Compiler compiler;
    if (!compiler.compile(program.get(), &chunk))
    {
        return "COMPILE_ERROR: " + (compiler.errors().empty() ? std::string() : compiler.errors()[0]);
    }
    bytecode::VM vm;
    vm.run(&chunk);
    return vm.globals()->get("기록").toString();
}

/// 평가기와 VM 이 같은 결과를 내는지 확인하고 그 결과를 돌려준다
std::string runBoth(const std::string& source)
{
    std::string evaluated = runEvaluator(source);
    EXPECT_EQ(runVm(source), evaluated) << source;
    return evaluated;
}

/// 식 하나를 시도 블록 안에서 평가해 결과를, 오류면 "오류" 를 기록한다
std::string tryExpression(const std::string& setup, const std::string& expression)
{
    return runBoth(setup + "\n기록 = \"\"\n시도 {\n    기록 = 문자열(" + expression +
                   ")\n} 오류 (e) {\n    기록 = \"오류\"\n}\n");
}

std::vector<std::string> semanticErrors(const std::string& source)
{
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    semantic::SemanticAnalyzer analyzer;
    analyzer.analyze(program.get());
    std::vector<std::string> messages;
    for (const auto& e : analyzer.errors())
    {
        messages.push_back(e.message);
    }
    return messages;
}

const std::string kWord = "s = \"안녕abc\"";

} // namespace

TEST(StringIndexTest, ShouldIndexByCharacter)
{
    EXPECT_EQ(tryExpression(kWord, "s[0]"), "안");
    EXPECT_EQ(tryExpression(kWord, "s[1]"), "녕");
    EXPECT_EQ(tryExpression(kWord, "s[2]"), "a");
    EXPECT_EQ(tryExpression(kWord, "s[4]"), "c");
    EXPECT_EQ(tryExpression("", "\"😀x\"[1]"), "x");  // 4바이트 글자도 한 글자다
}

TEST(StringIndexTest, ShouldCountNegativeIndexFromTheEnd)
{
    EXPECT_EQ(tryExpression(kWord, "s[-1]"), "c");
    EXPECT_EQ(tryExpression(kWord, "s[-5]"), "안");
}

TEST(StringIndexTest, ShouldRaiseCatchableErrorOutOfRange)
{
    EXPECT_EQ(tryExpression(kWord, "s[5]"), "오류");
    EXPECT_EQ(tryExpression(kWord, "s[-6]"), "오류");
    EXPECT_EQ(tryExpression("", "\"\"[0]"), "오류");
    EXPECT_EQ(tryExpression(kWord, "s[\"a\"]"), "오류");
    EXPECT_EQ(tryExpression(kWord, "s[1.0]"), "오류");
}

TEST(StringIndexTest, ShouldNamePositionAndLengthInError)
{
    std::string message = runBoth(kWord + "\n기록 = \"\"\n시도 { 출력(s[9]) } 오류 (e) { 기록 = 문자열(e) }\n");
    EXPECT_NE(message.find("인덱스 9"), std::string::npos) << message;
    EXPECT_NE(message.find("글자 수 5"), std::string::npos) << message;
}

TEST(StringIndexTest, ShouldKeepStringsImmutable)
{
    EXPECT_EQ(runBoth(kWord + "\n기록 = \"\"\n시도 { s[0] = \"x\" } 오류 (e) { 기록 = \"오류\" }\n"), "오류");
}

TEST(StringIndexTest, ShouldSliceByCharacter)
{
    EXPECT_EQ(tryExpression(kWord, "s[0부터 1까지]"), "안녕");
    EXPECT_EQ(tryExpression(kWord, "s[2부터 -1까지]"), "abc");
    EXPECT_EQ(tryExpression(kWord, "s[0부터 5 미만 2씩]"), "안ac");
    EXPECT_EQ(tryExpression(kWord, "s[3부터 1까지]"), "");
    EXPECT_EQ(tryExpression(kWord, "s[0부터 100까지]"), "안녕abc");  // 배열처럼 끝을 넘으면 잘라 낸다
}

TEST(StringIndexTest, ShouldConvertBetweenCharacterAndCodePoint)
{
    EXPECT_EQ(tryExpression("", "문자코드(\"A\")"), "65");
    EXPECT_EQ(tryExpression("", "문자코드(\"가\")"), "44032");
    EXPECT_EQ(tryExpression("", "문자코드(\"😀\")"), "128512");
    EXPECT_EQ(tryExpression("", "코드문자(65)"), "A");
    EXPECT_EQ(tryExpression("", "코드문자(44032)"), "가");
    EXPECT_EQ(tryExpression("", "코드문자(128512)"), "😀");
    EXPECT_EQ(tryExpression("", "코드문자(문자코드(\"a\") - 32)"), "A");
}

TEST(StringIndexTest, ShouldRejectInvalidCodeConversions)
{
    EXPECT_EQ(tryExpression("", "문자코드(\"ab\")"), "오류");
    EXPECT_EQ(tryExpression("", "문자코드(\"\")"), "오류");
    EXPECT_EQ(tryExpression("", "문자코드(65)"), "오류");
    EXPECT_EQ(tryExpression("", "코드문자(-1)"), "오류");
    EXPECT_EQ(tryExpression("", "코드문자(1114112)"), "오류");
    EXPECT_EQ(tryExpression("", "코드문자(55296)"), "오류");  // 서로게이트
    EXPECT_EQ(tryExpression("", "코드문자(65.0)"), "오류");
}

TEST(StringIndexTest, ShouldScanStringWithIndexLoop)
{
    // atoi: 숫자 글자를 하나씩 읽어 정수로 만든다
    std::string source =
        "s = \"12345\"\n"
        "값 = 0\n"
        "i가 0부터 길이(s) 미만 {\n"
        "    값 = 값 * 10 + (문자코드(s[i]) - 문자코드(\"0\"))\n"
        "}\n"
        "기록 = 값\n";
    EXPECT_EQ(runBoth(source), "12345");
}

TEST(StringIndexTest, ShouldPassSemanticAnalysis)
{
    EXPECT_TRUE(semanticErrors("s = \"안녕\"\n출력(s[0], s[0부터 1까지], 문자코드(s[0]), 코드문자(65))\n").empty());
    // 문자코드는 정수를 돌려주므로 조건 자리에서 논리 값이 아니라고 알린다
    EXPECT_FALSE(semanticErrors("만약 (문자코드(\"a\")) { 출력(1) }\n").empty());
}

TEST(UTF8UtilsTest, ShouldDecodeOnlySingleWellFormedCharacter)
{
    char32_t cp = 0;
    EXPECT_TRUE(common::decodeSingleUTF8Char("가", cp));
    EXPECT_EQ(static_cast<uint32_t>(cp), 0xAC00u);
    EXPECT_FALSE(common::decodeSingleUTF8Char("가나", cp));
    EXPECT_FALSE(common::decodeSingleUTF8Char(std::string("\xC0\x80"), cp));          // overlong NUL
    EXPECT_FALSE(common::decodeSingleUTF8Char(std::string("\xED\xA0\x80"), cp));      // 서로게이트 D800
    EXPECT_FALSE(common::decodeSingleUTF8Char(std::string("\xF4\x90\x80\x80"), cp));  // U+110000
    EXPECT_FALSE(common::decodeSingleUTF8Char(std::string("\x80"), cp));              // 연속 바이트만
    EXPECT_FALSE(common::decodeSingleUTF8Char(std::string("\xEA\xB0"), cp));          // 잘린 글자
}

TEST(UTF8UtilsTest, ShouldRoundTripEncoding)
{
    for (char32_t cp : {0x0u, 0x41u, 0x7Fu, 0x80u, 0x7FFu, 0x800u, 0xAC00u, 0xFFFFu, 0x10000u, 0x10FFFFu})
    {
        char32_t decoded = 0xFFFFFFFF;
        ASSERT_TRUE(common::decodeSingleUTF8Char(common::encodeUTF8(cp), decoded)) << static_cast<uint32_t>(cp);
        EXPECT_EQ(static_cast<uint32_t>(decoded), static_cast<uint32_t>(cp));
    }
}
