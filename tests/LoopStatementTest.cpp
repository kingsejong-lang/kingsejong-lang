/**
 * @file LoopStatementTest.cpp
 * @brief 반복문 테스트
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

evaluator::Value evalInput(const std::string& input)
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
// RepeatStatement 테스트
// ============================================================================

TEST(LoopStatementTest, ShouldExecuteRepeatZeroTimes)
{
    auto result = evalInput(R"(
        정수 count = 0
        0번 반복한다 {
            count = count + 1
        }
        count
    )");

    // TODO: Assignment 구현 후 활성화
    // EXPECT_TRUE(result.isInteger());
    // EXPECT_EQ(result.asInteger(), 0);
}

TEST(LoopStatementTest, ShouldExecuteRepeatOnce)
{
    auto result = evalInput(R"(
        1번 반복한다 {
            10
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 10);
}

TEST(LoopStatementTest, ShouldExecuteRepeatMultipleTimes)
{
    auto result = evalInput(R"(
        5번 반복한다 {
            20
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 20);
}

TEST(LoopStatementTest, ShouldExecuteRepeatWithExpression)
{
    auto result = evalInput(R"(
        정수 n = 3
        n번 반복한다 {
            100
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 100);
}

TEST(LoopStatementTest, ShouldThrowOnNegativeRepeatCount)
{
    EXPECT_THROW({
        evalInput(R"(
            -1번 반복한다 {
                10
            }
        )");
    }, std::runtime_error);
}

TEST(LoopStatementTest, ShouldThrowOnNonIntegerRepeatCount)
{
    EXPECT_THROW({
        evalInput(R"(
            "문자열"번 반복한다 {
                10
            }
        )");
    }, std::runtime_error);
}

// ============================================================================
// RangeForStatement 테스트
// ============================================================================

TEST(LoopStatementTest, ShouldExecuteRangeFor)
{
    auto result = evalInput(R"(
        i가 1부터 5까지 반복한다 {
            i
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 5); // 마지막 반복의 i 값
}

TEST(LoopStatementTest, ShouldExecuteRangeForWithVariableAccess)
{
    auto result = evalInput(R"(
        i가 1부터 3까지 반복한다 {
            i * 2
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 6); // 마지막 반복: 3 * 2
}

TEST(LoopStatementTest, ShouldExecuteRangeForWithZeroRange)
{
    auto result = evalInput(R"(
        i가 0부터 0까지 반복한다 {
            100
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 100);
}

TEST(LoopStatementTest, ShouldExecuteRangeForWithNegativeRange)
{
    auto result = evalInput(R"(
        i가 -2부터 2까지 반복한다 {
            i
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 2);
}

TEST(LoopStatementTest, ShouldExecuteRangeForWithExpressions)
{
    auto result = evalInput(R"(
        정수 start = 1
        정수 end = 4
        i가 start부터 end까지 반복한다 {
            i
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 4);
}

TEST(LoopStatementTest, ShouldThrowOnNonIntegerRangeStart)
{
    EXPECT_THROW({
        evalInput(R"(
            i가 "문자열"부터 10까지 반복한다 {
                i
            }
        )");
    }, std::runtime_error);
}

TEST(LoopStatementTest, ShouldThrowOnNonIntegerRangeEnd)
{
    EXPECT_THROW({
        evalInput(R"(
            i가 1부터 "문자열"까지 반복한다 {
                i
            }
        )");
    }, std::runtime_error);
}

// ============================================================================
// 중첩 반복문 테스트
// ============================================================================

TEST(LoopStatementTest, ShouldExecuteNestedRepeat)
{
    auto result = evalInput(R"(
        2번 반복한다 {
            3번 반복한다 {
                10
            }
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 10);
}

TEST(LoopStatementTest, ShouldExecuteNestedRangeFor)
{
    auto result = evalInput(R"(
        i가 1부터 2까지 반복한다 {
            j가 1부터 3까지 반복한다 {
                i + j
            }
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 5); // 마지막: i=2, j=3 → 2+3=5
}

TEST(LoopStatementTest, ShouldExecuteMixedNestedLoops)
{
    auto result = evalInput(R"(
        2번 반복한다 {
            i가 1부터 2까지 반복한다 {
                i
            }
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 2);
}

// ============================================================================
// 반복문과 조건문 결합 테스트
// ============================================================================

TEST(LoopStatementTest, ShouldCombineRepeatWithIf)
{
    auto result = evalInput(R"(
        3번 반복한다 {
            만약 (참) {
                100
            }
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 100);
}

TEST(LoopStatementTest, ShouldCombineRangeForWithIf)
{
    auto result = evalInput(R"(
        i가 1부터 5까지 반복한다 {
            만약 (i > 3) {
                i * 10
            } 아니면 {
                i
            }
        }
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 50); // 마지막: i=5, 5*10=50
}

// ============================================================================
// 에지 케이스 테스트
// ============================================================================

TEST(LoopStatementTest, ShouldHandleEmptyRepeatBody)
{
    auto result = evalInput(R"(
        3번 반복한다 {
        }
    )");

    EXPECT_TRUE(result.isNull());
}

TEST(LoopStatementTest, ShouldHandleEmptyRangeForBody)
{
    auto result = evalInput(R"(
        i가 1부터 3까지 반복한다 {
        }
    )");

    EXPECT_TRUE(result.isNull());
}

TEST(LoopStatementTest, ShouldHandleReverseRange)
{
    // start > end인 경우, 반복 실행 안됨
    auto result = evalInput(R"(
        i가 5부터 1까지 반복한다 {
            100
        }
    )");

    EXPECT_TRUE(result.isNull());
}
