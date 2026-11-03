/**
 * @file LoopControlTest.cpp
 * @brief 중단 / 계속 (break / continue) 테스트
 *
 * 평가기와 VM 이 같은 결과를 내는지, 시도 문의 마지막 블록이 도는지, 함수 경계를 넘지 않는지 본다.
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
#include "error/Error.h"
#include "linter/Linter.h"
#include "linter/rules/ConstantConditionRule.h"
#include "linter/rules/DeadCodeRule.h"

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
    {
        evaluator::Evaluator evaluator(env);
        evaluator.evalProgram(program.get());
    }
    return env->get("기록").toString();
}

/// VM 으로 돌리고 전역 변수 '기록' 을 문자열로 돌려준다. 컴파일이 실패하면 오류 메시지를 돌려준다
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

} // namespace

TEST(LoopControlTest, ShouldLexKeywords)
{
    lexer::Lexer lexer("중단 계속");
    EXPECT_EQ(lexer.nextToken().type, lexer::TokenType::JUNGDAN);
    EXPECT_EQ(lexer.nextToken().type, lexer::TokenType::GYESOK);
}

TEST(LoopControlTest, ShouldBreakAndContinueEveryLoopKind)
{
    EXPECT_EQ(runBoth(
        "기록 = []\n"
        "i = 0\n"
        "반복 (참) {\n"
        "    i = i + 1\n"
        "    만약 (i == 2) { 계속 }\n"
        "    만약 (i > 4) { 중단 }\n"
        "    기록 = 기록 + [i]\n"
        "}\n"), "[1, 3, 4]");

    EXPECT_EQ(runBoth(
        "기록 = []\n"
        "i가 1부터 10까지 {\n"
        "    만약 (i % 2 == 0) { 계속 }\n"
        "    만약 (i > 7) { 중단 }\n"
        "    기록 = 기록 + [i]\n"
        "}\n"), "[1, 3, 5, 7]");

    EXPECT_EQ(runBoth(
        "기록 = []\n"
        "i가 5부터 1 이상 {\n"
        "    만약 (i == 3) { 계속 }\n"
        "    기록 = 기록 + [i]\n"
        "}\n"), "[5, 4, 2, 1]");

    EXPECT_EQ(runBoth(
        "기록 = []\n"
        "n = 0\n"
        "5번 반복한다 {\n"
        "    n = n + 1\n"
        "    만약 (n == 2) { 계속 }\n"
        "    만약 (n == 4) { 중단 }\n"
        "    기록 = 기록 + [n]\n"
        "}\n"), "[1, 3]");
}

TEST(LoopControlTest, ShouldOnlyAffectInnermostLoop)
{
    EXPECT_EQ(runBoth(
        "기록 = []\n"
        "i가 0부터 3 미만 {\n"
        "    j가 0부터 3 미만 {\n"
        "        만약 (j == 1) { 중단 }\n"
        "        기록 = 기록 + [i * 10 + j]\n"
        "    }\n"
        "}\n"), "[0, 10, 20]");
}

TEST(LoopControlTest, ShouldRunFinallyWhenLeavingTryBlock)
{
    EXPECT_EQ(runBoth(
        "기록 = []\n"
        "i가 1부터 4까지 {\n"
        "    시도 {\n"
        "        만약 (i == 2) { 계속 }\n"
        "        만약 (i == 4) { 중단 }\n"
        "        기록 = 기록 + [\"시도\" + 문자열(i)]\n"
        "    } 오류 (e) {\n"
        "        기록 = 기록 + [\"오류\"]\n"
        "    } 마지막 {\n"
        "        기록 = 기록 + [\"마지막\" + 문자열(i)]\n"
        "    }\n"
        "}\n"), "[시도1, 마지막1, 마지막2, 시도3, 마지막3, 마지막4]");

    // 오류 블록 안에서 빠져나가도 마지막 블록이 돌고, 다음 회차의 시도 문이 정상 동작한다
    EXPECT_EQ(runBoth(
        "기록 = []\n"
        "i가 1부터 3까지 {\n"
        "    시도 {\n"
        "        던지다 \"x\"\n"
        "    } 오류 (e) {\n"
        "        만약 (i == 2) { 계속 }\n"
        "        만약 (i == 3) { 중단 }\n"
        "        기록 = 기록 + [\"잡음\" + 문자열(i)]\n"
        "    } 마지막 {\n"
        "        기록 = 기록 + [\"정리\" + 문자열(i)]\n"
        "    }\n"
        "}\n"), "[잡음1, 정리1, 정리2, 정리3]");
}

TEST(LoopControlTest, ShouldWorkInsideFunctions)
{
    EXPECT_EQ(runBoth(
        "찾기 = 함수(목록, 값) {\n"
        "    위치 = -1\n"
        "    i가 0부터 길이(목록) 미만 {\n"
        "        만약 (목록[i] == 값) {\n"
        "            위치 = i\n"
        "            중단\n"
        "        }\n"
        "    }\n"
        "    반환 위치\n"
        "}\n"
        "기록 = [찾기([5, 7, 9], 7), 찾기([5, 7, 9], 4)]\n"), "[1, -1]");
}

TEST(LoopControlTest, ShouldRejectOutsideLoopInSemanticAnalysis)
{
    auto top = semanticErrors("중단\n");
    ASSERT_EQ(top.size(), 1u);
    EXPECT_NE(top[0].find("반복문 안에서만"), std::string::npos);

    // 함수 본문은 바깥 반복문을 보지 않는다
    auto inFunction = semanticErrors(
        "i가 1부터 3까지 {\n"
        "    f = 함수() { 계속 }\n"
        "}\n");
    ASSERT_EQ(inFunction.size(), 1u);
    EXPECT_NE(inFunction[0].find("'계속'"), std::string::npos);

    EXPECT_TRUE(semanticErrors("i가 1부터 3까지 {\n    만약 (i == 2) { 중단 }\n}\n").empty());
}

TEST(LoopControlTest, ShouldNotLeaveFunctionAtRuntime)
{
    // 의미 분석 없이 평가해도 함수 밖의 반복문을 끝내지 않고 실행 에러가 된다
    EXPECT_THROW(runEvaluator(
        "기록 = 0\n"
        "i가 1부터 3까지 {\n"
        "    f = 함수() { 중단 }\n"
        "    f()\n"
        "}\n"), error::KingSejongError);
}

TEST(LoopControlTest, ShouldKeepAssignmentToSameName)
{
    // 키워드가 되기 전 같은 이름에 대입하던 문장은 할당문으로 읽는다
    lexer::Lexer lexer("중단 = 1\n");
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty());
    ASSERT_EQ(program->statements().size(), 1u);
    EXPECT_EQ(program->statements()[0]->type(), ast::NodeType::ASSIGNMENT_STATEMENT);
}

TEST(LoopControlTest, ShouldFormatKeywords)
{
    const std::string code =
        "i가 1부터 3까지 {\n"
        "    만약 (i == 1) {\n"
        "        계속\n"
        "    }\n"
        "    중단\n"
        "}\n";
    lexer::Lexer lexer(code);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    formatter::Formatter formatter;
    EXPECT_EQ(formatter.format(program.get(), code), code);
    EXPECT_FALSE(formatter.hasUnsupportedNodes());
}

TEST(LoopControlTest, VmShouldReportLoopControlInsideFinally)
{
    // 예외로 들어온 마지막 블록에서 빠져나가면 VM 이 보류한 예외를 정리할 수 없어 컴파일 단계에서 알린다
    std::string result = runVm(
        "기록 = 0\n"
        "i가 1부터 3까지 {\n"
        "    시도 {\n"
        "        기록 = i\n"
        "    } 마지막 {\n"
        "        중단\n"
        "    }\n"
        "}\n");
    EXPECT_NE(result.find("COMPILE_ERROR"), std::string::npos);
    EXPECT_NE(result.find("마지막 블록"), std::string::npos);

    // 평가기는 지원한다 (보류된 에러 없이 반복을 끝낸다)
    EXPECT_EQ(runEvaluator(
        "기록 = 0\n"
        "i가 1부터 3까지 {\n"
        "    시도 {\n"
        "        기록 = i\n"
        "    } 마지막 {\n"
        "        중단\n"
        "    }\n"
        "}\n"), "1");
}

namespace {
std::vector<std::string> lintMessages(const std::string& source, std::unique_ptr<linter::Rule> rule)
{
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    linter::Linter linter;
    linter.addRule(std::move(rule));
    linter.analyze(program.get(), "test.ksj");
    std::vector<std::string> messages;
    for (const auto& issue : linter.issues()) messages.push_back(issue.message);
    return messages;
}
} // namespace

TEST(LoopControlTest, LinterShouldAcceptInfiniteLoopThatBreaksOut)
{
    using linter::rules::ConstantConditionRule;
    EXPECT_TRUE(lintMessages("반복 (참) {\n    중단\n}\n", std::make_unique<ConstantConditionRule>()).empty());

    // 안쪽 반복문의 중단은 바깥 무한 루프를 끝내지 않는다
    auto inner = lintMessages("반복 (참) {\n    i가 1부터 2까지 {\n        중단\n    }\n}\n",
                              std::make_unique<ConstantConditionRule>());
    ASSERT_EQ(inner.size(), 1u);
    EXPECT_NE(inner[0].find("항상 참"), std::string::npos);
}

TEST(LoopControlTest, LinterShouldReportCodeAfterLoopControl)
{
    auto messages = lintMessages("i가 1부터 3까지 {\n    계속\n    출력(i)\n}\n",
                                 std::make_unique<linter::rules::DeadCodeRule>());
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_NE(messages[0].find("계속 문 이후"), std::string::npos);
}
