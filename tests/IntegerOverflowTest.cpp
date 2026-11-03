/**
 * @file IntegerOverflowTest.cpp
 * @brief 정수 오버플로 정책 테스트
 *
 * 기본 산술은 int64 범위를 넘으면 시도 블록으로 잡을 수 있는 실행 오류가 되고,
 * 감싸더하기·감싸빼기·감싸곱하기는 2의 보수로 감싸 돈다. 평가기와 VM(상수 접기 포함)이 같은 결과를 내는지 본다.
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"
#include "common/CheckedInt.h"

#include <limits>

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
    EXPECT_TRUE(compiler.compile(program.get(), &chunk));
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

/// 식을 시도 블록에서 계산해, 값이면 그 값을, 오류면 "오류" 를 기록한다
std::string tryExpression(const std::string& prelude, const std::string& expression)
{
    return runBoth(prelude + "\n기록 = \"\"\n시도 { 기록 = 문자열(" + expression + ") } 오류 (e) { 기록 = \"오류\" }\n");
}

const std::string kMax = "정수 최대 = 9223372036854775807\n정수 최소 = -최대 - 1";

} // namespace

TEST(IntegerOverflowTest, CheckedHelpersDetectEveryBoundary)
{
    constexpr int64_t max = std::numeric_limits<int64_t>::max();
    constexpr int64_t min = std::numeric_limits<int64_t>::min();
    int64_t r = 0;
    EXPECT_FALSE(common::checkedAdd(max, 1, r));
    EXPECT_FALSE(common::checkedAdd(min, -1, r));
    EXPECT_TRUE(common::checkedAdd(max, min, r));
    EXPECT_EQ(r, -1);
    EXPECT_FALSE(common::checkedSub(min, 1, r));
    EXPECT_FALSE(common::checkedSub(0, min, r));
    EXPECT_FALSE(common::checkedMul(min, -1, r));
    EXPECT_FALSE(common::checkedMul(int64_t{1} << 32, int64_t{1} << 31, r));
    EXPECT_TRUE(common::checkedMul(-(int64_t{1} << 32), int64_t{1} << 31, r));
    EXPECT_EQ(r, min);
    EXPECT_TRUE(common::negationOverflows(min));
    EXPECT_FALSE(common::negationOverflows(-max));
    EXPECT_EQ(common::wrappingAdd(max, 1), min);
    EXPECT_EQ(common::wrappingSub(min, 1), max);
    EXPECT_EQ(common::wrappingMul(max, 2), -2);
    EXPECT_FALSE(common::fitsInInt64(9223372036854775808.0));
    EXPECT_TRUE(common::fitsInInt64(-9223372036854775808.0));
    EXPECT_FALSE(common::fitsInInt64(std::numeric_limits<double>::quiet_NaN()));
}

TEST(IntegerOverflowTest, ArithmeticOverflowIsCatchableError)
{
    EXPECT_EQ(tryExpression(kMax, "최대 + 1"), "오류");
    EXPECT_EQ(tryExpression(kMax, "최소 - 1"), "오류");
    EXPECT_EQ(tryExpression(kMax, "최대 * 2"), "오류");
    EXPECT_EQ(tryExpression(kMax, "최소 / -1"), "오류");
    EXPECT_EQ(tryExpression(kMax, "-최소"), "오류");
    // 경계 바로 안쪽은 그대로 계산된다
    EXPECT_EQ(tryExpression(kMax, "최대 - 1 + 1"), "9223372036854775807");
    EXPECT_EQ(tryExpression(kMax, "최소 % -1"), "0");
    EXPECT_EQ(tryExpression(kMax, "최소 / 1"), "-9223372036854775808");
}

TEST(IntegerOverflowTest, ConstantFoldingDefersOverflowToRuntime)
{
    // 리터럴끼리의 식은 VM 컴파일러가 접는다. 넘치는 식은 접지 않아 실행 시점에 같은 오류가 난다
    EXPECT_EQ(tryExpression("", "9223372036854775807 + 1"), "오류");
    EXPECT_EQ(tryExpression("", "4611686018427387904 * 2"), "오류");
    EXPECT_EQ(tryExpression("", "4611686018427387904 * -2"), "-9223372036854775808");
    EXPECT_EQ(tryExpression("", "7 / -1"), "-7");
}

TEST(IntegerOverflowTest, WrappingBuiltinsUseTwosComplement)
{
    EXPECT_EQ(tryExpression(kMax, "감싸더하기(최대, 1)"), "-9223372036854775808");
    EXPECT_EQ(tryExpression(kMax, "감싸빼기(최소, 1)"), "9223372036854775807");
    EXPECT_EQ(tryExpression(kMax, "감싸곱하기(최대, 최대)"), "1");
    EXPECT_EQ(tryExpression("", "감싸더하기(2, 3)"), "5");
    EXPECT_EQ(tryExpression("", "감싸더하기(1, 2.0)"), "오류");
    EXPECT_EQ(tryExpression("", "감싸곱하기(1)"), "오류");
}

TEST(IntegerOverflowTest, MathBuiltinsRejectOutOfRangeResults)
{
    EXPECT_EQ(tryExpression(kMax, "절대값(최소)"), "오류");
    EXPECT_EQ(tryExpression(kMax, "절대값(-최대)"), "9223372036854775807");
    EXPECT_EQ(tryExpression("", "제곱(2, 62)"), "4611686018427387904");
    EXPECT_EQ(tryExpression("", "제곱(-2, 63)"), "-9223372036854775808");
    EXPECT_EQ(tryExpression("", "제곱(2, 63)"), "오류");
    // 2^53 을 넘어도 정확하다 (double 로 계산하면 끝자리가 틀린다)
    EXPECT_EQ(tryExpression("", "제곱(3, 39)"), "4052555153018976267");
    EXPECT_EQ(tryExpression("", "제곱(2, -1)"), "0.5");
    EXPECT_EQ(tryExpression("", "정수(100000000000000000000.0)"), "오류");
    EXPECT_EQ(tryExpression("", "내림(-100000000000000000000.0)"), "오류");
    EXPECT_EQ(tryExpression("", "정수(-3.7)"), "-3");
}

TEST(IntegerOverflowTest, RangeLoopEndsAtIntegerLimits)
{
    // 끝 값이 정수 최댓값·최솟값이어도 마지막 회차 뒤 반복 변수를 늘리다 넘치지 않고 끝난다
    EXPECT_EQ(runBoth(kMax + "\n기록 = 0\ni가 최대 - 2부터 최대까지 { 기록 = 기록 + 1 }\n"), "3");
    EXPECT_EQ(runBoth(kMax + "\n기록 = 0\ni가 최대 - 1부터 최대 이하 { 기록 = 기록 + 1 }\n"), "2");
    EXPECT_EQ(runBoth(kMax + "\n기록 = 0\ni가 최소 + 2부터 최소 이상 { 기록 = 기록 + 1 }\n"), "3");
    EXPECT_EQ(runBoth(kMax + "\n기록 = 0\ni가 최대 - 2부터 최대 미만 { 기록 = 기록 + 1 }\n"), "2");
    EXPECT_EQ(runBoth(kMax + "\n기록 = 0\ni가 최소부터 최소까지 { 기록 = 기록 + 1 }\n"), "1");
}
