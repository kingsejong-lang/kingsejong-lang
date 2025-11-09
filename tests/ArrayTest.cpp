/**
 * @file ArrayTest.cpp
 * @brief 배열 기능 테스트
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
// 배열 리터럴 테스트
// ============================================================================

TEST(ArrayTest, ShouldCreateEmptyArray)
{
    auto result = evalInput("[]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 0);
}

TEST(ArrayTest, ShouldCreateIntegerArray)
{
    auto result = evalInput("[1, 2, 3, 4, 5]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 5);

    const auto& arr = result.asArray();
    EXPECT_EQ(arr[0].asInteger(), 1);
    EXPECT_EQ(arr[1].asInteger(), 2);
    EXPECT_EQ(arr[2].asInteger(), 3);
    EXPECT_EQ(arr[3].asInteger(), 4);
    EXPECT_EQ(arr[4].asInteger(), 5);
}

TEST(ArrayTest, ShouldCreateMixedArray)
{
    auto result = evalInput("[1, \"안녕\", 참, 3.14]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 4);

    const auto& arr = result.asArray();
    EXPECT_EQ(arr[0].asInteger(), 1);
    EXPECT_EQ(arr[1].asString(), "안녕");
    EXPECT_EQ(arr[2].asBoolean(), true);
    EXPECT_EQ(arr[3].asFloat(), 3.14);
}

TEST(ArrayTest, ShouldCreateNestedArray)
{
    auto result = evalInput("[[1, 2], [3, 4], [5, 6]]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 3);

    const auto& arr = result.asArray();

    EXPECT_TRUE(arr[0].isArray());
    EXPECT_EQ(arr[0].asArray().size(), 2);
    EXPECT_EQ(arr[0].asArray()[0].asInteger(), 1);
    EXPECT_EQ(arr[0].asArray()[1].asInteger(), 2);

    EXPECT_TRUE(arr[1].isArray());
    EXPECT_EQ(arr[1].asArray().size(), 2);
    EXPECT_EQ(arr[1].asArray()[0].asInteger(), 3);
    EXPECT_EQ(arr[1].asArray()[1].asInteger(), 4);
}

TEST(ArrayTest, ShouldCreateArrayWithExpressions)
{
    auto result = evalInput("[1 + 2, 3 * 4, 10 / 2]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 3);

    const auto& arr = result.asArray();
    EXPECT_EQ(arr[0].asInteger(), 3);
    EXPECT_EQ(arr[1].asInteger(), 12);
    EXPECT_EQ(arr[2].asInteger(), 5);
}

// ============================================================================
// 배열 인덱싱 테스트
// ============================================================================

TEST(ArrayTest, ShouldAccessArrayElement)
{
    auto result = evalInput("정수 배열 = [10, 20, 30, 40, 50]\n배열[2]");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 30);
}

TEST(ArrayTest, ShouldAccessFirstElement)
{
    auto result = evalInput("[\"가\", \"나\", \"다\"][0]");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "가");
}

TEST(ArrayTest, ShouldAccessLastElement)
{
    auto result = evalInput("[1, 2, 3, 4, 5][4]");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 5);
}

TEST(ArrayTest, ShouldAccessWithNegativeIndex)
{
    auto result = evalInput("[10, 20, 30, 40, 50][-1]");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 50);
}

TEST(ArrayTest, ShouldAccessWithNegativeIndexSecondLast)
{
    auto result = evalInput("[10, 20, 30, 40, 50][-2]");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 40);
}

TEST(ArrayTest, ShouldAccessNestedArrayElement)
{
    auto result = evalInput("[[1, 2, 3], [4, 5, 6], [7, 8, 9]][1][2]");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 6);
}

TEST(ArrayTest, ShouldAccessWithVariableIndex)
{
    auto result = evalInput("정수 배열 = [100, 200, 300]\n정수 인덱스 = 1\n배열[인덱스]");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 200);
}

TEST(ArrayTest, ShouldAccessWithExpressionIndex)
{
    auto result = evalInput("[10, 20, 30, 40, 50][1 + 2]");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 40);
}

// ============================================================================
// 에러 처리 테스트
// ============================================================================

TEST(ArrayTest, ShouldThrowOnOutOfBoundsIndex)
{
    EXPECT_THROW({
        evalInput("[1, 2, 3][10]");
    }, std::runtime_error);
}

TEST(ArrayTest, ShouldThrowOnNegativeOutOfBoundsIndex)
{
    EXPECT_THROW({
        evalInput("[1, 2, 3][-10]");
    }, std::runtime_error);
}

TEST(ArrayTest, ShouldThrowOnIndexingNonArray)
{
    EXPECT_THROW({
        evalInput("42[0]");
    }, std::runtime_error);
}

TEST(ArrayTest, ShouldThrowOnNonIntegerIndex)
{
    EXPECT_THROW({
        evalInput("[1, 2, 3][\"hello\"]");
    }, std::runtime_error);
}

// ============================================================================
// 배열과 내장 함수 테스트
// ============================================================================

TEST(ArrayTest, ShouldGetArrayLength)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("길이([1, 2, 3, 4, 5])");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 5);
}

TEST(ArrayTest, ShouldGetEmptyArrayLength)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("길이([])");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 0);
}

TEST(ArrayTest, ShouldPrintArray)
{
    evaluator::Builtin::registerAllBuiltins();

    // 출력 테스트는 예외가 발생하지 않는 것만 확인
    EXPECT_NO_THROW({
        evalInput("출력([1, 2, 3])");
    });
}

// ============================================================================
// 배열 toString 테스트
// ============================================================================

TEST(ArrayTest, ShouldConvertArrayToString)
{
    auto result = evalInput("[1, 2, 3]");

    EXPECT_EQ(result.toString(), "[1, 2, 3]");
}

TEST(ArrayTest, ShouldConvertMixedArrayToString)
{
    auto result = evalInput("[1, \"안녕\", 참]");

    EXPECT_EQ(result.toString(), "[1, 안녕, 참]");
}

TEST(ArrayTest, ShouldConvertNestedArrayToString)
{
    auto result = evalInput("[[1, 2], [3, 4]]");

    EXPECT_EQ(result.toString(), "[[1, 2], [3, 4]]");
}

// ============================================================================
// 배열 Truthy 테스트
// ============================================================================

TEST(ArrayTest, EmptyArrayShouldBeFalsy)
{
    auto result = evalInput("만약 ([]) { 참 } 아니면 { 거짓 }");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_FALSE(result.asBoolean());
}

TEST(ArrayTest, NonEmptyArrayShouldBeTruthy)
{
    auto result = evalInput("만약 ([1, 2, 3]) { 참 } 아니면 { 거짓 }");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

// ============================================================================
// 배열 슬라이싱 테스트
// ============================================================================

TEST(ArrayTest, ShouldSliceArrayInclusive)
{
    auto result = evalInput("[0, 10, 20, 30, 40, 50][1부터 4까지]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 4);

    const auto& arr = result.asArray();
    EXPECT_EQ(arr[0].asInteger(), 10);
    EXPECT_EQ(arr[1].asInteger(), 20);
    EXPECT_EQ(arr[2].asInteger(), 30);
    EXPECT_EQ(arr[3].asInteger(), 40);
}

TEST(ArrayTest, ShouldSliceArrayHalfOpen)
{
    auto result = evalInput("[0, 10, 20, 30, 40, 50][1부터 4미만]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 3);

    const auto& arr = result.asArray();
    EXPECT_EQ(arr[0].asInteger(), 10);
    EXPECT_EQ(arr[1].asInteger(), 20);
    EXPECT_EQ(arr[2].asInteger(), 30);
}

TEST(ArrayTest, ShouldSliceArrayWithIsangIha)
{
    auto result = evalInput("[0, 10, 20, 30, 40, 50][2이상 4이하]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 3);

    const auto& arr = result.asArray();
    EXPECT_EQ(arr[0].asInteger(), 20);
    EXPECT_EQ(arr[1].asInteger(), 30);
    EXPECT_EQ(arr[2].asInteger(), 40);
}

TEST(ArrayTest, ShouldSliceArrayWithChogaMiman)
{
    auto result = evalInput("[0, 10, 20, 30, 40, 50][1초과 4미만]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 2);

    const auto& arr = result.asArray();
    EXPECT_EQ(arr[0].asInteger(), 20);
    EXPECT_EQ(arr[1].asInteger(), 30);
}

TEST(ArrayTest, ShouldSliceFromBeginning)
{
    auto result = evalInput("[10, 20, 30, 40, 50][0부터 2까지]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 3);

    const auto& arr = result.asArray();
    EXPECT_EQ(arr[0].asInteger(), 10);
    EXPECT_EQ(arr[1].asInteger(), 20);
    EXPECT_EQ(arr[2].asInteger(), 30);
}

TEST(ArrayTest, ShouldSliceToEnd)
{
    auto result = evalInput("[10, 20, 30, 40, 50][2부터 4까지]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 3);

    const auto& arr = result.asArray();
    EXPECT_EQ(arr[0].asInteger(), 30);
    EXPECT_EQ(arr[1].asInteger(), 40);
    EXPECT_EQ(arr[2].asInteger(), 50);
}

TEST(ArrayTest, ShouldSliceWithNegativeIndices)
{
    auto result = evalInput("[10, 20, 30, 40, 50][-3부터 -1까지]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 3);

    const auto& arr = result.asArray();
    EXPECT_EQ(arr[0].asInteger(), 30);
    EXPECT_EQ(arr[1].asInteger(), 40);
    EXPECT_EQ(arr[2].asInteger(), 50);
}

TEST(ArrayTest, ShouldSliceWithVariableIndices)
{
    auto result = evalInput("정수 배열 = [0, 10, 20, 30, 40, 50]\n정수 시작 = 1\n정수 끝 = 3\n배열[시작부터 끝까지]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 3);

    const auto& arr = result.asArray();
    EXPECT_EQ(arr[0].asInteger(), 10);
    EXPECT_EQ(arr[1].asInteger(), 20);
    EXPECT_EQ(arr[2].asInteger(), 30);
}

TEST(ArrayTest, ShouldReturnEmptyArrayForInvalidRange)
{
    auto result = evalInput("[10, 20, 30, 40, 50][4부터 2까지]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 0);
}

TEST(ArrayTest, ShouldHandleSliceOutOfBounds)
{
    auto result = evalInput("[10, 20, 30][0부터 10까지]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 3);

    const auto& arr = result.asArray();
    EXPECT_EQ(arr[0].asInteger(), 10);
    EXPECT_EQ(arr[1].asInteger(), 20);
    EXPECT_EQ(arr[2].asInteger(), 30);
}

TEST(ArrayTest, ShouldSliceMixedTypeArray)
{
    auto result = evalInput("[1, \"안녕\", 참, 3.14, 거짓][1부터 3까지]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 3);

    const auto& arr = result.asArray();
    EXPECT_EQ(arr[0].asString(), "안녕");
    EXPECT_EQ(arr[1].asBoolean(), true);
    EXPECT_EQ(arr[2].asFloat(), 3.14);
}

TEST(ArrayTest, ShouldSliceNestedArray)
{
    auto result = evalInput("[[1, 2], [3, 4], [5, 6], [7, 8]][1부터 2까지]");

    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 2);

    const auto& arr = result.asArray();
    EXPECT_TRUE(arr[0].isArray());
    EXPECT_EQ(arr[0].asArray()[0].asInteger(), 3);
    EXPECT_EQ(arr[0].asArray()[1].asInteger(), 4);

    EXPECT_TRUE(arr[1].isArray());
    EXPECT_EQ(arr[1].asArray()[0].asInteger(), 5);
    EXPECT_EQ(arr[1].asArray()[1].asInteger(), 6);
}
