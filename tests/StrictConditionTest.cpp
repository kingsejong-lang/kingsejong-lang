/**
 * @file StrictConditionTest.cpp
 * @brief 조건은 참/거짓만 받는다
 *
 * 의미 분석기는 타입을 아는 식만 검사하므로, 변수에 담긴 값도 실행 중에 같은 규칙으로 막는지 본다.
 * 만약·반복 조건과 경우 가드가 대상이고, 평가기와 VM 이 같은 결과를 내야 한다.
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"

using namespace kingsejong;

namespace {

/// 평가기로 돌리고 전역 변수 '기록' 을 문자열로 돌려준다. 실행 오류는 "오류" 로 돌려준다
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
    try
    {
        evaluator::Evaluator evaluator(env);
        evaluator.evalProgram(program.get());
    }
    catch (const std::exception&)
    {
        return "오류";
    }
    return env->get("기록").toString();
}

/// VM 으로 돌리고 전역 변수 '기록' 을 문자열로 돌려준다. 실행 오류는 "오류" 로 돌려준다
std::string runVm(const std::string& source)
{
    evaluator::Builtin::registerAllBuiltins();
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    EXPECT_TRUE(parser.errors().empty());
    bytecode::Chunk chunk;
    bytecode::Compiler compiler;
    EXPECT_TRUE(compiler.compile(program.get(), &chunk));
    bytecode::VM vm;
    if (vm.run(&chunk) == bytecode::VMResult::RUNTIME_ERROR)
    {
        return "오류";
    }
    return vm.globals()->get("기록").toString();
}

/// 평가기와 VM 이 같은 결과를 내는지 확인하고 그 결과를 돌려준다
std::string runBoth(const std::string& source)
{
    std::string evaluated = runEvaluator(source);
    EXPECT_EQ(runVm(source), evaluated) << source;
    return evaluated;
}

} // namespace

TEST(StrictConditionTest, ShouldAcceptBooleanConditions)
{
    EXPECT_EQ(runBoth("기록 = 0\nx = 참\n만약 (x) { 기록 = 1 } 아니면 { 기록 = 2 }\n"), "1");
    EXPECT_EQ(runBoth("기록 = 0\n반복 (기록 < 3) { 기록 = 기록 + 1 }\n"), "3");
    EXPECT_EQ(runBoth("기록 = 0\n목록 = []\n만약 (길이(목록) > 0) { 기록 = 1 } 아니면 { 기록 = 2 }\n"), "2");
}

TEST(StrictConditionTest, ShouldRejectNonBooleanVariableInIf)
{
    // 의미 분석기가 타입을 모르는 변수도 실행 중에 막는다 (전에는 진리값 규칙으로 참이 됐다)
    EXPECT_EQ(runBoth("기록 = 0\nx = 1\n만약 (x) { 기록 = 1 }\n"), "오류");
    EXPECT_EQ(runBoth("기록 = 0\nx = \"\"\n만약 (x) { 기록 = 1 } 아니면 { 기록 = 2 }\n"), "오류");
    EXPECT_EQ(runBoth("기록 = 0\nx = 없음\n만약 (x) { 기록 = 1 } 아니면 { 기록 = 2 }\n"), "오류");
}

TEST(StrictConditionTest, ShouldRejectNonBooleanLiteralInVm)
{
    // VM 컴파일러는 정수 리터럴 조건을 접어 한쪽만 컴파일했다. 이제 평가기처럼 실행 오류가 된다
    EXPECT_EQ(runBoth("기록 = 0\n만약 (1) { 기록 = 1 }\n"), "오류");
    EXPECT_EQ(runBoth("기록 = 0\n만약 (0) { 기록 = 1 } 아니면 { 기록 = 2 }\n"), "오류");
}

TEST(StrictConditionTest, ShouldRejectNonBooleanWhileCondition)
{
    EXPECT_EQ(runBoth("기록 = 0\nn = 3\n반복 (n) { n = n - 1 }\n"), "오류");
}

TEST(StrictConditionTest, ShouldRejectNonBooleanMatchGuard)
{
    // 가드가 참/거짓이 아니면 평가기는 맞지 않은 것으로, VM 은 진리값으로 다르게 처리했다. 이제 둘 다 오류다
    EXPECT_EQ(runBoth("기록 = 5 에 대해 {\n    n when n -> \"가드\"\n    _ -> \"기타\"\n}\n"), "오류");
    EXPECT_EQ(runBoth("기록 = 5 에 대해 {\n    n when n > 3 -> \"큼\"\n    _ -> \"기타\"\n}\n"), "큼");
}

TEST(StrictConditionTest, ShouldBeCatchable)
{
    EXPECT_EQ(runBoth("기록 = \"\"\nx = 1\n시도 { 만약 (x) { 기록 = \"참\" } } 오류 (e) { 기록 = \"잡음\" }\n"), "잡음");
}
