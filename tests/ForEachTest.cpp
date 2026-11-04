/**
 * @file ForEachTest.cpp
 * @brief 요소 반복문 (컬렉션 각각에 대해 변수로 { }) 테스트
 *
 * 배열은 시작할 때의 요소, 문자열은 글자, 딕셔너리는 키(정렬 순)를 돈다. 중단·계속과 회차별 범위는
 * 범위 반복과 같다. 평가기와 VM 이 같은 결과를 내는지, 파서가 반복 변수 뒤의 조사를 받침에 맞게 떼는지 본다.
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "semantic/SemanticAnalyzer.h"
#include "formatter/Formatter.h"
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"
#include "linter/Linter.h"
#include "linter/rules/EmptyBlockRule.h"

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

std::vector<std::string> parseErrors(const std::string& source)
{
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    parser.parseProgram();
    return parser.errors();
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

std::string format(const std::string& source)
{
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty());
    formatter::Formatter formatter;
    return formatter.format(program.get());
}

} // namespace

TEST(ForEachTest, ShouldIterateArrayElements)
{
    std::string source =
        "기록 = \"\"\n"
        "이름들 = [\"철수\", \"영희\", \"민수\"]\n"
        "이름들 각각에 대해 이름으로 {\n"
        "    기록 = 기록 + 이름 + \",\"\n"
        "}\n";
    EXPECT_EQ(runBoth(source), "철수,영희,민수,");
}

TEST(ForEachTest, ShouldIterateStringCharacters)
{
    std::string source =
        "기록 = []\n"
        "\"안녕a\" 각각에 대해 글자로 {\n"
        "    기록 = 기록 + [글자]\n"
        "}\n";
    EXPECT_EQ(runBoth(source), "[안, 녕, a]");
}

TEST(ForEachTest, ShouldIterateDictionaryKeysInSortedOrder)
{
    std::string source =
        "기록 = \"\"\n"
        "점수 = {\"수학\": 90, \"국어\": 80, \"영어\": 70}\n"
        "점수 각각에 대해 과목으로 {\n"
        "    기록 = 기록 + 과목 + 문자열(점수[과목]) + \" \"\n"
        "}\n";
    EXPECT_EQ(runBoth(source), "국어80 수학90 영어70 ");
}

TEST(ForEachTest, ShouldSkipEmptyCollections)
{
    EXPECT_EQ(runBoth("기록 = 0\n[] 각각에 대해 x로 {\n    기록 = 기록 + 1\n}\n"), "0");
    EXPECT_EQ(runBoth("기록 = 0\n\"\" 각각에 대해 x로 {\n    기록 = 기록 + 1\n}\n"), "0");
    // 줄 맨 앞의 { 는 블록이므로 빈 딕셔너리는 변수에 담아 쓴다
    EXPECT_EQ(runBoth("기록 = 0\n빈 = {}\n빈 각각에 대해 x로 {\n    기록 = 기록 + 1\n}\n"), "0");
}

TEST(ForEachTest, ShouldSupportBreakAndContinue)
{
    std::string source =
        "기록 = 0\n"
        "[1, 2, 3, 4, 5] 각각에 대해 n으로 {\n"
        "    만약 (n == 2) { 계속 }\n"
        "    만약 (n == 4) { 중단 }\n"
        "    기록 = 기록 + n\n"
        "}\n";
    EXPECT_EQ(runBoth(source), "4");
}

TEST(ForEachTest, ShouldNestLoops)
{
    std::string source =
        "기록 = \"\"\n"
        "[1, 2] 각각에 대해 a로 {\n"
        "    \"xy\" 각각에 대해 b로 {\n"
        "        기록 = 기록 + 문자열(a) + b + \" \"\n"
        "    }\n"
        "}\n";
    EXPECT_EQ(runBoth(source), "1x 1y 2x 2y ");
}

TEST(ForEachTest, ShouldCaptureEachIterationValueInClosures)
{
    std::string source =
        "함수들 = []\n"
        "[10, 20, 30] 각각에 대해 k로 {\n"
        "    함수들 = 함수들 + [함수() { 반환 k }]\n"
        "}\n"
        "기록 = [함수들[0](), 함수들[1](), 함수들[2]()]\n";
    EXPECT_EQ(runBoth(source), "[10, 20, 30]");
}

TEST(ForEachTest, ShouldIterateElementsPresentAtStart)
{
    // 반복 중에 배열을 바꿔도 시작할 때의 요소를 돈다 (끝나지 않거나 건너뛰지 않는다)
    std::string source =
        "목록 = [1, 2, 3]\n"
        "기록 = 0\n"
        "목록 각각에 대해 x로 {\n"
        "    목록[2] = 100\n"
        "    기록 = 기록 + x\n"
        "}\n";
    EXPECT_EQ(runBoth(source), "6");
}

TEST(ForEachTest, ShouldWorkInsideFunctionsWithReturn)
{
    std::string source =
        "함수 찾기_위치(목록, 값) {\n"
        "    위치 = 0\n"
        "    목록 각각에 대해 x로 {\n"
        "        만약 (x == 값) { 반환 위치 }\n"
        "        위치 = 위치 + 1\n"
        "    }\n"
        "    반환 -1\n"
        "}\n"
        "기록 = [찾기_위치([5, 6, 7], 7), 찾기_위치([5], 9)]\n";
    EXPECT_EQ(runBoth(source), "[2, -1]");
}

TEST(ForEachTest, ShouldRejectNonIterableValuesAtRuntime)
{
    std::string source =
        "수 = 5\n"
        "기록 = \"\"\n"
        "시도 {\n"
        "    수 각각에 대해 x로 { 기록 = \"돌았음\" }\n"
        "} 오류 (e) {\n"
        "    기록 = \"오류\"\n"
        "}\n";
    EXPECT_EQ(runBoth(source), "오류");
}

TEST(ForEachTest, ShouldScopeLoopVariableToTheLoop)
{
    EXPECT_FALSE(semanticErrors("[1] 각각에 대해 x로 {\n    출력(x)\n}\n출력(x)\n").empty());
    EXPECT_EQ(runBoth("x = 7\n[1, 2] 각각에 대해 x로 {\n    출력(x)\n}\n기록 = x\n"), "7");
}

TEST(ForEachTest, ShouldReportNonIterableLiteralInSemanticAnalysis)
{
    EXPECT_FALSE(semanticErrors("5 각각에 대해 x로 {\n    출력(x)\n}\n").empty());
    EXPECT_TRUE(semanticErrors("\"ab\" 각각에 대해 x로 {\n    출력(x)\n}\n").empty());
}

TEST(ForEachParserTest, ShouldStripJosaMatchingTheFinalConsonant)
{
    // 받침 있음 → 으로, 받침 없음·ㄹ받침 → 로, 영문은 둘 다
    EXPECT_EQ(runBoth("기록 = \"\"\n[\"a\"] 각각에 대해 사람으로 {\n    기록 = 사람\n}\n"), "a");
    EXPECT_EQ(runBoth("기록 = \"\"\n[\"a\"] 각각에 대해 경로로 {\n    기록 = 경로\n}\n"), "a");
    EXPECT_EQ(runBoth("기록 = \"\"\n[\"a\"] 각각에 대해 길로 {\n    기록 = 길\n}\n"), "a");
    EXPECT_EQ(runBoth("기록 = \"\"\n[\"a\"] 각각에 대해 item으로 {\n    기록 = item\n}\n"), "a");
    EXPECT_EQ(runBoth("기록 = \"\"\n[\"a\"] 각각에 대해 값1로 {\n    기록 = 값1\n}\n"), "a");
}

TEST(ForEachParserTest, ShouldRejectMissingOrMismatchedJosa)
{
    EXPECT_FALSE(parseErrors("[1] 각각에 대해 경로 {\n}\n").empty());   // 조사 없음 ("경로" 를 경 + 로 로 떼지 않는다)
    EXPECT_FALSE(parseErrors("[1] 각각에 대해 사람로 {\n}\n").empty()); // 받침 있는데 로
    EXPECT_FALSE(parseErrors("[1] 각각에 대해 {\n}\n").empty());        // 변수 없음
    EXPECT_FALSE(parseErrors("[1] 각각 x로 {\n}\n").empty());           // 에 대해 없음
}

TEST(ForEachParserTest, ShouldAcceptOptionalLoopWord)
{
    EXPECT_EQ(runBoth("기록 = 0\n[1, 2] 각각에 대해 n으로 반복한다 {\n    기록 = 기록 + n\n}\n"), "3");
    EXPECT_EQ(runBoth("기록 = 0\n[1, 2] 각각에 대해 n으로 반복 {\n    기록 = 기록 + n\n}\n"), "3");
}

TEST(ForEachParserTest, ShouldKeepMatchExpressionWorking)
{
    // "에 대해" 는 패턴 매칭에도 쓰인다. 각각 없이 쓰면 전처럼 패턴 매칭이다
    std::string source =
        "x = 2\n"
        "기록 = x 에 대해 {\n"
        "    1 -> \"하나\"\n"
        "    2 -> \"둘\"\n"
        "    _ -> \"그 밖\"\n"
        "}\n";
    EXPECT_EQ(runBoth(source), "둘");
}

TEST(ForEachFormatterTest, ShouldKeepJosaAndLoopWord)
{
    EXPECT_EQ(format("목록   각각에 대해 값으로  반복한다{\n출력(값)\n}\n"),
              "목록 각각에 대해 값으로 반복한다 {\n    출력(값)\n}\n");
    EXPECT_EQ(format("(a + [3]) 각각에 대해 v로 {\n출력(v)\n}\n"),
              "(a + [3]) 각각에 대해 v로 {\n    출력(v)\n}\n");
}

TEST(ForEachLinterTest, ShouldReportEmptyBody)
{
    lexer::Lexer lexer("[1] 각각에 대해 x로 {\n}\n");
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    linter::Linter linter;
    linter.addRule(std::make_unique<linter::rules::EmptyBlockRule>());
    linter.analyze(program.get(), "test.ksj");
    bool reported = false;
    for (const auto& issue : linter.issues())
    {
        reported = reported || issue.ruleId == "empty-block";
    }
    EXPECT_TRUE(reported);
}
