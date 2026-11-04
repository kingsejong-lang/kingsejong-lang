/**
 * @file ExecutionLimitsTest.cpp
 * @brief 평가기·VM 이 common::ExecutionLimits::current() 의 한도를 따르는지 테스트
 *
 * CLI 가 파일을 실행할 때 한도를 끄고 --sandbox 에서 켜는 것은 CMakeLists.txt 의 cli-limits-* 테스트가 본다.
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"
#include "common/ExecutionLimits.h"

using namespace kingsejong;

namespace {

const char* kLoop = "i = 0\n반복 (i < 5000) {\n    i = i + 1\n}\n";

/// 테스트가 바꾼 한도를 끝날 때 되돌린다
struct RestoreLimits
{
    common::ExecutionLimits saved = common::ExecutionLimits::current();
    ~RestoreLimits() { common::ExecutionLimits::current() = saved; }
};

/// 평가기로 돌려 실행 오류 메시지를, 끝까지 돌면 "" 를 돌려준다
std::string runEvaluator(const std::string& source)
{
    evaluator::Builtin::registerAllBuiltins();
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    try
    {
        evaluator::Evaluator evaluator;
        evaluator.evalProgram(program.get());
    }
    catch (const error::KingSejongError& e)
    {
        return e.what();
    }
    return "";
}

/// VM 으로 돌려 실패하면 "실패", 끝까지 돌면 "" 를 돌려준다
std::string runVm(const std::string& source)
{
    evaluator::Builtin::registerAllBuiltins();
    lexer::Lexer lexer(source);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();
    bytecode::Chunk chunk;
    bytecode::Compiler compiler;
    EXPECT_TRUE(compiler.compile(program.get(), &chunk));
    bytecode::VM vm;
    auto result = vm.run(&chunk);
    return (result == bytecode::VMResult::OK || result == bytecode::VMResult::HALT) ? "" : "실패";
}

} // namespace

TEST(ExecutionLimitsTest, ShouldKeepDefaultLimitsForEmbedders)
{
    // 플레이그라운드·REPL·테스트가 쓰는 기본값
    common::ExecutionLimits defaults;
    EXPECT_EQ(defaults.maxEvaluations, 10000000u);
    EXPECT_EQ(defaults.maxInstructions, 100000000u);
    EXPECT_EQ(defaults.maxTime.count(), 5000);
}

TEST(ExecutionLimitsTest, ShouldStopEvaluatorAtCurrentLimit)
{
    RestoreLimits restore;
    EXPECT_EQ(runEvaluator(kLoop), "");
    common::ExecutionLimits::current().maxEvaluations = 1000;
    EXPECT_NE(runEvaluator(kLoop).find("최대 평가 횟수"), std::string::npos);
}

TEST(ExecutionLimitsTest, ShouldStopVmAtCurrentLimit)
{
    RestoreLimits restore;
    EXPECT_EQ(runVm(kLoop), "");
    common::ExecutionLimits::current().maxInstructions = 1000;
    EXPECT_EQ(runVm(kLoop), "실패");
}

TEST(ExecutionLimitsTest, ShouldRunPastDefaultsWhenUnlimited)
{
    RestoreLimits restore;
    common::ExecutionLimits::current() = common::ExecutionLimits::unlimited();
    // 최댓값 한도는 횟수·시간 검사에 걸리지 않는다 (기본 한도를 넘는 긴 반복은 cli-limits-file 이 본다)
    EXPECT_EQ(runEvaluator(kLoop), "");
    EXPECT_EQ(runVm(kLoop), "");
}
