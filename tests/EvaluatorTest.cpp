/**
 * @file EvaluatorTest.cpp
 * @brief Evaluator 클래스 테스트
 * @author KingSejong Team
 * @date 2025-11-07
 */

#include <gtest/gtest.h>
#include "evaluator/Evaluator.h"
#include "parser/Parser.h"
#include "lexer/Lexer.h"

using namespace kingsejong;

// ============================================================================
// 헬퍼 함수
// ============================================================================

/**
 * @brief 코드를 파싱하고 평가하는 헬퍼 함수
 * @param input 입력 코드
 * @return 평가 결과 Value
 */
static evaluator::Value evalInput(const std::string& input)
{
    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    EXPECT_EQ(parser.errors().size(), 0) << "파서 에러: " << (parser.errors().empty() ? "" : parser.errors()[0]);

    evaluator::Evaluator evaluator;
    return evaluator.evalProgram(program.get());
}

// ============================================================================
// 리터럴 평가 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateIntegerLiteral)
{
    auto result = evalInput("42");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 42);
}

TEST(EvaluatorTest, ShouldEvaluateFloatLiteral)
{
    auto result = evalInput("3.14");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 3.14, 1e-9);
}

TEST(EvaluatorTest, ShouldEvaluateStringLiteral)
{
    auto result = evalInput("\"안녕하세요\"");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "안녕하세요");
}

TEST(EvaluatorTest, ShouldEvaluateBooleanLiteral)
{
    auto result1 = evalInput("참");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    auto result2 = evalInput("거짓");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

// ============================================================================
// 산술 연산 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateIntegerAddition)
{
    auto result = evalInput("5 + 3");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 8);
}

TEST(EvaluatorTest, ShouldEvaluateIntegerSubtraction)
{
    auto result = evalInput("10 - 4");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 6);
}

TEST(EvaluatorTest, ShouldEvaluateIntegerMultiplication)
{
    auto result = evalInput("6 * 7");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 42);
}

TEST(EvaluatorTest, ShouldEvaluateIntegerDivision)
{
    auto result = evalInput("20 / 5");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 4);
}

TEST(EvaluatorTest, ShouldEvaluateIntegerModulo)
{
    auto result = evalInput("17 % 5");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 2);
}

TEST(EvaluatorTest, ShouldEvaluateComplexArithmetic)
{
    auto result = evalInput("2 + 3 * 4");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 14);  // 우선순위: 3*4=12, 2+12=14
}

TEST(EvaluatorTest, ShouldEvaluateArithmeticWithParentheses)
{
    auto result = evalInput("(2 + 3) * 4");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 20);  // 괄호 우선: 2+3=5, 5*4=20
}

// ============================================================================
// 실수 연산 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateFloatAddition)
{
    auto result = evalInput("3.5 + 2.5");

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 6.0, 1e-9);
}

TEST(EvaluatorTest, ShouldEvaluateMixedArithmetic)
{
    auto result = evalInput("10 + 3.5");  // 정수 + 실수 = 실수

    EXPECT_TRUE(result.isFloat());
    EXPECT_NEAR(result.asFloat(), 13.5, 1e-9);
}

// ============================================================================
// 단항 연산 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateNegation)
{
    auto result = evalInput("-5");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), -5);
}

TEST(EvaluatorTest, ShouldEvaluateLogicalNot)
{
    auto result1 = evalInput("!참");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_FALSE(result1.asBoolean());

    auto result2 = evalInput("!거짓");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_TRUE(result2.asBoolean());
}

// ============================================================================
// 비교 연산 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateEquality)
{
    auto result1 = evalInput("5 == 5");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    auto result2 = evalInput("5 == 3");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

TEST(EvaluatorTest, ShouldEvaluateInequality)
{
    auto result1 = evalInput("5 != 3");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    auto result2 = evalInput("5 != 5");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

TEST(EvaluatorTest, ShouldEvaluateLessThan)
{
    auto result1 = evalInput("3 < 5");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    auto result2 = evalInput("5 < 3");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

TEST(EvaluatorTest, ShouldEvaluateGreaterThan)
{
    auto result1 = evalInput("5 > 3");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    auto result2 = evalInput("3 > 5");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

// ============================================================================
// 논리 연산 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateLogicalAnd)
{
    auto result1 = evalInput("참 && 참");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    auto result2 = evalInput("참 && 거짓");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

TEST(EvaluatorTest, ShouldEvaluateLogicalOr)
{
    auto result1 = evalInput("참 || 거짓");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());

    auto result2 = evalInput("거짓 || 거짓");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());
}

// ============================================================================
// 변수 선언 및 참조 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateVarDeclaration)
{
    auto result = evalInput("정수 x = 42\nx");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 42);
}

TEST(EvaluatorTest, ShouldEvaluateVarDeclarationWithoutInitializer)
{
    auto result = evalInput("정수 x\nx");

    EXPECT_TRUE(result.isNull());
}

TEST(EvaluatorTest, ShouldEvaluateMultipleVarDeclarations)
{
    auto result = evalInput(R"(
        정수 a = 10
        정수 b = 20
        a + b
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 30);
}

TEST(EvaluatorTest, ShouldEvaluateVarReassignment)
{
    auto result = evalInput(R"(
        정수 x = 5
        x = 10
        x
    )");

    // AssignmentStatement가 구현되어 정상 동작
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 10);
}

// ============================================================================
// 문자열 연산 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateStringConcatenation)
{
    auto result = evalInput("\"안녕\" + \"하세요\"");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "안녕하세요");
}

// ============================================================================
// 에러 처리 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldThrowOnUndefinedVariable)
{
    EXPECT_THROW(
        {
            evalInput("undefined_var");
        },
        std::runtime_error
    );
}

TEST(EvaluatorTest, ShouldThrowOnDivisionByZero)
{
    EXPECT_THROW(
        {
            evalInput("10 / 0");
        },
        std::runtime_error
    );
}

TEST(EvaluatorTest, ShouldThrowOnModuloByZero)
{
    EXPECT_THROW(
        {
            evalInput("10 % 0");
        },
        std::runtime_error
    );
}

// ============================================================================
// 복합 표현식 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateComplexExpression)
{
    auto result = evalInput(R"(
        정수 a = 5
        정수 b = 3
        정수 c = 2
        (a + b) * c
    )");

    // Parser-level ASI로 해결됨: 줄이 바뀌면 자동으로 세미콜론 삽입
    // 정수 c = 2 다음에 개행되므로 ASI가 적용되어 (a + b)가 함수 호출로 인식되지 않음
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 16);  // (5+3)*2 = 16
}

TEST(EvaluatorTest, ShouldEvaluateNestedExpressions)
{
    auto result = evalInput("((2 + 3) * (4 + 5)) / 3");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 15);  // ((5)*(9))/3 = 45/3 = 15
}

// ============================================================================
// Truthy/Falsy 테스트
// ============================================================================

TEST(EvaluatorTest, ShouldEvaluateTruthiness)
{
    auto result1 = evalInput("!0");
    EXPECT_TRUE(result1.isBoolean());
    EXPECT_TRUE(result1.asBoolean());  // 0은 거짓

    auto result2 = evalInput("!1");
    EXPECT_TRUE(result2.isBoolean());
    EXPECT_FALSE(result2.asBoolean());  // 1은 참

    auto result3 = evalInput("!\"\"");
    EXPECT_TRUE(result3.isBoolean());
    EXPECT_TRUE(result3.asBoolean());  // 빈 문자열은 거짓
}
