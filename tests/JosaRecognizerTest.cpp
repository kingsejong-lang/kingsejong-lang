#include <gtest/gtest.h>
#include "lexer/JosaRecognizer.h"

/**
 * @file JosaRecognizerTest.cpp
 * @brief JosaRecognizer 테스트
 */

using namespace kingsejong::lexer;

/**
 * @brief JosaRecognizer 테스트 픽스처
 */
class JosaRecognizerTest : public ::testing::Test
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
 * @brief 받침 있는 단어 판별 테스트
 */
TEST_F(JosaRecognizerTest, ShouldDetectFinalConsonant)
{
    // Arrange & Act & Assert - 받침 있는 단어들
    EXPECT_TRUE(JosaRecognizer::hasFinalConsonant("책"));    // ㄱ받침
    EXPECT_TRUE(JosaRecognizer::hasFinalConsonant("산"));    // ㄴ받침
    EXPECT_TRUE(JosaRecognizer::hasFinalConsonant("꽃"));    // ㅅ받침
    EXPECT_TRUE(JosaRecognizer::hasFinalConsonant("밥"));    // ㅂ받침
    EXPECT_TRUE(JosaRecognizer::hasFinalConsonant("물"));    // ㄹ받침
    EXPECT_TRUE(JosaRecognizer::hasFinalConsonant("공"));    // ㅇ받침
    EXPECT_TRUE(JosaRecognizer::hasFinalConsonant("집"));    // ㅂ받침
}

/**
 * @brief 받침 없는 단어 판별 테스트
 */
TEST_F(JosaRecognizerTest, ShouldDetectNoFinalConsonant)
{
    // Arrange & Act & Assert - 받침 없는 단어들
    EXPECT_FALSE(JosaRecognizer::hasFinalConsonant("사과"));
    EXPECT_FALSE(JosaRecognizer::hasFinalConsonant("토마토"));
    EXPECT_FALSE(JosaRecognizer::hasFinalConsonant("바나나"));
    EXPECT_FALSE(JosaRecognizer::hasFinalConsonant("커피"));
    EXPECT_FALSE(JosaRecognizer::hasFinalConsonant("차"));
}

/**
 * @brief 을/를 조사 선택 테스트
 */
TEST_F(JosaRecognizerTest, ShouldSelectEulReul)
{
    // Arrange & Act & Assert - 받침 있을 때 "을"
    EXPECT_EQ(JosaRecognizer::select("책", JosaRecognizer::JosaType::EUL_REUL), "을");
    EXPECT_EQ(JosaRecognizer::select("공", JosaRecognizer::JosaType::EUL_REUL), "을");
    EXPECT_EQ(JosaRecognizer::select("집", JosaRecognizer::JosaType::EUL_REUL), "을");

    // Arrange & Act & Assert - 받침 없을 때 "를"
    EXPECT_EQ(JosaRecognizer::select("사과", JosaRecognizer::JosaType::EUL_REUL), "를");
    EXPECT_EQ(JosaRecognizer::select("바나나", JosaRecognizer::JosaType::EUL_REUL), "를");
    EXPECT_EQ(JosaRecognizer::select("커피", JosaRecognizer::JosaType::EUL_REUL), "를");
}

/**
 * @brief 이/가 조사 선택 테스트
 */
TEST_F(JosaRecognizerTest, ShouldSelectIGa)
{
    // Arrange & Act & Assert - 받침 있을 때 "이"
    EXPECT_EQ(JosaRecognizer::select("책", JosaRecognizer::JosaType::I_GA), "이");
    EXPECT_EQ(JosaRecognizer::select("산", JosaRecognizer::JosaType::I_GA), "이");
    EXPECT_EQ(JosaRecognizer::select("집", JosaRecognizer::JosaType::I_GA), "이");

    // Arrange & Act & Assert - 받침 없을 때 "가"
    EXPECT_EQ(JosaRecognizer::select("사과", JosaRecognizer::JosaType::I_GA), "가");
    EXPECT_EQ(JosaRecognizer::select("나무", JosaRecognizer::JosaType::I_GA), "가");
    EXPECT_EQ(JosaRecognizer::select("토마토", JosaRecognizer::JosaType::I_GA), "가");
}

/**
 * @brief 은/는 조사 선택 테스트
 */
