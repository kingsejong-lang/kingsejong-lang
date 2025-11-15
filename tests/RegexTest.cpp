/**
 * @file RegexTest.cpp
 * @brief 정규표현식 함수 테스트
 * @author KingSejong Team
 * @date 2025-11-15
 */

#include <gtest/gtest.h>
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "parser/Parser.h"
#include "lexer/Lexer.h"

using namespace kingsejong;

// ============================================================================
// 헬퍼 함수
// ============================================================================

namespace {

/**
 * @brief 코드를 파싱하고 평가하는 헬퍼 함수
 */
static evaluator::Value evalInput(const std::string& input)
{
    evaluator::Builtin::registerAllBuiltins();

    lexer::Lexer lexer(input);
    parser::Parser parser(lexer);
    auto program = parser.parseProgram();

    EXPECT_EQ(parser.errors().size(), 0) << "파서 에러: "
        << (parser.errors().empty() ? "" : parser.errors()[0]);

    evaluator::Evaluator evaluator;
    return evaluator.evalProgram(program.get());
}

} // anonymous namespace

// ============================================================================
// 정규표현식_일치 테스트
// ============================================================================

TEST(RegexTest, ShouldMatchPattern)
{
    auto result = evalInput(R"(
        정규표현식_일치("hello123", "^[a-z]+[0-9]+$")
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(RegexTest, ShouldNotMatchPattern)
{
    auto result = evalInput(R"(
        정규표현식_일치("hello", "^[0-9]+$")
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_FALSE(result.asBoolean());
}

// ============================================================================
// 정규표현식_검색 테스트
// ============================================================================

TEST(RegexTest, ShouldSearchPattern)
{
    auto result = evalInput(R"(
        정규표현식_검색("The price is 100 won", "\\d+")
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(RegexTest, ShouldNotFindPattern)
{
    auto result = evalInput(R"(
        정규표현식_검색("Hello World", "\\d+")
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_FALSE(result.asBoolean());
}

// ============================================================================
// 정규표현식_모두_찾기 테스트
// ============================================================================

TEST(RegexTest, ShouldFindAllMatches)
{
    auto result = evalInput(R"(
        정규표현식_모두_찾기("I have 3 apples and 5 oranges", "\\d+")
    )");
    EXPECT_TRUE(result.isArray());
    const auto& arr = result.asArray();
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0].asString(), "3");
    EXPECT_EQ(arr[1].asString(), "5");
}

TEST(RegexTest, ShouldReturnEmptyArrayWhenNoMatches)
{
    auto result = evalInput(R"(
        정규표현식_모두_찾기("Hello World", "\\d+")
    )");
    EXPECT_TRUE(result.isArray());
    EXPECT_EQ(result.asArray().size(), 0);
}

// ============================================================================
// 정규표현식_치환 테스트
// ============================================================================

TEST(RegexTest, ShouldReplacePattern)
{
    auto result = evalInput(R"(
        정규표현식_치환("Hello World", "World", "KingSejong")
    )");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "Hello KingSejong");
}

TEST(RegexTest, ShouldReplaceAllOccurrences)
{
    auto result = evalInput(R"(
        정규표현식_치환("Hello Hello Hello", "Hello", "Hi")
    )");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "Hi Hi Hi");
}

// ============================================================================
// 정규표현식_분리 테스트
// ============================================================================

TEST(RegexTest, ShouldSplitByPattern)
{
    auto result = evalInput(R"(
        정규표현식_분리("apple,banana,orange", ",")
    )");
    EXPECT_TRUE(result.isArray());
    const auto& arr = result.asArray();
    EXPECT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0].asString(), "apple");
    EXPECT_EQ(arr[1].asString(), "banana");
    EXPECT_EQ(arr[2].asString(), "orange");
}

TEST(RegexTest, ShouldSplitByWhitespace)
{
    auto result = evalInput(R"(
        정규표현식_분리("one  two   three", "\\s+")
    )");
    EXPECT_TRUE(result.isArray());
    const auto& arr = result.asArray();
    EXPECT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0].asString(), "one");
    EXPECT_EQ(arr[1].asString(), "two");
    EXPECT_EQ(arr[2].asString(), "three");
}

// ============================================================================
// 이메일_검증 테스트
// ============================================================================

TEST(RegexTest, ShouldValidateEmail)
{
    auto result = evalInput(R"(
        이메일_검증("test@example.com")
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(RegexTest, ShouldRejectInvalidEmail)
{
    auto result = evalInput(R"(
        이메일_검증("invalid-email")
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_FALSE(result.asBoolean());
}

// ============================================================================
// URL_검증 테스트
// ============================================================================

TEST(RegexTest, ShouldValidateURL)
{
    auto result = evalInput(R"(
        URL_검증("https://example.com")
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(RegexTest, ShouldRejectInvalidURL)
{
    auto result = evalInput(R"(
        URL_검증("not-a-url")
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_FALSE(result.asBoolean());
}

// ============================================================================
// 전화번호_검증 테스트
// ============================================================================

TEST(RegexTest, ShouldValidatePhoneNumber)
{
    auto result = evalInput(R"(
        전화번호_검증("010-1234-5678")
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(RegexTest, ShouldRejectInvalidPhoneNumber)
{
    auto result = evalInput(R"(
        전화번호_검증("01012345678")
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_FALSE(result.asBoolean());
}

// ============================================================================
// 정규표현식_추출 테스트
// ============================================================================

TEST(RegexTest, ShouldExtractFirstMatch)
{
    auto result = evalInput(R"(
        정규표현식_추출("Error code: 404", "\\d+")
    )");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "404");
}

TEST(RegexTest, ShouldReturnEmptyStringWhenNoMatch)
{
    auto result = evalInput(R"(
        정규표현식_추출("Hello World", "\\d+")
    )");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "");
}

// ============================================================================
// 정규표현식_개수 테스트
// ============================================================================

TEST(RegexTest, ShouldCountMatches)
{
    auto result = evalInput(R"(
        정규표현식_개수("I have 3 apples and 5 oranges", "\\d+")
    )");
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 2);
}

TEST(RegexTest, ShouldReturnZeroWhenNoMatches)
{
    auto result = evalInput(R"(
        정규표현식_개수("Hello World", "\\d+")
    )");
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 0);
}
