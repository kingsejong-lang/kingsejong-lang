/**
 * @file NumericFormatTest.cpp
 * @brief 실수 출력 형식, 문자열→수 변환, 실수 나눗셈·나머지 테스트
 *
 * - 실수는 되읽으면 같은 값이 되는 가장 짧은 자릿수로 쓰고, 정수 값이면 ".0" 을 붙인다 (Python 의 repr).
 *   출력·문자열()·문자열 보간이 같은 규칙을 쓴다 (전에는 %g 6자리와 소수 6자리 고정이 섞여 있었다).
 * - 정수()·실수() 는 앞뒤 공백만 허용하고 남는 글자가 있으면 오류다 (전에는 "12abc" 가 12 였다).
 * - 실수 나눗셈은 정확히 0 일 때만 오류다 (전에는 |나누는 수| < 1e-9 이면 오류). 실수 % 는 fmod 다.
 * - 실수 == 는 정확히 비교한다 (전에는 차가 1e-9 미만이면 같았고, VM 의 리터럴 상수 접기만 정확히 비교했다).
 *   오차를 허용하는 확인은 assert_근사 로 한다.
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "bytecode/Compiler.h"
#include "bytecode/VM.h"
#include "common/FloatFormat.h"

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

} // namespace

TEST(NumericFormatTest, ShouldFormatFloatsLikePythonRepr)
{
    using common::formatFloat;
    EXPECT_EQ(formatFloat(0.1 + 0.2), "0.30000000000000004");
    EXPECT_EQ(formatFloat(1.0 / 3), "0.3333333333333333");
    EXPECT_EQ(formatFloat(3.0), "3.0");
    EXPECT_EQ(formatFloat(-2.5), "-2.5");
    EXPECT_EQ(formatFloat(123456789.0), "123456789.0");
    EXPECT_EQ(formatFloat(1e15), "1000000000000000.0");
    EXPECT_EQ(formatFloat(1e16), "1e+16");
    EXPECT_EQ(formatFloat(12345678901234567.0), "1.2345678901234568e+16");
    EXPECT_EQ(formatFloat(0.0001), "0.0001");
    EXPECT_EQ(formatFloat(1e-5), "1e-05");
    EXPECT_EQ(formatFloat(1.5e-7), "1.5e-07");
    EXPECT_EQ(formatFloat(5e-324), "5e-324");
    EXPECT_EQ(formatFloat(1.7976931348623157e308), "1.7976931348623157e+308");
    EXPECT_EQ(formatFloat(0.0), "0.0");
    EXPECT_EQ(formatFloat(-0.0), "-0.0");
    EXPECT_EQ(formatFloat(1e300 * 1e10), "inf");
    EXPECT_EQ(formatFloat(-1e300 * 1e10), "-inf");
}

TEST(NumericFormatTest, ShouldUseOneFormatEverywhere)
{
    // 출력할 때·문자열()·보간·배열 안이 모두 같다
    EXPECT_EQ(runBoth("x = 0.5\n기록 = [\"${x}\", 문자열(x), 문자열([x, 3.0])]\n"), "[0.5, 0.5, [0.5, 3.0]]");
    EXPECT_EQ(runBoth("기록 = 문자열(실수(3)) + \" \" + 문자열(0.1 + 0.2)\n"), "3.0 0.30000000000000004");
}

TEST(NumericFormatTest, ShouldRejectTrailingGarbageInConversions)
{
    EXPECT_EQ(tryExpression("", "정수(\"12abc\")"), "오류");
    EXPECT_EQ(tryExpression("", "정수(\"1e400\")"), "오류");
    EXPECT_EQ(tryExpression("", "정수(\"0x10\")"), "오류");
    EXPECT_EQ(tryExpression("", "정수(\"9223372036854775808\")"), "오류");
    EXPECT_EQ(tryExpression("", "정수(\"\")"), "오류");
    EXPECT_EQ(tryExpression("", "실수(\"3.5kg\")"), "오류");
    EXPECT_EQ(tryExpression("", "실수(\"1e\")"), "오류");
    EXPECT_EQ(tryExpression("", "실수(\"0x10\")"), "오류");
    EXPECT_EQ(tryExpression("", "실수(\"1e400\")"), "오류");
}

TEST(NumericFormatTest, ShouldAcceptWholeNumbersWithSpaces)
{
    EXPECT_EQ(tryExpression("", "정수(\" 7 \")"), "7");
    EXPECT_EQ(tryExpression("", "정수(\"+5\")"), "5");
    EXPECT_EQ(tryExpression("", "정수(\"-9223372036854775808\")"), "-9223372036854775808");
    // 실수 모양은 실수로 읽어 소수점 아래를 버린다 (문서의 정수("3.7") → 3)
    EXPECT_EQ(tryExpression("", "정수(\"3.7\")"), "3");
    EXPECT_EQ(tryExpression("", "정수(\"-2.5\")"), "-2");
    EXPECT_EQ(tryExpression("", "정수(\"1e3\")"), "1000");
    EXPECT_EQ(tryExpression("", "실수(\" 2.5 \")"), "2.5");
    EXPECT_EQ(tryExpression("", "실수(\"5e-324\")"), "5e-324");
    EXPECT_EQ(tryExpression("", "실수(\"inf\")"), "inf");
}

TEST(NumericFormatTest, ShouldDivideBySmallFloats)
{
    // 상수 접기(VM)와 실행 중 계산이 같은 결과를 낸다
    EXPECT_EQ(tryExpression("a = 1e-10", "1.0 / a"), "10000000000.0");
    EXPECT_EQ(tryExpression("", "1.0 / 1e-10"), "10000000000.0");
    EXPECT_EQ(tryExpression("z = 0.0", "1.0 / z"), "오류");
    EXPECT_EQ(tryExpression("z = 0", "1.5 / z"), "오류");
}

TEST(NumericFormatTest, ShouldComputeFloatModuloLikeFmod)
{
    EXPECT_EQ(tryExpression("", "-7.5 % 2"), "-1.5");
    EXPECT_EQ(tryExpression("", "7.5 % -2"), "1.5");
    EXPECT_EQ(tryExpression("x = 5", "x % 2.5"), "0.0");
    EXPECT_EQ(tryExpression("z = 0.0", "1.5 % z"), "오류");
}

TEST(NumericFormatTest, ShouldParseSubnormalLiterals)
{
    EXPECT_EQ(runBoth("기록 = [5e-324 > 0, 1e-400 == 0]\n"), "[참, 참]");
}

TEST(NumericFormatTest, ShouldCompareFloatsExactly)
{
    // 변수로 비교(실행 시점)와 리터럴끼리 비교(VM 상수 접기)가 같은 규칙이다
    EXPECT_EQ(runBoth("a = 0.1 + 0.2\n기록 = [a == 0.3, a != 0.3, 0.1 + 0.2 == 0.3]\n"), "[거짓, 참, 거짓]");
    EXPECT_EQ(runBoth("a = 1e-300\nb = 2e-300\n기록 = [a == b, 1e-300 == 2e-300, a < b]\n"), "[거짓, 거짓, 참]");
    EXPECT_EQ(runBoth("a = 1.0\nb = 1.0000000000000002\n기록 = [a == b, a == 1.0]\n"), "[거짓, 참]");
    // 무한대는 자기 자신과 같고, NaN 은 자기 자신과도 다르다
    EXPECT_EQ(runBoth("i = 1e308 * 10.0\n기록 = [i == i, i == -i, i != i]\n"), "[참, 거짓, 거짓]");
    EXPECT_EQ(runBoth("i = 1e308 * 10.0\nn = i - i\n기록 = [n == n, n != n]\n"), "[거짓, 참]");
    // 정수와 실수는 수의 크기로 비교한다
    EXPECT_EQ(runBoth("기록 = [1 == 1.0, 9007199254740993 == 9007199254740992.0, 2 == 2.0000000001]\n"), "[참, 참, 거짓]");
    EXPECT_EQ(runBoth("기록 = [[0.5, 1.5] == [0.5, 1.5], {\"a\": 0.1 + 0.2} == {\"a\": 0.3}]\n"), "[참, 거짓]");
}

TEST(NumericFormatTest, ShouldAssertFloatsExactlyOrApproximately)
{
    EXPECT_EQ(tryExpression("", "assert_같음(0.1 + 0.2, 0.3)"), "오류");
    EXPECT_EQ(tryExpression("", "assert_같음(1e-300, 2e-300)"), "오류");
    EXPECT_EQ(tryExpression("i = 1e308 * 10.0", "assert_같음(i, i)"), "없음");
    EXPECT_EQ(tryExpression("", "assert_근사(0.1 + 0.2, 0.3)"), "없음");
    EXPECT_EQ(tryExpression("", "assert_근사(1e20 + 1e5, 1e20)"), "없음");
    EXPECT_EQ(tryExpression("", "assert_근사(1.001, 1.0)"), "오류");
    EXPECT_EQ(tryExpression("", "assert_근사(1.001, 1.0, 0.01)"), "없음");
    EXPECT_EQ(tryExpression("", "assert_근사(3, 3.0)"), "없음");
    EXPECT_EQ(tryExpression("i = 1e308 * 10.0", "assert_근사(i, i)"), "없음");
    EXPECT_EQ(tryExpression("i = 1e308 * 10.0", "assert_근사(i, 1e308)"), "오류");
    EXPECT_EQ(tryExpression("i = 1e308 * 10.0", "assert_근사(i - i, i - i)"), "오류");
    EXPECT_EQ(tryExpression("", "assert_근사(\"1\", 1)"), "오류");
    EXPECT_EQ(tryExpression("", "assert_근사(1.0, 1.0, -1)"), "오류");
}
