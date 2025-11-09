/**
 * @file ArrayMethodTest.cpp
 * @brief 배열 메서드 및 조사 체이닝 테스트
 * @author KingSejong Team
 * @date 2025-11-09
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Value.h"

using namespace kingsejong;

/**
 * @brief 코드를 평가하여 Value 반환
 */
static evaluator::Value evalInput(const std::string& input)
{
    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    EXPECT_TRUE(parser.errors().empty())
        << "Parser errors: " << (parser.errors().empty() ? "" : parser.errors()[0]);

    auto env = std::make_shared<evaluator::Environment>();
    evaluator::Evaluator evaluator(env);

    return evaluator.evalProgram(program.get());
}

// ============================================================================
// 기본 배열 메서드 테스트
// ============================================================================

/**
 * @test 배열 정렬 메서드
 */
TEST(ArrayMethodTest, ShouldSortArray)
{
    auto result = evalInput(R"(
        arr = [3, 1, 4, 1, 5, 9, 2, 6]
        arr을 정렬한다
    )");

    EXPECT_TRUE(result.isArray());
    auto sorted = result.asArray();

    EXPECT_EQ(sorted.size(), 8);
    EXPECT_EQ(sorted[0].asInteger(), 1);
    EXPECT_EQ(sorted[1].asInteger(), 1);
    EXPECT_EQ(sorted[2].asInteger(), 2);
    EXPECT_EQ(sorted[3].asInteger(), 3);
    EXPECT_EQ(sorted[4].asInteger(), 4);
    EXPECT_EQ(sorted[5].asInteger(), 5);
    EXPECT_EQ(sorted[6].asInteger(), 6);
    EXPECT_EQ(sorted[7].asInteger(), 9);
}

/**
 * @test 배열 역순 메서드
 */
TEST(ArrayMethodTest, ShouldReverseArray)
{
    auto result = evalInput(R"(
        arr = [1, 2, 3, 4, 5]
        arr을 역순으로_나열한다
    )");

    EXPECT_TRUE(result.isArray());
    auto reversed = result.asArray();

    EXPECT_EQ(reversed.size(), 5);
    EXPECT_EQ(reversed[0].asInteger(), 5);
    EXPECT_EQ(reversed[1].asInteger(), 4);
    EXPECT_EQ(reversed[2].asInteger(), 3);
    EXPECT_EQ(reversed[3].asInteger(), 2);
    EXPECT_EQ(reversed[4].asInteger(), 1);
}

/**
 * @test 빈 배열 정렬
 */
TEST(ArrayMethodTest, ShouldSortEmptyArray)
{
    auto result = evalInput("[]를 정렬한다");

    EXPECT_TRUE(result.isArray());
    auto sorted = result.asArray();
    EXPECT_EQ(sorted.size(), 0);
}

/**
 * @test 단일 요소 배열 정렬
 */
TEST(ArrayMethodTest, ShouldSortSingleElementArray)
{
    auto result = evalInput("[42]를 정렬한다");

    EXPECT_TRUE(result.isArray());
    auto sorted = result.asArray();
    EXPECT_EQ(sorted.size(), 1);
    EXPECT_EQ(sorted[0].asInteger(), 42);
}

// ============================================================================
// 메서드 체이닝 테스트
// ============================================================================

/**
 * @test 정렬 후 역순 (메서드 체이닝)
 */
TEST(ArrayMethodTest, ShouldChainSortAndReverse)
{
    auto result = evalInput(R"(
        arr = [3, 1, 4, 1, 5, 9, 2, 6]
        sorted = arr을 정렬한다
        sorted를 역순으로_나열한다
    )");

    EXPECT_TRUE(result.isArray());
    auto chained = result.asArray();

    EXPECT_EQ(chained.size(), 8);
    // 정렬 후 역순이므로 내림차순
    EXPECT_EQ(chained[0].asInteger(), 9);
    EXPECT_EQ(chained[1].asInteger(), 6);
    EXPECT_EQ(chained[2].asInteger(), 5);
    EXPECT_EQ(chained[3].asInteger(), 4);
    EXPECT_EQ(chained[4].asInteger(), 3);
    EXPECT_EQ(chained[5].asInteger(), 2);
    EXPECT_EQ(chained[6].asInteger(), 1);
    EXPECT_EQ(chained[7].asInteger(), 1);
}

