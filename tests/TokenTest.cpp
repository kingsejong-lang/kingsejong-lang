#include <gtest/gtest.h>
#include "lexer/Token.h"

/**
 * @file TokenTest.cpp
 * @brief Token 시스템 테스트
 */

using namespace kingsejong::lexer;

/**
 * @brief Token 테스트 픽스처
 */
class TokenTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 각 테스트 전 실행
    }

    void TearDown() override
    {
        // 각 테스트 후 실행
    }
};

/**
 * @brief Token 기본 생성 테스트
 */
TEST_F(TokenTest, ShouldCreateBasicToken)
{
    // Arrange & Act
    Token token(TokenType::INTEGER, "123");

    // Assert
    EXPECT_EQ(token.type, TokenType::INTEGER);
    EXPECT_EQ(token.literal, "123");
}

/**
 * @brief TokenType을 문자열로 변환하는 테스트
 */
TEST_F(TokenTest, ShouldConvertTokenTypeToString)
{
    // Arrange & Act & Assert
    EXPECT_EQ(tokenTypeToString(TokenType::INTEGER), "INTEGER");
    EXPECT_EQ(tokenTypeToString(TokenType::PLUS), "PLUS");
    EXPECT_EQ(tokenTypeToString(TokenType::JOSA_EUL), "JOSA_EUL");
    EXPECT_EQ(tokenTypeToString(TokenType::BUTEO), "BUTEO");
    EXPECT_EQ(tokenTypeToString(TokenType::JEONGSU), "JEONGSU");
}

/**
 * @brief 한글 키워드 조사 인식 테스트
 */
TEST_F(TokenTest, ShouldRecognizeJosaKeywords)
{
    // Arrange & Act & Assert
    EXPECT_EQ(lookupKeyword("을"), TokenType::JOSA_EUL);
    EXPECT_EQ(lookupKeyword("를"), TokenType::JOSA_REUL);
    EXPECT_EQ(lookupKeyword("이"), TokenType::JOSA_I);
    EXPECT_EQ(lookupKeyword("가"), TokenType::JOSA_GA);
    EXPECT_EQ(lookupKeyword("은"), TokenType::JOSA_EUN);
    EXPECT_EQ(lookupKeyword("는"), TokenType::JOSA_NEUN);
    EXPECT_EQ(lookupKeyword("의"), TokenType::JOSA_UI);
    EXPECT_EQ(lookupKeyword("로"), TokenType::JOSA_RO);
    EXPECT_EQ(lookupKeyword("으로"), TokenType::JOSA_EURO);
    EXPECT_EQ(lookupKeyword("에서"), TokenType::JOSA_ESO);
    EXPECT_EQ(lookupKeyword("에"), TokenType::JOSA_E);
}

/**
 * @brief 범위 키워드 인식 테스트
 */
TEST_F(TokenTest, ShouldRecognizeRangeKeywords)
{
    // Arrange & Act & Assert
    EXPECT_EQ(lookupKeyword("부터"), TokenType::BUTEO);
    EXPECT_EQ(lookupKeyword("까지"), TokenType::KKAJI);
    EXPECT_EQ(lookupKeyword("미만"), TokenType::MIMAN);
    EXPECT_EQ(lookupKeyword("초과"), TokenType::CHOGA);
    EXPECT_EQ(lookupKeyword("이하"), TokenType::IHA);
    EXPECT_EQ(lookupKeyword("이상"), TokenType::ISANG);
}

/**
 * @brief 반복 키워드 인식 테스트
 */
TEST_F(TokenTest, ShouldRecognizeIterationKeywords)
{
    // Arrange & Act & Assert
    EXPECT_EQ(lookupKeyword("번"), TokenType::BEON);
    EXPECT_EQ(lookupKeyword("반복"), TokenType::BANBOKK);
    EXPECT_EQ(lookupKeyword("각각"), TokenType::GAKGAK);
}

/**
 * @brief 제어문 키워드 인식 테스트
 */
TEST_F(TokenTest, ShouldRecognizeControlKeywords)
{
    // Arrange & Act & Assert
    EXPECT_EQ(lookupKeyword("만약"), TokenType::MANYAK);
    EXPECT_EQ(lookupKeyword("아니면"), TokenType::ANIMYEON);
    EXPECT_EQ(lookupKeyword("동안"), TokenType::DONGAN);
}