TEST_F(JosaRecognizerTest, ShouldSelectEunNeun)
{
    // Arrange & Act & Assert - 받침 있을 때 "은"
    EXPECT_EQ(JosaRecognizer::select("책", JosaRecognizer::JosaType::EUN_NEUN), "은");
    EXPECT_EQ(JosaRecognizer::select("물", JosaRecognizer::JosaType::EUN_NEUN), "은");
    EXPECT_EQ(JosaRecognizer::select("밥", JosaRecognizer::JosaType::EUN_NEUN), "은");

    // Arrange & Act & Assert - 받침 없을 때 "는"
    EXPECT_EQ(JosaRecognizer::select("사과", JosaRecognizer::JosaType::EUN_NEUN), "는");
    EXPECT_EQ(JosaRecognizer::select("배", JosaRecognizer::JosaType::EUN_NEUN), "는");
    EXPECT_EQ(JosaRecognizer::select("바나나", JosaRecognizer::JosaType::EUN_NEUN), "는");
}

/**
 * @brief 로/으로 조사 선택 테스트 (ㄹ받침 특수 처리)
 */
TEST_F(JosaRecognizerTest, ShouldSelectRoEuro)
{
    // Arrange & Act & Assert - 받침 없을 때 "로"
    EXPECT_EQ(JosaRecognizer::select("도로", JosaRecognizer::JosaType::RO_EURO), "로");
    EXPECT_EQ(JosaRecognizer::select("바다", JosaRecognizer::JosaType::RO_EURO), "로");

    // Arrange & Act & Assert - ㄹ받침일 때도 "로"
    EXPECT_EQ(JosaRecognizer::select("물", JosaRecognizer::JosaType::RO_EURO), "로");
    EXPECT_EQ(JosaRecognizer::select("길", JosaRecognizer::JosaType::RO_EURO), "로");
    EXPECT_EQ(JosaRecognizer::select("별", JosaRecognizer::JosaType::RO_EURO), "로");

    // Arrange & Act & Assert - ㄹ 외 받침 있을 때 "으로"
    EXPECT_EQ(JosaRecognizer::select("집", JosaRecognizer::JosaType::RO_EURO), "으로");
    EXPECT_EQ(JosaRecognizer::select("책", JosaRecognizer::JosaType::RO_EURO), "으로");
    EXPECT_EQ(JosaRecognizer::select("산", JosaRecognizer::JosaType::RO_EURO), "으로");
}

/**
 * @brief 의 조사 선택 테스트 (받침 무관)
 */
TEST_F(JosaRecognizerTest, ShouldSelectUi)
{
    // Arrange & Act & Assert - 받침 유무와 관계없이 "의"
    EXPECT_EQ(JosaRecognizer::select("나", JosaRecognizer::JosaType::UI), "의");
    EXPECT_EQ(JosaRecognizer::select("책", JosaRecognizer::JosaType::UI), "의");
    EXPECT_EQ(JosaRecognizer::select("사과", JosaRecognizer::JosaType::UI), "의");
    EXPECT_EQ(JosaRecognizer::select("컴퓨터", JosaRecognizer::JosaType::UI), "의");
}

/**
 * @brief 에서 조사 선택 테스트 (받침 무관)
 */
TEST_F(JosaRecognizerTest, ShouldSelectEso)
{
    // Arrange & Act & Assert - 받침 유무와 관계없이 "에서"
    EXPECT_EQ(JosaRecognizer::select("집", JosaRecognizer::JosaType::ESO), "에서");
    EXPECT_EQ(JosaRecognizer::select("학교", JosaRecognizer::JosaType::ESO), "에서");
    EXPECT_EQ(JosaRecognizer::select("서울", JosaRecognizer::JosaType::ESO), "에서");
}

/**
 * @brief 에 조사 선택 테스트 (받침 무관)
 */
TEST_F(JosaRecognizerTest, ShouldSelectE)
{
    // Arrange & Act & Assert - 받침 유무와 관계없이 "에"
    EXPECT_EQ(JosaRecognizer::select("집", JosaRecognizer::JosaType::E), "에");
    EXPECT_EQ(JosaRecognizer::select("학교", JosaRecognizer::JosaType::E), "에");
    EXPECT_EQ(JosaRecognizer::select("공원", JosaRecognizer::JosaType::E), "에");
}