/**
 * @test 역순 후 정렬 (메서드 체이닝)
 */
TEST(ArrayMethodTest, ShouldChainReverseAndSort)
{
    auto result = evalInput(R"(
        arr = [5, 3, 1, 4, 2]
        reversed = arr을 역순으로_나열한다
        reversed를 정렬한다
    )");

    EXPECT_TRUE(result.isArray());
    auto chained = result.asArray();

    EXPECT_EQ(chained.size(), 5);
    // 역순 후 정렬이므로 결국 오름차순
    EXPECT_EQ(chained[0].asInteger(), 1);
    EXPECT_EQ(chained[1].asInteger(), 2);
    EXPECT_EQ(chained[2].asInteger(), 3);
    EXPECT_EQ(chained[3].asInteger(), 4);
    EXPECT_EQ(chained[4].asInteger(), 5);
}

/**
 * @test 인라인 배열 리터럴에 메서드 적용
 */
TEST(ArrayMethodTest, ShouldApplyMethodToArrayLiteral)
{
    auto result = evalInput(R"(
        [5, 2, 8, 1, 9]를 정렬한다
    )");

    EXPECT_TRUE(result.isArray());
    auto sorted = result.asArray();

    EXPECT_EQ(sorted.size(), 5);
    EXPECT_EQ(sorted[0].asInteger(), 1);
    EXPECT_EQ(sorted[1].asInteger(), 2);
    EXPECT_EQ(sorted[2].asInteger(), 5);
    EXPECT_EQ(sorted[3].asInteger(), 8);
    EXPECT_EQ(sorted[4].asInteger(), 9);
}

/**
 * @test 인라인 배열에 체이닝
 */
TEST(ArrayMethodTest, ShouldChainOnArrayLiteral)
{
    auto result = evalInput(R"(
        reversed = [1, 2, 3, 4, 5]를 역순으로_나열한다
        reversed를 정렬한다
    )");

    EXPECT_TRUE(result.isArray());
    auto chained = result.asArray();

    EXPECT_EQ(chained.size(), 5);
    EXPECT_EQ(chained[0].asInteger(), 1);
    EXPECT_EQ(chained[1].asInteger(), 2);
    EXPECT_EQ(chained[2].asInteger(), 3);
    EXPECT_EQ(chained[3].asInteger(), 4);
    EXPECT_EQ(chained[4].asInteger(), 5);
}

// ============================================================================
// 다양한 타입 정렬 테스트
// ============================================================================

/**
 * @test 문자열 배열 정렬
 */
TEST(ArrayMethodTest, ShouldSortStringArray)
{
    auto result = evalInput(R"(
        ["다", "가", "나"]를 정렬한다
    )");

    EXPECT_TRUE(result.isArray());
    auto sorted = result.asArray();

    EXPECT_EQ(sorted.size(), 3);
    EXPECT_EQ(sorted[0].asString(), "가");
    EXPECT_EQ(sorted[1].asString(), "나");
    EXPECT_EQ(sorted[2].asString(), "다");
}

/**
 * @test 실수 배열 정렬
 */
TEST(ArrayMethodTest, ShouldSortFloatArray)
{
    auto result = evalInput(R"(
        [3.14, 1.41, 2.71, 1.73]을 정렬한다
    )");

    EXPECT_TRUE(result.isArray());
    auto sorted = result.asArray();

    EXPECT_EQ(sorted.size(), 4);
    EXPECT_DOUBLE_EQ(sorted[0].asFloat(), 1.41);
    EXPECT_DOUBLE_EQ(sorted[1].asFloat(), 1.73);
    EXPECT_DOUBLE_EQ(sorted[2].asFloat(), 2.71);
    EXPECT_DOUBLE_EQ(sorted[3].asFloat(), 3.14);
}

// ============================================================================
// 에러 처리 테스트
// ============================================================================

