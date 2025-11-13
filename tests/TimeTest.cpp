/**
 * @file TimeTest.cpp
 * @brief 시간/날짜 처리 내장 함수 테스트
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * 날짜/시간 함수 테스트
 */

#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "evaluator/Evaluator.h"
#include "evaluator/Builtin.h"
#include "module/ModuleLoader.h"
#include <chrono>
#include <thread>

using namespace kingsejong;
using namespace kingsejong::evaluator;

class TimeTest : public ::testing::Test
{
protected:
    std::shared_ptr<module::ModuleLoader> moduleLoader_;

    void SetUp() override
    {
        Builtin::registerAllBuiltins();

        // ModuleLoader 설정 (현재 디렉토리에서 stdlib 찾기)
        moduleLoader_ = std::make_shared<module::ModuleLoader>(".");
    }

    Value evaluate(const std::string& code)
    {
        lexer::Lexer lexer(code);
        parser::Parser parser(lexer);
        auto program = parser.parseProgram();

        Evaluator evaluator;
        evaluator.setModuleLoader(moduleLoader_.get());

        return evaluator.evalProgram(program.get());
    }
};

// ============================================================================
// 현재 시간/날짜 함수 테스트
// ============================================================================

TEST_F(TimeTest, ShouldReturnCurrentTime)
{
    // Arrange & Act
    Value result = evaluate("현재_시간()");

    // Assert - Unix timestamp는 정수여야 함
    EXPECT_TRUE(result.isInteger());

    // 2025년 이후의 타임스탬프여야 함 (대략 1735689600 = 2025-01-01)
    EXPECT_GT(result.asInteger(), 1735689600);
}

TEST_F(TimeTest, ShouldReturnCurrentDate)
{
    // Arrange & Act
    Value result = evaluate("현재_날짜()");

    // Assert - YYYY-MM-DD 형식의 문자열
    EXPECT_TRUE(result.isString());
    std::string date = result.asString();

    // 형식 검증: YYYY-MM-DD (10자)
    EXPECT_EQ(date.length(), 10u);
    EXPECT_EQ(date[4], '-');
    EXPECT_EQ(date[7], '-');
}

TEST_F(TimeTest, ShouldReturnCurrentTimeOfDay)
{
    // Arrange & Act
    Value result = evaluate("현재_시각()");

    // Assert - HH:MM:SS 형식의 문자열
    EXPECT_TRUE(result.isString());
    std::string time = result.asString();

    // 형식 검증: HH:MM:SS (8자)
    EXPECT_EQ(time.length(), 8u);
    EXPECT_EQ(time[2], ':');
    EXPECT_EQ(time[5], ':');
}

TEST_F(TimeTest, ShouldReturnCurrentDateTime)
{
    // Arrange & Act
    Value result = evaluate("현재_날짜시간()");

    // Assert - YYYY-MM-DD HH:MM:SS 형식
    EXPECT_TRUE(result.isString());
    std::string datetime = result.asString();

    // 형식 검증: YYYY-MM-DD HH:MM:SS (19자)
    EXPECT_EQ(datetime.length(), 19u);
    EXPECT_EQ(datetime[4], '-');
    EXPECT_EQ(datetime[7], '-');
    EXPECT_EQ(datetime[10], ' ');
    EXPECT_EQ(datetime[13], ':');
    EXPECT_EQ(datetime[16], ':');
}

TEST_F(TimeTest, ShouldReturnTimestamp)
{
    // Arrange & Act
    Value result = evaluate("타임스탬프()");

    // Assert - 밀리초 단위 타임스탬프
    EXPECT_TRUE(result.isInteger());

    // 밀리초는 초 * 1000이므로 훨씬 큰 값
    EXPECT_GT(result.asInteger(), 1735689600000LL);
}

// ============================================================================
// 시간 포맷 함수 테스트
// ============================================================================

TEST_F(TimeTest, ShouldFormatTimeWithYearMonthDay)
{
    // Arrange - 2025-01-15 12:30:45 = 1736945445 (대략)
    std::string code = R"(시간_포맷(1736945445, "%Y-%m-%d"))";

    // Act
    Value result = evaluate(code);

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "2025-01-15");
}

TEST_F(TimeTest, ShouldFormatTimeWithHourMinuteSecond)
{
    // Arrange
    std::string code = R"(시간_포맷(1736945445, "%H:%M:%S"))";

    // Act
    Value result = evaluate(code);

    // Assert
    EXPECT_TRUE(result.isString());
    // 시간은 시스템 타임존에 따라 다를 수 있으므로 형식만 검증
    std::string time = result.asString();
    EXPECT_EQ(time.length(), 8u);
    EXPECT_EQ(time[2], ':');
    EXPECT_EQ(time[5], ':');
}

