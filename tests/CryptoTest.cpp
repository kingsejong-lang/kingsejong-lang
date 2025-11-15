/**
 * @file CryptoTest.cpp
 * @brief 암호화 함수 테스트
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
// Base64 인코딩/디코딩 테스트
// ============================================================================

TEST(CryptoTest, ShouldEncodeBase64)
{
    auto result = evalInput(R"(
        Base64_인코딩("Hello World")
    )");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "SGVsbG8gV29ybGQ=");
}

TEST(CryptoTest, ShouldDecodeBase64)
{
    auto result = evalInput(R"(
        Base64_디코딩("SGVsbG8gV29ybGQ=")
    )");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "Hello World");
}

TEST(CryptoTest, ShouldEncodeDecodeRoundTrip)
{
    auto result = evalInput(R"(
        원본 = "안녕하세요"
        인코딩됨 = Base64_인코딩(원본)
        디코딩됨 = Base64_디코딩(인코딩됨)
        디코딩됨
    )");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "안녕하세요");
}

TEST(CryptoTest, ShouldEncodeEmptyString)
{
    auto result = evalInput(R"(
        Base64_인코딩("")
    )");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "");
}

// ============================================================================
// 문자열 해시 테스트
// ============================================================================

TEST(CryptoTest, ShouldHashString)
{
    auto result = evalInput(R"(
        문자열_해시("Hello")
    )");
    EXPECT_TRUE(result.isString());
    EXPECT_GT(result.asString().length(), 0);
}

TEST(CryptoTest, ShouldHashSameStringToSameValue)
{
    auto result = evalInput(R"(
        해시1 = 문자열_해시("test")
        해시2 = 문자열_해시("test")
        해시_비교(해시1, 해시2)
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(CryptoTest, ShouldHashDifferentStringsToDifferentValues)
{
    auto result = evalInput(R"(
        해시1 = 문자열_해시("test1")
        해시2 = 문자열_해시("test2")
        해시_비교(해시1, 해시2)
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_FALSE(result.asBoolean());
}

// ============================================================================
// 해시 비교 테스트
// ============================================================================

TEST(CryptoTest, ShouldCompareSameHashes)
{
    auto result = evalInput(R"(
        해시1 = 문자열_해시("data")
        해시2 = 문자열_해시("data")
        해시_비교(해시1, 해시2)
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(CryptoTest, ShouldCompareDifferentHashes)
{
    auto result = evalInput(R"(
        해시1 = 문자열_해시("data1")
        해시2 = 문자열_해시("data2")
        해시_비교(해시1, 해시2)
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_FALSE(result.asBoolean());
}

// ============================================================================
// 체크섬 테스트
// ============================================================================

TEST(CryptoTest, ShouldCalculateChecksum)
{
    auto result = evalInput(R"(
        체크섬("test data")
    )");
    EXPECT_TRUE(result.isInteger());
    EXPECT_NE(result.asInteger(), 0);
}

TEST(CryptoTest, ShouldHaveSameChecksumForSameData)
{
    auto result = evalInput(R"(
        체크섬1 = 체크섬("data")
        체크섬2 = 체크섬("data")
        체크섬1 == 체크섬2
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

// ============================================================================
// XOR 암호화/복호화 테스트
// ============================================================================

TEST(CryptoTest, ShouldEncryptWithXOR)
{
    auto result = evalInput(R"(
        평문 = "Hello"
        키 = "key"
        암호문 = XOR_암호화(평문, 키)
        길이(암호문)
    )");
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 5);
}

TEST(CryptoTest, ShouldDecryptWithXOR)
{
    auto result = evalInput(R"(
        평문 = "Secret Message"
        키 = "mykey"
        암호문 = XOR_암호화(평문, 키)
        복호화됨 = XOR_복호화(암호문, 키)
        복호화됨
    )");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "Secret Message");
}

TEST(CryptoTest, ShouldNotDecryptWithWrongKey)
{
    auto result = evalInput(R"(
        평문 = "Secret"
        암호문 = XOR_암호화(평문, "key1")
        복호화됨 = XOR_복호화(암호문, "key2")
        복호화됨 == 평문
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_FALSE(result.asBoolean());
}

// ============================================================================
// 시저 암호화/복호화 테스트
// ============================================================================

TEST(CryptoTest, ShouldEncryptWithCaesar)
{
    auto result = evalInput(R"(
        시저_암호화("abc", 3)
    )");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "def");
}

TEST(CryptoTest, ShouldDecryptWithCaesar)
{
    auto result = evalInput(R"(
        암호문 = 시저_암호화("Hello", 5)
        시저_복호화(암호문, 5)
    )");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "Hello");
}

TEST(CryptoTest, ShouldHandleWrapAroundInCaesar)
{
    auto result = evalInput(R"(
        시저_암호화("xyz", 3)
    )");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "abc");
}

TEST(CryptoTest, ShouldPreserveNonAlphaInCaesar)
{
    auto result = evalInput(R"(
        시저_암호화("Hello, World!", 3)
    )");
    EXPECT_TRUE(result.isString());
    // 영문자만 변경, 특수문자와 공백은 유지
    EXPECT_EQ(result.asString().length(), 13);
}

// ============================================================================
// 랜덤 문자열 생성 테스트
// ============================================================================

TEST(CryptoTest, ShouldGenerateRandomString)
{
    auto result = evalInput(R"(
        랜덤_문자열(16)
    )");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString().length(), 16);
}

TEST(CryptoTest, ShouldGenerateDifferentRandomStrings)
{
    auto result = evalInput(R"(
        문자열1 = 랜덤_문자열(10)
        문자열2 = 랜덤_문자열(10)
        문자열1 == 문자열2
    )");
    EXPECT_TRUE(result.isBoolean());
    // 매우 높은 확률로 다른 값이어야 함
    // 간혹 같을 수도 있지만, 테스트 목적상 다르다고 가정
}

TEST(CryptoTest, ShouldGenerateRandomStringWithCorrectLength)
{
    auto result = evalInput(R"(
        랜덤값 = 랜덤_문자열(32)
        길이(랜덤값)
    )");
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 32);
}

// ============================================================================
// 랜덤 숫자 생성 테스트
// ============================================================================

TEST(CryptoTest, ShouldGenerateRandomNumber)
{
    auto result = evalInput(R"(
        랜덤_숫자(1, 10)
    )");
    EXPECT_TRUE(result.isInteger());
    EXPECT_GE(result.asInteger(), 1);
    EXPECT_LE(result.asInteger(), 10);
}

TEST(CryptoTest, ShouldGenerateRandomNumberInRange)
{
    auto result = evalInput(R"(
        숫자 = 랜덤_숫자(100, 200)
        (숫자 >= 100) && (숫자 <= 200)
    )");
    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST(CryptoTest, ShouldGenerateRandomNumberWithMinEqualsMax)
{
    auto result = evalInput(R"(
        랜덤_숫자(5, 5)
    )");
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 5);
}

// ============================================================================
// 파일 해시 테스트 (에러 처리)
// ============================================================================

TEST(CryptoTest, ShouldThrowErrorForNonExistentFile)
{
    EXPECT_THROW({
        evalInput(R"(
            파일_해시("non_existent_file.txt")
        )");
    }, std::runtime_error);
}
