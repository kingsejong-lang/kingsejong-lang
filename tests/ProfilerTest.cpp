/**
 * @file ProfilerTest.cpp
 * @brief Profiler 테스트
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include <gtest/gtest.h>
#include "profiler/Profiler.h"
#include <thread>
#include <chrono>

using namespace kingsejong::profiler;

/**
 * @test 프로파일러 기본 기능
 */
TEST(ProfilerTest, ShouldProfileFunctionCall)
{
    Profiler profiler;
    profiler.start();

    profiler.enterFunction("test_function");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    profiler.exitFunction("test_function");

    const auto& stats = profiler.getStats();
    ASSERT_EQ(stats.size(), 1);
    ASSERT_TRUE(stats.find("test_function") != stats.end());

    const auto& funcStats = stats.at("test_function");
    EXPECT_EQ(funcStats.callCount, 1);
    EXPECT_GE(funcStats.totalTime, 10.0);  // 최소 10ms
    EXPECT_GE(funcStats.avgTime, 10.0);
}

/**
 * @test 여러 번 호출
 */
TEST(ProfilerTest, ShouldTrackMultipleCalls)
{
    Profiler profiler;
    profiler.start();

    for (int i = 0; i < 5; i++)
    {
        profiler.enterFunction("repeated_function");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        profiler.exitFunction("repeated_function");
    }

    const auto& stats = profiler.getStats();
    ASSERT_EQ(stats.size(), 1);

    const auto& funcStats = stats.at("repeated_function");
    EXPECT_EQ(funcStats.callCount, 5);
    EXPECT_GE(funcStats.totalTime, 5.0);
}

/**
 * @test 여러 함수 프로파일링
 */
TEST(ProfilerTest, ShouldProfileMultipleFunctions)
{
    Profiler profiler;
    profiler.start();

    profiler.enterFunction("func1");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    profiler.exitFunction("func1");

    profiler.enterFunction("func2");
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    profiler.exitFunction("func2");

    const auto& stats = profiler.getStats();
    ASSERT_EQ(stats.size(), 2);
    EXPECT_TRUE(stats.find("func1") != stats.end());
    EXPECT_TRUE(stats.find("func2") != stats.end());
}

/**
 * @test 중첩 함수 호출
 */
TEST(ProfilerTest, ShouldHandleNestedCalls)
{
    Profiler profiler;
    profiler.start();

    profiler.enterFunction("outer");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    profiler.enterFunction("inner");
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    profiler.exitFunction("inner");

    profiler.exitFunction("outer");

    const auto& stats = profiler.getStats();
    ASSERT_EQ(stats.size(), 2);
    EXPECT_EQ(stats.at("outer").callCount, 1);
    EXPECT_EQ(stats.at("inner").callCount, 1);
}

/**
 * @test ScopedProfiler RAII
 */
TEST(ProfilerTest, ScopedProfilerShouldWorkWithRAII)
{
    Profiler profiler;
    profiler.start();

    {
        ScopedProfiler scoped(&profiler, "scoped_function");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    const auto& stats = profiler.getStats();
    ASSERT_EQ(stats.size(), 1);
    EXPECT_EQ(stats.at("scoped_function").callCount, 1);
}

/**
 * @test 프로파일러 비활성화
 */
TEST(ProfilerTest, ShouldNotProfileWhenDisabled)
{
    Profiler profiler;
    // start()를 호출하지 않음 (비활성화 상태)

    profiler.enterFunction("test");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    profiler.exitFunction("test");

    const auto& stats = profiler.getStats();
    EXPECT_EQ(stats.size(), 0);  // 프로파일링되지 않음
}

/**
 * @test reset 기능
 */
TEST(ProfilerTest, ShouldResetStats)
{
    Profiler profiler;
    profiler.start();

    profiler.enterFunction("test");
    profiler.exitFunction("test");

    EXPECT_EQ(profiler.getStats().size(), 1);

    profiler.reset();
    EXPECT_EQ(profiler.getStats().size(), 0);
}

/**
 * @test JSON 보고서 생성
 */
TEST(ProfilerTest, ShouldGenerateJSONReport)
{
    Profiler profiler;
    profiler.start();

    profiler.enterFunction("test");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    profiler.exitFunction("test");

    std::string json = profiler.getReportJSON();
    EXPECT_TRUE(json.find("\"name\": \"test\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"callCount\": 1") != std::string::npos);
    EXPECT_TRUE(json.find("\"totalTime\"") != std::string::npos);
}
