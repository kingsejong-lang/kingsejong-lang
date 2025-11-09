/**
 * @file FloatTest.cpp
 * @brief 실수 타입 기능 테스트
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include <gtest/gtest.h>
#include "../src/lexer/Lexer.h"
#include "../src/parser/Parser.h"
#include "../src/evaluator/Evaluator.h"
#include "../src/evaluator/Builtin.h"
#include "../src/evaluator/Value.h"

using namespace kingsejong;

/**
 * @brief 코드를 평가하여 Value 반환
 */
static evaluator::Value evalInput(const std::string& input)
{
    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    EXPECT_TRUE(parser.errors().empty()) << "Parser errors: " << parser.errors()[0];

    auto env = std::make_shared<evaluator::Environment>();
    evaluator::Evaluator evaluator(env);

    return evaluator.evalProgram(program.get());
}

// ============================================================================
// 실수 리터럴 테스트
// ============================================================================

TEST(FloatTest, ShouldEvaluateFloatLiteral)
{
    auto result = evalInput("3.14");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 3.14, 1e-9);
}

TEST(FloatTest, ShouldEvaluateNegativeFloat)
{
    auto result = evalInput("-2.5");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), -2.5, 1e-9);
}

TEST(FloatTest, ShouldEvaluateZeroFloat)
{
    auto result = evalInput("0.0");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 0.0, 1e-9);
}

TEST(FloatTest, ShouldEvaluateSmallFloat)
{
    auto result = evalInput("0.001");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 0.001, 1e-9);
}

TEST(FloatTest, ShouldEvaluateLargeFloat)
{
    auto result = evalInput("123456.789");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 123456.789, 1e-9);
}

// ============================================================================
// 실수 연산 테스트
// ============================================================================

TEST(FloatTest, ShouldAddFloats)
{
    auto result = evalInput("3.5 + 2.5");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 6.0, 1e-9);
}

TEST(FloatTest, ShouldSubtractFloats)
{
    auto result = evalInput("10.5 - 3.2");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 7.3, 1e-9);
}

TEST(FloatTest, ShouldMultiplyFloats)
{
    auto result = evalInput("2.5 * 4.0");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 10.0, 1e-9);
}

TEST(FloatTest, ShouldDivideFloats)
{
    auto result = evalInput("7.5 / 2.5");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 3.0, 1e-9);
}

TEST(FloatTest, ShouldHandleComplexFloatExpression)
{
    auto result = evalInput("(3.5 + 2.5) * 2.0");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 12.0, 1e-9);
}

// ============================================================================
// 정수-실수 혼합 연산 테스트
// ============================================================================

TEST(FloatTest, ShouldAddIntegerAndFloat)
{
    auto result = evalInput("10 + 3.5");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 13.5, 1e-9);
}

TEST(FloatTest, ShouldSubtractFloatFromInteger)
{
    auto result = evalInput("20 - 7.5");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 12.5, 1e-9);
}

TEST(FloatTest, ShouldMultiplyIntegerAndFloat)
{
    auto result = evalInput("5 * 2.5");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 12.5, 1e-9);
}

TEST(FloatTest, ShouldDivideIntegerByFloat)
{
    auto result = evalInput("15 / 2.5");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 6.0, 1e-9);
}

TEST(FloatTest, ShouldDivideFloatByInteger)
{
    auto result = evalInput("7.5 / 3");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 2.5, 1e-9);
}

// ============================================================================
// 타입 변환 함수 테스트
// ============================================================================

TEST(FloatTest, ShouldConvertIntegerToFloat)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("실수(42)");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 42.0, 1e-9);
}

TEST(FloatTest, ShouldConvertFloatToFloat)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("실수(3.14)");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 3.14, 1e-9);
}

TEST(FloatTest, ShouldConvertStringToFloat)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("실수(\"2.718\")");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 2.718, 1e-9);
}

TEST(FloatTest, ShouldConvertBooleanToFloat)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result1 = evalInput("실수(참)");
    auto result2 = evalInput("실수(거짓)");

    EXPECT_TRUE(result1.isFloat());
    EXPECT_NEAR(result1.asFloat(), 1.0, 1e-9);

    EXPECT_TRUE(result2.isFloat());
    EXPECT_NEAR(result2.asFloat(), 0.0, 1e-9);
}