/**
 * @brief isJosa 함수 테스트
 */
TEST_F(JosaRecognizerTest, ShouldCheckIfStringIsJosa)
{
    // Arrange & Act & Assert
    EXPECT_TRUE(JosaRecognizer::isJosa("을"));
    EXPECT_TRUE(JosaRecognizer::isJosa("를"));
    EXPECT_TRUE(JosaRecognizer::isJosa("이"));
    EXPECT_TRUE(JosaRecognizer::isJosa("가"));
    EXPECT_TRUE(JosaRecognizer::isJosa("은"));
    EXPECT_TRUE(JosaRecognizer::isJosa("는"));
    EXPECT_TRUE(JosaRecognizer::isJosa("의"));
    EXPECT_TRUE(JosaRecognizer::isJosa("로"));
    EXPECT_TRUE(JosaRecognizer::isJosa("으로"));
    EXPECT_TRUE(JosaRecognizer::isJosa("에서"));
    EXPECT_TRUE(JosaRecognizer::isJosa("에"));

    EXPECT_FALSE(JosaRecognizer::isJosa("사과"));
    EXPECT_FALSE(JosaRecognizer::isJosa("책"));
    EXPECT_FALSE(JosaRecognizer::isJosa("만약"));
}

/**
 * @brief getType 함수 테스트
 */
TEST_F(JosaRecognizerTest, ShouldGetJosaType)
{
    // Arrange & Act & Assert
    EXPECT_EQ(JosaRecognizer::getType("을"), JosaRecognizer::JosaType::EUL_REUL);
    EXPECT_EQ(JosaRecognizer::getType("를"), JosaRecognizer::JosaType::EUL_REUL);
    EXPECT_EQ(JosaRecognizer::getType("이"), JosaRecognizer::JosaType::I_GA);
    EXPECT_EQ(JosaRecognizer::getType("가"), JosaRecognizer::JosaType::I_GA);
    EXPECT_EQ(JosaRecognizer::getType("은"), JosaRecognizer::JosaType::EUN_NEUN);
    EXPECT_EQ(JosaRecognizer::getType("는"), JosaRecognizer::JosaType::EUN_NEUN);
    EXPECT_EQ(JosaRecognizer::getType("의"), JosaRecognizer::JosaType::UI);
    EXPECT_EQ(JosaRecognizer::getType("로"), JosaRecognizer::JosaType::RO_EURO);
    EXPECT_EQ(JosaRecognizer::getType("으로"), JosaRecognizer::JosaType::RO_EURO);
    EXPECT_EQ(JosaRecognizer::getType("에서"), JosaRecognizer::JosaType::ESO);
    EXPECT_EQ(JosaRecognizer::getType("에"), JosaRecognizer::JosaType::E);
}

/**
 * @brief getType 함수 잘못된 입력 테스트
 */
TEST_F(JosaRecognizerTest, ShouldThrowOnInvalidJosa)
{
    // Arrange & Act & Assert
    EXPECT_THROW(JosaRecognizer::getType("사과"), std::invalid_argument);
    EXPECT_THROW(JosaRecognizer::getType("만약"), std::invalid_argument);
}

/**
 * @brief josaTypeToString 함수 테스트
 */
TEST_F(JosaRecognizerTest, ShouldConvertJosaTypeToString)
{
    // Arrange & Act & Assert
    EXPECT_EQ(JosaRecognizer::josaTypeToString(JosaRecognizer::JosaType::EUL_REUL), "EUL_REUL");
    EXPECT_EQ(JosaRecognizer::josaTypeToString(JosaRecognizer::JosaType::I_GA), "I_GA");
    EXPECT_EQ(JosaRecognizer::josaTypeToString(JosaRecognizer::JosaType::EUN_NEUN), "EUN_NEUN");
    EXPECT_EQ(JosaRecognizer::josaTypeToString(JosaRecognizer::JosaType::UI), "UI");
    EXPECT_EQ(JosaRecognizer::josaTypeToString(JosaRecognizer::JosaType::RO_EURO), "RO_EURO");
    EXPECT_EQ(JosaRecognizer::josaTypeToString(JosaRecognizer::JosaType::ESO), "ESO");
    EXPECT_EQ(JosaRecognizer::josaTypeToString(JosaRecognizer::JosaType::E), "E");
}