/**
 * @test 배열이 아닌 값에 메서드 적용 시 에러
 */
TEST(ArrayMethodTest, ShouldThrowErrorOnNonArray)
{
    EXPECT_THROW({
        evalInput("42를 정렬한다");
    }, std::exception);
}

/**
 * @test 존재하지 않는 메서드 호출 시 에러
 */
TEST(ArrayMethodTest, ShouldThrowErrorOnUnknownMethod)
{
    EXPECT_THROW({
        evalInput("[1, 2, 3]을 존재하지않는메서드()");
    }, std::exception);
}

// ============================================================================
// 함수형 배열 메서드 테스트
// ============================================================================

/**
 * @test 걸러낸다 (filter) - 짝수만 필터링
 */
TEST(ArrayMethodTest, ShouldFilterEvenNumbers)
{
    auto result = evalInput(R"(
        arr = [1, 2, 3, 4, 5, 6]
        arr을 걸러낸다(함수(x) { 반환 x % 2 == 0 })
    )");

    EXPECT_TRUE(result.isArray());
    auto filtered = result.asArray();

    EXPECT_EQ(filtered.size(), 3);
    EXPECT_EQ(filtered[0].asInteger(), 2);
    EXPECT_EQ(filtered[1].asInteger(), 4);
    EXPECT_EQ(filtered[2].asInteger(), 6);
}

/**
 * @test 걸러낸다 (filter) - 양수만 필터링
 */
TEST(ArrayMethodTest, ShouldFilterPositiveNumbers)
{
    auto result = evalInput(R"(
        arr = [-2, -1, 0, 1, 2, 3]
        arr을 걸러낸다(함수(x) { 반환 x > 0 })
    )");

    EXPECT_TRUE(result.isArray());
    auto filtered = result.asArray();

    EXPECT_EQ(filtered.size(), 3);
    EXPECT_EQ(filtered[0].asInteger(), 1);
    EXPECT_EQ(filtered[1].asInteger(), 2);
    EXPECT_EQ(filtered[2].asInteger(), 3);
}

/**
 * @test 변환한다 (map) - 각 요소를 2배로
 */
TEST(ArrayMethodTest, ShouldMapDoubleValues)
{
    auto result = evalInput(R"(
        arr = [1, 2, 3, 4, 5]
        arr을 변환한다(함수(x) { 반환 x * 2 })
    )");

    EXPECT_TRUE(result.isArray());
    auto mapped = result.asArray();

    EXPECT_EQ(mapped.size(), 5);
    EXPECT_EQ(mapped[0].asInteger(), 2);
    EXPECT_EQ(mapped[1].asInteger(), 4);
    EXPECT_EQ(mapped[2].asInteger(), 6);
    EXPECT_EQ(mapped[3].asInteger(), 8);
    EXPECT_EQ(mapped[4].asInteger(), 10);
}

/**
 * @test 변환한다 (map) - 제곱
 */
TEST(ArrayMethodTest, ShouldMapSquareValues)
{
    auto result = evalInput(R"(
        arr = [1, 2, 3, 4, 5]
        arr을 변환한다(함수(x) { 반환 x * x })
    )");

    EXPECT_TRUE(result.isArray());
    auto mapped = result.asArray();

    EXPECT_EQ(mapped.size(), 5);
    EXPECT_EQ(mapped[0].asInteger(), 1);
    EXPECT_EQ(mapped[1].asInteger(), 4);
    EXPECT_EQ(mapped[2].asInteger(), 9);
    EXPECT_EQ(mapped[3].asInteger(), 16);
    EXPECT_EQ(mapped[4].asInteger(), 25);
}

/**
 * @test 축약한다 (reduce) - 합계
 */
TEST(ArrayMethodTest, ShouldReduceSum)
{
    auto result = evalInput(R"(
        arr = [1, 2, 3, 4, 5]
        arr을 축약한다(0, 함수(누적, 현재) { 반환 누적 + 현재 })
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 15);
}

/**
 * @test 축약한다 (reduce) - 곱셈
 */