TEST(FloatTest, ShouldConvertFloatToInteger)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("정수(3.7)");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 3);
}

TEST(FloatTest, ShouldConvertNegativeFloatToInteger)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("정수(-2.9)");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), -2);
}

// ============================================================================
// 수학 함수 테스트
// ============================================================================

TEST(FloatTest, ShouldRoundFloat)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result1 = evalInput("반올림(3.4)");
    auto result2 = evalInput("반올림(3.5)");
    auto result3 = evalInput("반올림(3.6)");

    EXPECT_TRUE(result1.isInteger());
    EXPECT_EQ(result1.asInteger(), 3);

    EXPECT_TRUE(result2.isInteger());
    EXPECT_EQ(result2.asInteger(), 4);

    EXPECT_TRUE(result3.isInteger());
    EXPECT_EQ(result3.asInteger(), 4);
}

TEST(FloatTest, ShouldRoundNegativeFloat)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("반올림(-2.5)");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), -2);
}

TEST(FloatTest, ShouldCeilFloat)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result1 = evalInput("올림(3.1)");
    auto result2 = evalInput("올림(3.9)");

    EXPECT_TRUE(result1.isInteger());
    EXPECT_EQ(result1.asInteger(), 4);

    EXPECT_TRUE(result2.isInteger());
    EXPECT_EQ(result2.asInteger(), 4);
}

TEST(FloatTest, ShouldFloorFloat)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result1 = evalInput("내림(3.1)");
    auto result2 = evalInput("내림(3.9)");

    EXPECT_TRUE(result1.isInteger());
    EXPECT_EQ(result1.asInteger(), 3);

    EXPECT_TRUE(result2.isInteger());
    EXPECT_EQ(result2.asInteger(), 3);
}

TEST(FloatTest, ShouldCalculateAbsoluteValueOfFloat)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result1 = evalInput("절대값(3.5)");
    auto result2 = evalInput("절대값(-3.5)");

    EXPECT_TRUE(result1.isFloat());
    EXPECT_NEAR(result1.asFloat(), 3.5, 1e-9);

    EXPECT_TRUE(result2.isFloat());
    EXPECT_NEAR(result2.asFloat(), 3.5, 1e-9);
}

TEST(FloatTest, ShouldCalculateAbsoluteValueOfInteger)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result1 = evalInput("절대값(10)");
    auto result2 = evalInput("절대값(-10)");

    EXPECT_TRUE(result1.isInteger());
    EXPECT_EQ(result1.asInteger(), 10);

    EXPECT_TRUE(result2.isInteger());
    EXPECT_EQ(result2.asInteger(), 10);
}

TEST(FloatTest, ShouldCalculateSquareRoot)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result1 = evalInput("제곱근(4)");
    auto result2 = evalInput("제곱근(9.0)");
    auto result3 = evalInput("제곱근(2.0)");

    EXPECT_TRUE(result1.isFloat());
    EXPECT_NEAR(result1.asFloat(), 2.0, 1e-9);

    EXPECT_TRUE(result2.isFloat());
    EXPECT_NEAR(result2.asFloat(), 3.0, 1e-9);

    EXPECT_TRUE(result3.isFloat());
    EXPECT_NEAR(result3.asFloat(), 1.414213562, 1e-6);
}

TEST(FloatTest, ShouldThrowOnNegativeSquareRoot)
{
    evaluator::Builtin::registerAllBuiltins();

    EXPECT_THROW({
        evalInput("제곱근(-1)");
    }, std::runtime_error);
}

TEST(FloatTest, ShouldCalculatePower)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result1 = evalInput("제곱(2, 3)");
    auto result2 = evalInput("제곱(5, 2)");
    auto result3 = evalInput("제곱(2.0, 0.5)");

    EXPECT_TRUE(result1.isInteger());
    EXPECT_EQ(result1.asInteger(), 8);

    EXPECT_TRUE(result2.isInteger());
    EXPECT_EQ(result2.asInteger(), 25);

    EXPECT_TRUE(result3.isFloat());
    EXPECT_NEAR(result3.asFloat(), 1.414213562, 1e-6);
}