/**
 * @brief 함수 키워드 인식 테스트
 */
TEST_F(TokenTest, ShouldRecognizeFunctionKeywords)
{
    // Arrange & Act & Assert
    EXPECT_EQ(lookupKeyword("함수"), TokenType::HAMSU);
    EXPECT_EQ(lookupKeyword("반환"), TokenType::BANHWAN);
}

/**
 * @brief 타입 키워드 인식 테스트
 */
TEST_F(TokenTest, ShouldRecognizeTypeKeywords)
{
    // Arrange & Act & Assert
    EXPECT_EQ(lookupKeyword("정수"), TokenType::JEONGSU);
    EXPECT_EQ(lookupKeyword("실수"), TokenType::SILSU);
    EXPECT_EQ(lookupKeyword("문자"), TokenType::MUNJA);
    EXPECT_EQ(lookupKeyword("문자열"), TokenType::MUNJAYEOL);
    EXPECT_EQ(lookupKeyword("논리"), TokenType::NONLI);
}

/**
 * @brief 불리언 리터럴 인식 테스트
 */
TEST_F(TokenTest, ShouldRecognizeBooleanLiterals)
{
    // Arrange & Act & Assert
    EXPECT_EQ(lookupKeyword("참"), TokenType::CHAM);
    EXPECT_EQ(lookupKeyword("거짓"), TokenType::GEOJIT);
}

/**
 * @brief 메서드 체이닝 키워드 인식 테스트
 */
TEST_F(TokenTest, ShouldRecognizeMethodChainingKeywords)
{
    // Arrange & Act & Assert
    EXPECT_EQ(lookupKeyword("하고"), TokenType::HAGO);
    EXPECT_EQ(lookupKeyword("하라"), TokenType::HARA);
}

/**
 * @brief 키워드가 아닌 경우 IDENTIFIER 반환 테스트
 */
TEST_F(TokenTest, ShouldReturnIdentifierForNonKeyword)
{
    // Arrange & Act & Assert
    EXPECT_EQ(lookupKeyword("변수명"), TokenType::IDENTIFIER);
    EXPECT_EQ(lookupKeyword("함수명"), TokenType::IDENTIFIER);
    EXPECT_EQ(lookupKeyword("알수없음"), TokenType::IDENTIFIER);
}

/**
 * @brief isJosa 함수 테스트
 */
TEST_F(TokenTest, ShouldCheckIfStringIsJosa)
{
    // Arrange & Act & Assert
    EXPECT_TRUE(isJosa("을"));
    EXPECT_TRUE(isJosa("를"));
    EXPECT_TRUE(isJosa("이"));
    EXPECT_TRUE(isJosa("가"));
    EXPECT_TRUE(isJosa("은"));
    EXPECT_TRUE(isJosa("는"));
    EXPECT_TRUE(isJosa("의"));
    EXPECT_TRUE(isJosa("로"));
    EXPECT_TRUE(isJosa("으로"));
    EXPECT_TRUE(isJosa("에서"));
    EXPECT_TRUE(isJosa("에"));

    EXPECT_FALSE(isJosa("부터"));
    EXPECT_FALSE(isJosa("만약"));
    EXPECT_FALSE(isJosa("변수"));
}

/**
 * @brief isRangeKeyword 함수 테스트
 */
TEST_F(TokenTest, ShouldCheckIfStringIsRangeKeyword)
{
    // Arrange & Act & Assert
    EXPECT_TRUE(isRangeKeyword("부터"));
    EXPECT_TRUE(isRangeKeyword("까지"));
    EXPECT_TRUE(isRangeKeyword("미만"));
    EXPECT_TRUE(isRangeKeyword("초과"));
    EXPECT_TRUE(isRangeKeyword("이하"));
    EXPECT_TRUE(isRangeKeyword("이상"));

    EXPECT_FALSE(isRangeKeyword("을"));
    EXPECT_FALSE(isRangeKeyword("만약"));
    EXPECT_FALSE(isRangeKeyword("변수"));
}
