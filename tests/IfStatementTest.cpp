/**
 * @file IfStatementTest.cpp
 * @brief If문 테스트
 * @author KingSejong Team
 * @date 2025-11-08
 */

#include <gtest/gtest.h>
#include "evaluator/Evaluator.h"
#include "parser/Parser.h"
#include "lexer/Lexer.h"

using namespace kingsejong;

// ============================================================================
// 헬퍼 함수
// ============================================================================

namespace {

static evaluator::Value evalInput(const std::string& input)
{
    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    EXPECT_EQ(parser.errors().size(), 0) << "파서 에러: " << (parser.errors().empty() ? "" : parser.errors()[0]);

    evaluator::Evaluator evaluator;
    return evaluator.evalProgram(program.get());
}

} // anonymous namespace

// ============================================================================
// 기본 if문 테스트
// ============================================================================

TEST(IfStatementTest, ShouldEvaluateIfTrue)
{
    auto result = evalInput(R"(
        만약 (참) {
            10
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 10);
}

TEST(IfStatementTest, ShouldEvaluateIfFalse)
{
    auto result = evalInput(R"(
        만약 (거짓) {
            10
        }
    )");

    EXPECT_TRUE(result.isNull());
}

TEST(IfStatementTest, ShouldEvaluateIfWithCondition)
{
    auto result = evalInput(R"(
        정수 x = 10
        만약 (x > 5) {
            20
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 20);
}

TEST(IfStatementTest, ShouldEvaluateIfFalseCondition)
{
    auto result = evalInput(R"(
        정수 x = 3
        만약 (x > 5) {
            20
        }
    )");

    EXPECT_TRUE(result.isNull());
}

// ============================================================================
// if-else문 테스트
// ============================================================================

TEST(IfStatementTest, ShouldEvaluateIfElseTrueBranch)
{
    auto result = evalInput(R"(
        만약 (참) {
            10
        } 아니면 {
            20
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 10);
}

TEST(IfStatementTest, ShouldEvaluateIfElseFalseBranch)
{
    auto result = evalInput(R"(
        만약 (거짓) {
            10
        } 아니면 {
            20
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 20);
}

TEST(IfStatementTest, ShouldEvaluateIfElseWithCondition)
{
    auto result = evalInput(R"(
        정수 x = 3
        만약 (x > 5) {
            "크다"
        } 아니면 {
            "작다"
        }
    )");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "작다");
}

// ============================================================================
// 복잡한 조건 테스트
// ============================================================================

TEST(IfStatementTest, ShouldEvaluateComplexCondition)
{
    auto result = evalInput(R"(
        정수 x = 10
        정수 y = 5
        만약 (x > 5 && y < 10) {
            "맞다"
        } 아니면 {
            "틀리다"
        }
    )");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "맞다");
}

TEST(IfStatementTest, ShouldEvaluateNestedIf)
{
    auto result = evalInput(R"(
        정수 x = 10
        만약 (x > 5) {
            만약 (x > 8) {
                "매우 크다"
            } 아니면 {
                "크다"
            }
        } 아니면 {
            "작다"
        }
    )");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "매우 크다");
}

// ============================================================================
// 변수 할당과 함께 사용
// ============================================================================

TEST(IfStatementTest, DISABLED_ShouldWorkWithVariableDeclaration)
{
    auto result = evalInput(R"(
        정수 x = 10
        정수 result = 0
        만약 (x > 5) {
            result = 100
        } 아니면 {
            result = 50
        }
        result
    )");

    // TODO: Assignment 구현 후 활성화
    // EXPECT_TRUE(result.isInteger());
    // EXPECT_EQ(result.asInteger(), 100);
}

TEST(IfStatementTest, ShouldReturnValueFromBlock)
{
    auto result = evalInput(R"(
        정수 x = 10
        만약 (x == 10) {
            정수 y = 20
            x + y
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 30);
}

// ============================================================================
// Truthiness 테스트
// ============================================================================

TEST(IfStatementTest, ShouldEvaluateTruthinessZero)
{
    auto result = evalInput(R"(
        만약 (0) {
            "참"
        } 아니면 {
            "거짓"
        }
    )");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "거짓");
}

TEST(IfStatementTest, ShouldEvaluateTruthinessNonZero)
{
    auto result = evalInput(R"(
        만약 (42) {
            "참"
        } 아니면 {
            "거짓"
        }
    )");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "참");
}

TEST(IfStatementTest, ShouldEvaluateTruthinessEmptyString)
{
    auto result = evalInput(R"(
        만약 ("") {
            "참"
        } 아니면 {
            "거짓"
        }
    )");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "거짓");
}

TEST(IfStatementTest, ShouldEvaluateTruthinessNonEmptyString)
{
    auto result = evalInput(R"(
        만약 ("hello") {
            "참"
        } 아니면 {
            "거짓"
        }
    )");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "참");
}