TEST(FloatTest, ShouldCalculateNegativePower)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("제곱(2, -1)");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 0.5, 1e-9);
}

TEST(FloatTest, ShouldCalculateFractionalPower)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("제곱(27, 1.0 / 3.0)");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 3.0, 1e-6);
}

// ============================================================================
// 실수 비교 연산 테스트
// ============================================================================

TEST(FloatTest, ShouldCompareFloatsEqual)
{
    auto result = evalInput("3.14 == 3.14");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(FloatTest, ShouldCompareFloatsNotEqual)
{
    auto result = evalInput("3.14 != 2.71");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(FloatTest, ShouldCompareFloatsLessThan)
{
    auto result = evalInput("2.5 < 3.5");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(FloatTest, ShouldCompareFloatsGreaterThan)
{
    auto result = evalInput("5.5 > 3.5");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(FloatTest, ShouldCompareFloatAndInteger)
{
    auto result1 = evalInput("3.0 == 3");
    auto result2 = evalInput("3.5 > 3");

    EXPECT_TRUE(result1.isBoolean());
    EXPECT_FALSE(result1.asBoolean());  // 타입이 다름

    EXPECT_TRUE(result2.isBoolean());
    EXPECT_TRUE(result2.asBoolean());
}

// ============================================================================
// 실수 Truthy 테스트
// ============================================================================

TEST(FloatTest, ZeroFloatShouldBeFalsy)
{
    auto result = evalInput("만약 (0.0) { 참 } 아니면 { 거짓 }");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_FALSE(result.asBoolean());
}

TEST(FloatTest, NonZeroFloatShouldBeTruthy)
{
    auto result = evalInput("만약 (3.14) { 참 } 아니면 { 거짓 }");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(FloatTest, NegativeFloatShouldBeTruthy)
{
    auto result = evalInput("만약 (-1.5) { 참 } 아니면 { 거짓 }");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

// ============================================================================
// 에러 처리 테스트
// ============================================================================

TEST(FloatTest, ShouldThrowOnInvalidStringToFloatConversion)
{
    evaluator::Builtin::registerAllBuiltins();

    EXPECT_THROW({
        evalInput("실수(\"abc\")");
    }, std::runtime_error);
}

TEST(FloatTest, ShouldThrowOnInvalidStringToIntegerConversion)
{
    evaluator::Builtin::registerAllBuiltins();

    EXPECT_THROW({
        evalInput("정수(\"xyz\")");
    }, std::runtime_error);
}

TEST(FloatTest, ShouldThrowOnInvalidRoundArgument)
{
    evaluator::Builtin::registerAllBuiltins();

    EXPECT_THROW({
        evalInput("반올림(\"hello\")");
    }, std::runtime_error);
}

TEST(FloatTest, ShouldThrowOnInvalidPowerArgument)
{
    evaluator::Builtin::registerAllBuiltins();

    EXPECT_THROW({
        evalInput("제곱(\"hello\", 2)");
    }, std::runtime_error);
}

// ============================================================================
// 복합 사용 테스트
// ============================================================================

TEST(FloatTest, ShouldCombineFloatOperations)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("반올림(제곱근(16.0) + 2.5)");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 7);
}

TEST(FloatTest, ShouldUseFloatInLoop)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput(R"(
        실수 합 = 0.0
        i가 1부터 5까지 {
            합 = 합 + 실수(i) * 0.5
        }
        합
    )");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 7.5, 1e-9);  // (1+2+3+4+5) * 0.5 = 7.5
}

TEST(FloatTest, ShouldUseFloatInFunction)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput(R"(
        함수 원의_넓이(반지름) {
            반환 3.14159 * 제곱(반지름, 2)
        }
        원의_넓이(5)
    )");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 78.53975, 1e-3);
}

TEST(FloatTest, ShouldCalculatePythagorean)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("제곱근(제곱(3, 2) + 제곱(4, 2))");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 5.0, 1e-9);
}
