/**
 * @file StringTest.cpp
 * @brief 문자열 기능 테스트
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
// 문자열 연결 테스트
// ============================================================================

TEST(StringTest, ShouldConcatenateTwoStrings)
{
    auto result = evalInput("\"안녕\" + \"하세요\"");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "안녕하세요");
}

TEST(StringTest, ShouldConcatenateMultipleStrings)
{
    auto result = evalInput("\"Hello\" + \" \" + \"World\"");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "Hello World");
}

TEST(StringTest, ShouldConcatenateKoreanAndEnglish)
{
    auto result = evalInput("\"킹세종\" + \" \" + \"Language\"");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "킹세종 Language");
}

TEST(StringTest, ShouldConcatenateWithVariable)
{
    auto result = evalInput("문자열 이름 = \"세종\"\n\"안녕 \" + 이름");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "안녕 세종");
}

// ============================================================================
// 문자열 비교 테스트
// ============================================================================

TEST(StringTest, ShouldCompareStringsEqual)
{
    auto result = evalInput("\"안녕\" == \"안녕\"");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(StringTest, ShouldCompareStringsNotEqual)
{
    auto result = evalInput("\"안녕\" != \"하이\"");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(StringTest, ShouldCompareStringsLessThan)
{
    auto result = evalInput("\"a\" < \"b\"");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(StringTest, ShouldCompareStringsGreaterThan)
{
    auto result = evalInput("\"z\" > \"a\"");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(StringTest, ShouldCompareStringsLessThanOrEqual)
{
    auto result = evalInput("\"apple\" <= \"banana\"");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(StringTest, ShouldCompareStringsGreaterThanOrEqual)
{
    auto result = evalInput("\"zebra\" >= \"apple\"");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(StringTest, ShouldCompareKoreanStrings)
{
    auto result = evalInput("\"가\" < \"나\"");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

// ============================================================================
// 길이() 함수 테스트
// ============================================================================

TEST(StringTest, ShouldGetStringLength)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("길이(\"안녕하세요\")");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 5);
}

TEST(StringTest, ShouldGetEmptyStringLength)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("길이(\"\")");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 0);
}

TEST(StringTest, ShouldGetMixedStringLength)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("길이(\"Hello안녕123\")");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 10);
}

// ============================================================================
// 분리() 함수 테스트
// ============================================================================

TEST(StringTest, ShouldSplitStringByDelimiter)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("분리(\"사과,바나나,딸기\", \",\")");

    EXPECT_TRUE(result.isArray());
    const auto& arr = result.asArray();
    EXPECT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0].asString(), "사과");
    EXPECT_EQ(arr[1].asString(), "바나나");
    EXPECT_EQ(arr[2].asString(), "딸기");
}

TEST(StringTest, ShouldSplitStringBySpace)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("분리(\"Hello World KingSejong\", \" \")");

    EXPECT_TRUE(result.isArray());
    const auto& arr = result.asArray();
    EXPECT_EQ(arr.size(), 3);
    EXPECT_EQ(arr[0].asString(), "Hello");
    EXPECT_EQ(arr[1].asString(), "World");
    EXPECT_EQ(arr[2].asString(), "KingSejong");
}

TEST(StringTest, ShouldSplitStringIntoCharacters)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("분리(\"안녕\", \"\")");

    EXPECT_TRUE(result.isArray());
    const auto& arr = result.asArray();
    EXPECT_EQ(arr.size(), 2);
    EXPECT_EQ(arr[0].asString(), "안");
    EXPECT_EQ(arr[1].asString(), "녕");
}

TEST(StringTest, ShouldSplitEmptyString)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("분리(\"\", \",\")");

    EXPECT_TRUE(result.isArray());
    const auto& arr = result.asArray();
    EXPECT_EQ(arr.size(), 1);
    EXPECT_EQ(arr[0].asString(), "");
}

// ============================================================================
// 찾기() 함수 테스트
// ============================================================================

TEST(StringTest, ShouldFindSubstring)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("찾기(\"안녕하세요\", \"하세요\")");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 2);
}

TEST(StringTest, ShouldFindSubstringAtBeginning)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("찾기(\"Hello World\", \"Hello\")");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 0);
}

TEST(StringTest, ShouldReturnNegativeWhenNotFound)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("찾기(\"안녕하세요\", \"감사합니다\")");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), -1);
}

TEST(StringTest, ShouldFindKoreanCharacter)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("찾기(\"킹세종언어\", \"언어\")");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 3);
}

// ============================================================================
// 바꾸기() 함수 테스트
// ============================================================================

TEST(StringTest, ShouldReplaceSubstring)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("바꾸기(\"안녕하세요\", \"안녕\", \"잘가\")");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "잘가하세요");
}

TEST(StringTest, ShouldReplaceAllOccurrences)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("바꾸기(\"apple apple apple\", \"apple\", \"orange\")");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "orange orange orange");
}

TEST(StringTest, ShouldReplaceWithEmptyString)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("바꾸기(\"Hello World\", \" World\", \"\")");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "Hello");
}

TEST(StringTest, ShouldNotReplaceWhenNotFound)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("바꾸기(\"안녕하세요\", \"감사합니다\", \"고맙습니다\")");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "안녕하세요");
}

TEST(StringTest, ShouldHandleEmptyOldString)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("바꾸기(\"Hello\", \"\", \"x\")");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "Hello");
}

// ============================================================================
// 대문자() 함수 테스트
// ============================================================================

TEST(StringTest, ShouldConvertToUppercase)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("대문자(\"hello\")");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "HELLO");
}

TEST(StringTest, ShouldConvertMixedCaseToUppercase)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("대문자(\"Hello World\")");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "HELLO WORLD");
}

TEST(StringTest, ShouldNotChangeKoreanInUppercase)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("대문자(\"안녕Hello\")");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "안녕HELLO");
}

TEST(StringTest, ShouldNotChangeAlreadyUppercase)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("대문자(\"HELLO\")");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "HELLO");
}

// ============================================================================
// 소문자() 함수 테스트
// ============================================================================

TEST(StringTest, ShouldConvertToLowercase)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("소문자(\"HELLO\")");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "hello");
}

TEST(StringTest, ShouldConvertMixedCaseToLowercase)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("소문자(\"Hello World\")");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "hello world");
}

TEST(StringTest, ShouldNotChangeKoreanInLowercase)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("소문자(\"안녕HELLO\")");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "안녕hello");
}

TEST(StringTest, ShouldNotChangeAlreadyLowercase)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("소문자(\"hello\")");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "hello");
}

// ============================================================================
// 문자열 Truthy 테스트
// ============================================================================

TEST(StringTest, EmptyStringShouldBeFalsy)
{
    auto result = evalInput("만약 (\"\") { 참 } 아니면 { 거짓 }");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_FALSE(result.asBoolean());
}

TEST(StringTest, NonEmptyStringShouldBeTruthy)
{
    auto result = evalInput("만약 (\"안녕\") { 참 } 아니면 { 거짓 }");

    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

// ============================================================================
// 에러 처리 테스트
// ============================================================================

TEST(StringTest, ShouldThrowOnInvalidSplitArguments)
{
    evaluator::Builtin::registerAllBuiltins();

    EXPECT_THROW({
        evalInput("분리(\"test\")");
    }, std::runtime_error);
}

TEST(StringTest, ShouldThrowOnInvalidFindArguments)
{
    evaluator::Builtin::registerAllBuiltins();

    EXPECT_THROW({
        evalInput("찾기(\"test\")");
    }, std::runtime_error);
}

TEST(StringTest, ShouldThrowOnInvalidReplaceArguments)
{
    evaluator::Builtin::registerAllBuiltins();

    EXPECT_THROW({
        evalInput("바꾸기(\"test\", \"old\")");
    }, std::runtime_error);
}

TEST(StringTest, ShouldThrowOnNonStringInUppercase)
{
    evaluator::Builtin::registerAllBuiltins();

    EXPECT_THROW({
        evalInput("대문자(123)");
    }, std::runtime_error);
}

TEST(StringTest, ShouldThrowOnNonStringInLowercase)
{
    evaluator::Builtin::registerAllBuiltins();

    EXPECT_THROW({
        evalInput("소문자(123)");
    }, std::runtime_error);
}

// ============================================================================
// 복합 사용 테스트
// ============================================================================

TEST(StringTest, ShouldCombineStringOperations)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("대문자(바꾸기(\"hello world\", \"world\", \"kingsejong\"))");

    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "HELLO KINGSEJONG");
}

TEST(StringTest, ShouldSplitAndGetLength)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("길이(분리(\"a,b,c\", \",\"))");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 3);
}

TEST(StringTest, ShouldFindInConcatenatedString)
{
    evaluator::Builtin::registerAllBuiltins();
    auto result = evalInput("찾기(\"Hello\" + \" \" + \"World\", \"World\")");

    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 6);
}
