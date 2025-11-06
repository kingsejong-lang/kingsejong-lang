#include <gtest/gtest.h>

/**
 * @file SanityTest.cpp
 * @brief GoogleTest 프레임워크 동작 확인을 위한 기본 테스트
 */

namespace kingsejong {
namespace test {

/**
 * @brief 기본 테스트 픽스처
 */
class SanityTest : public ::testing::Test
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
 * @brief GoogleTest 프레임워크가 정상적으로 동작하는지 확인
 */
TEST_F(SanityTest, GoogleTestFrameworkWorks)
{
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
    EXPECT_EQ(1, 1);
    EXPECT_NE(1, 2);
}

/**
 * @brief 기본 산술 연산 테스트
 */
TEST_F(SanityTest, BasicArithmeticWorks)
{
    // Arrange
    int a = 5;
    int b = 3;

    // Act
    int sum = a + b;
    int diff = a - b;
    int product = a * b;

    // Assert
    EXPECT_EQ(sum, 8);
    EXPECT_EQ(diff, 2);
    EXPECT_EQ(product, 15);
}

/**
 * @brief 문자열 비교 테스트
 */
TEST_F(SanityTest, StringComparisonWorks)
{
    // Arrange
    std::string str1 = "Hello";
    std::string str2 = "World";
    std::string str3 = "Hello";

    // Assert
    EXPECT_EQ(str1, str3);
    EXPECT_NE(str1, str2);
    EXPECT_STREQ(str1.c_str(), "Hello");
}

/**
 * @brief 예외 처리 테스트
 */
TEST_F(SanityTest, ExceptionHandlingWorks)
{
    // Assert - 예외가 발생하는지 확인
    EXPECT_THROW(
        {
            throw std::runtime_error("Test exception");
        },
        std::runtime_error
    );

    // Assert - 예외가 발생하지 않는지 확인
    EXPECT_NO_THROW(
        {
            int x = 1 + 1;
            (void)x;  // 경고 방지
        }
    );
}

/**
 * @brief nullptr 테스트
 */
TEST_F(SanityTest, NullptrCheckWorks)
{
    // Arrange
    int* ptr1 = nullptr;
    int value = 42;
    int* ptr2 = &value;

    // Assert
    EXPECT_EQ(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
}

/**
 * @brief 부동소수점 비교 테스트
 */
TEST_F(SanityTest, FloatingPointComparisonWorks)
{
    // Arrange
    double a = 0.1 + 0.2;
    double b = 0.3;

    // Assert - 부동소수점은 EXPECT_DOUBLE_EQ 사용
    EXPECT_DOUBLE_EQ(a, b);

    // Assert - 근사값 비교
    EXPECT_NEAR(a, b, 0.0001);
}

} // namespace test
} // namespace kingsejong