TEST(ArrayMethodTest, ShouldReduceProduct)
{
    auto result = evalInput(R"(
        arr = [1, 2, 3, 4, 5]
        arr을 축약한다(1, 함수(누적, 현재) { 반환 누적 * 현재 })
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 120);  // 5!
}

/**
 * @test 찾다 (find) - 첫 번째 짝수 찾기
 */
TEST(ArrayMethodTest, ShouldFindFirstEvenNumber)
{
    auto result = evalInput(R"(
        arr = [1, 3, 5, 6, 8, 9]
        arr을 찾다(함수(x) { 반환 x % 2 == 0 })
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 6);
}

/**
 * @test 찾다 (find) - 조건에 맞는 요소가 없는 경우
 */
TEST(ArrayMethodTest, ShouldReturnNullWhenNotFound)
{
    auto result = evalInput(R"(
        arr = [1, 3, 5, 7, 9]
        arr을 찾다(함수(x) { 반환 x > 10 })
    )");

    EXPECT_TRUE(result.isNull());
}

/**
 * @test 함수형 메서드 체이닝 - filter + map
 */
TEST(ArrayMethodTest, ShouldChainFilterAndMap)
{
    auto result = evalInput(R"(
        arr = [1, 2, 3, 4, 5, 6]
        filtered = arr을 걸러낸다(함수(x) { 반환 x % 2 == 0 })
        filtered를 변환한다(함수(x) { 반환 x * x })
    )");

    EXPECT_TRUE(result.isArray());
    auto chained = result.asArray();

    EXPECT_EQ(chained.size(), 3);
    EXPECT_EQ(chained[0].asInteger(), 4);   // 2^2
    EXPECT_EQ(chained[1].asInteger(), 16);  // 4^2
    EXPECT_EQ(chained[2].asInteger(), 36);  // 6^2
}

/**
 * @test 함수형 메서드 체이닝 - filter + map + reduce
 */
TEST(ArrayMethodTest, ShouldChainFilterMapReduce)
{
    auto result = evalInput(R"(
        arr = [1, 2, 3, 4, 5, 6]
        filtered = arr을 걸러낸다(함수(x) { 반환 x % 2 == 0 })
        mapped = filtered를 변환한다(함수(x) { 반환 x * x })
        mapped를 축약한다(0, 함수(누적, 현재) { 반환 누적 + 현재 })
    )");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 56);  // 2^2 + 4^2 + 6^2 = 4 + 16 + 36 = 56
}

/**
 * @test 함수형 메서드와 기본 메서드 체이닝
 */
TEST(ArrayMethodTest, ShouldChainFunctionalAndBasicMethods)
{
    auto result = evalInput(R"(
        arr = [5, 2, 8, 1, 9, 3]
        filtered = arr을 걸러낸다(함수(x) { 반환 x > 3 })
        filtered를 정렬한다
    )");

    EXPECT_TRUE(result.isArray());
    auto chained = result.asArray();

    EXPECT_EQ(chained.size(), 3);
    EXPECT_EQ(chained[0].asInteger(), 5);
    EXPECT_EQ(chained[1].asInteger(), 8);
    EXPECT_EQ(chained[2].asInteger(), 9);
}

/**
 * @test 빈 배열에 걸러낸다 적용
 */
TEST(ArrayMethodTest, ShouldFilterEmptyArray)
{
    auto result = evalInput(R"(
        arr = []
        arr을 걸러낸다(함수(x) { 반환 x > 0 })
    )");

    EXPECT_TRUE(result.isArray());
    auto filtered = result.asArray();
    EXPECT_EQ(filtered.size(), 0);
}

/**
 * @test 모든 요소가 조건을 만족하지 않는 경우
 */
TEST(ArrayMethodTest, ShouldReturnEmptyWhenNoMatch)
{
    auto result = evalInput(R"(
        arr = [1, 2, 3, 4, 5]
        arr을 걸러낸다(함수(x) { 반환 x > 10 })
    )");

    EXPECT_TRUE(result.isArray());
    auto filtered = result.asArray();
    EXPECT_EQ(filtered.size(), 0);
}