TEST_F(TimeTest, ShouldFormatTimeWithCustomFormat)
{
    // Arrange - 년도만 추출
    std::string code = R"(시간_포맷(1736945445, "%Y"))";

    // Act
    Value result = evaluate(code);

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "2025");
}

TEST_F(TimeTest, ShouldThrowOnInvalidFormatArguments)
{
    // Arrange - 타임스탬프가 문자열이면 에러
    std::string code = R"(시간_포맷("not a number", "%Y"))";

    // Act & Assert
    EXPECT_THROW(evaluate(code), std::runtime_error);
}

// ============================================================================
// 슬립 함수 테스트
// ============================================================================

TEST_F(TimeTest, ShouldSleepForSpecifiedTime)
{
    // Arrange
    auto start = std::chrono::steady_clock::now();

    // Act - 100ms 대기
    evaluate("슬립(100)");

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Assert - 최소 100ms 이상 대기했어야 함
    EXPECT_GE(duration, 100);
    // 최대 200ms 이내 (너무 오래 대기하지 않았는지)
    EXPECT_LE(duration, 200);
}

TEST_F(TimeTest, ShouldThrowOnNegativeSleep)
{
    // Arrange
    std::string code = "슬립(-100)";

    // Act & Assert
    EXPECT_THROW(evaluate(code), std::runtime_error);
}

TEST_F(TimeTest, ShouldAcceptZeroSleep)
{
    // Arrange & Act - 0ms 대기는 허용
    EXPECT_NO_THROW(evaluate("슬립(0)"));
}

// ============================================================================
// 통합 테스트
// ============================================================================

TEST_F(TimeTest, ShouldCalculateTimeDifference)
{
    // Arrange - stdlib/time.ksj 로드 필요
    std::string code = R"(
        가져오기 "stdlib/time"
        시간1 = 1736945445
        시간2 = 1736941845
        차이 = 시간_차이(시간1, 시간2)
        차이
    )";

    // Act
    Value result = evaluate(code);

    // Assert - 차이는 3600초 (1시간)
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 3600);
}

TEST_F(TimeTest, ShouldAddDaysToTimestamp)
{
    // Arrange
    std::string code = R"(
        가져오기 "stdlib/time"
        기준 = 1736945445
        이틀후 = 날짜_더하기(기준, 2)
        이틀후 - 기준
    )";

    // Act
    Value result = evaluate(code);

    // Assert - 2일 = 172800초
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 172800);
}

TEST_F(TimeTest, ShouldSubtractDaysFromTimestamp)
{
    // Arrange
    std::string code = R"(
        가져오기 "stdlib/time"
        기준 = 1736945445
        이틀전 = 날짜_빼기(기준, 2)
        기준 - 이틀전
    )";

    // Act
    Value result = evaluate(code);

    // Assert - 2일 = 172800초
    EXPECT_TRUE(result.isInteger());
    EXPECT_EQ(result.asInteger(), 172800);
}

TEST_F(TimeTest, ShouldExtractYearFromTimestamp)
{
    // Arrange
    std::string code = R"(
        가져오기 "stdlib/time"
        년 = 년_추출(1736945445)
        년
    )";

    // Act
    Value result = evaluate(code);

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "2025");
}

TEST_F(TimeTest, ShouldFormatTimeAsISO)
{
    // Arrange
    std::string code = R"(
        가져오기 "stdlib/time"
        iso = ISO_날짜(1736945445)
        iso
    )";

    // Act
    Value result = evaluate(code);

    // Assert
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "2025-01-15");
}

TEST_F(TimeTest, ShouldCheckIfTimestampIsPast)
{
    // Arrange - 2020년 타임스탬프 (과거)
    std::string code = R"(
        가져오기 "stdlib/time"
        과거 = 과거_확인(1577836800)
        과거
    )";

    // Act
    Value result = evaluate(code);

    // Assert
    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}

TEST_F(TimeTest, ShouldCheckIfTimestampIsFuture)
{
    // Arrange - 2099년 타임스탬프 (미래)
    std::string code = R"(
        가져오기 "stdlib/time"
        미래 = 미래_확인(4102444800)
        미래
    )";

    // Act
    Value result = evaluate(code);

    // Assert
    EXPECT_TRUE(result.isBoolean());
    EXPECT_TRUE(result.asBoolean());
}
