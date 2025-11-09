/**
 * @file HotPathDetectorTest.cpp
 * @brief HotPathDetector 테스트
 * @author KingSejong Team
 * @date 2025-11-10
 */

#include <gtest/gtest.h>
#include "jit/HotPathDetector.h"
#include <thread>
#include <chrono>

using namespace kingsejong::jit;

class HotPathDetectorTest : public ::testing::Test {
protected:
    HotPathDetector detector;

    void SetUp() override {
        detector.reset();
        detector.setFunctionThreshold(100);
        detector.setLoopThreshold(1000);
    }
};

// 기본 기능 테스트
TEST_F(HotPathDetectorTest, BasicTracking) {
    // 함수 호출 추적
    detector.trackFunctionCall("test_func", 1);

    // 프로파일 확인
    auto profile = detector.getProfile(1, HotPathType::FUNCTION);
    ASSERT_NE(profile, nullptr);
    EXPECT_EQ(profile->executionCount, 1);
    EXPECT_EQ(profile->name, "test_func");
    EXPECT_EQ(profile->type, HotPathType::FUNCTION);
}

TEST_F(HotPathDetectorTest, MultipleExecutions) {
    // 여러 번 실행
    for (int i = 0; i < 50; i++) {
        detector.trackFunctionCall("test_func", 1);
    }

    auto profile = detector.getProfile(1, HotPathType::FUNCTION);
    ASSERT_NE(profile, nullptr);
    EXPECT_EQ(profile->executionCount, 50);
}

TEST_F(HotPathDetectorTest, HotDetection) {
    // 임계값 미만
    for (int i = 0; i < 99; i++) {
        detector.trackFunctionCall("func1", 1);
    }
    EXPECT_FALSE(detector.isHot(1, HotPathType::FUNCTION));

    // 임계값 도달
    detector.trackFunctionCall("func1", 1);
    EXPECT_TRUE(detector.isHot(1, HotPathType::FUNCTION));
}

TEST_F(HotPathDetectorTest, VeryHotDetection) {
    // 임계값의 10배
    for (int i = 0; i < 1000; i++) {
        detector.trackFunctionCall("func1", 1);
    }

    EXPECT_TRUE(detector.isHot(1, HotPathType::FUNCTION));
    EXPECT_TRUE(detector.isVeryHot(1, HotPathType::FUNCTION));
}

TEST_F(HotPathDetectorTest, LoopTracking) {
    // 루프 백엣지 추적
    for (int i = 0; i < 10000; i++) {
        detector.trackLoopBackedge(1);
    }

    auto profile = detector.getProfile(1, HotPathType::LOOP);
    ASSERT_NE(profile, nullptr);
    EXPECT_EQ(profile->executionCount, 10000);
    EXPECT_EQ(profile->type, HotPathType::LOOP);
    EXPECT_TRUE(detector.isHot(1, HotPathType::LOOP));
}

TEST_F(HotPathDetectorTest, MultipleFunctions) {
    // 여러 함수 추적
    detector.trackFunctionCall("func1", 1);
    detector.trackFunctionCall("func2", 2);
    detector.trackFunctionCall("func3", 3);

    EXPECT_EQ(detector.getFunctionProfiles().size(), 3);

    auto profile1 = detector.getProfile(1, HotPathType::FUNCTION);
    auto profile2 = detector.getProfile(2, HotPathType::FUNCTION);
    auto profile3 = detector.getProfile(3, HotPathType::FUNCTION);

    ASSERT_NE(profile1, nullptr);
    ASSERT_NE(profile2, nullptr);
    ASSERT_NE(profile3, nullptr);

    EXPECT_EQ(profile1->name, "func1");
    EXPECT_EQ(profile2->name, "func2");
    EXPECT_EQ(profile3->name, "func3");
}

TEST_F(HotPathDetectorTest, GetHotFunctions) {
    // 3개 함수, 다른 실행 횟수
    for (int i = 0; i < 200; i++) detector.trackFunctionCall("func1", 1);
    for (int i = 0; i < 50; i++) detector.trackFunctionCall("func2", 2);
    for (int i = 0; i < 150; i++) detector.trackFunctionCall("func3", 3);

    auto hotFunctions = detector.getHotFunctions();

    // func1과 func3만 핫 (100 이상)
    EXPECT_EQ(hotFunctions.size(), 2);

    // 실행 횟수 순으로 정렬되어야 함
    EXPECT_EQ(hotFunctions[0], 1); // func1: 200
    EXPECT_EQ(hotFunctions[1], 3); // func3: 150
}

TEST_F(HotPathDetectorTest, GetHotLoops) {
    // 2개 루프, 다른 실행 횟수
    for (int i = 0; i < 20000; i++) detector.trackLoopBackedge(1);
    for (int i = 0; i < 500; i++) detector.trackLoopBackedge(2);

    auto hotLoops = detector.getHotLoops();

    // 루프 1만 핫 (1000 이상)
    EXPECT_EQ(hotLoops.size(), 1);
    EXPECT_EQ(hotLoops[0], 1);
}

TEST_F(HotPathDetectorTest, JITCompilationMarking) {
    detector.trackFunctionCall("func1", 1);

    auto profile = detector.getProfile(1, HotPathType::FUNCTION);
    EXPECT_EQ(profile->jitTier, JITTier::NONE);

    // Tier 1로 표시
    detector.markJITCompiled(1, HotPathType::FUNCTION, JITTier::TIER_1);
    profile = detector.getProfile(1, HotPathType::FUNCTION);
    EXPECT_EQ(profile->jitTier, JITTier::TIER_1);

    // Tier 2로 업그레이드
    detector.markJITCompiled(1, HotPathType::FUNCTION, JITTier::TIER_2);
    profile = detector.getProfile(1, HotPathType::FUNCTION);
    EXPECT_EQ(profile->jitTier, JITTier::TIER_2);
}

TEST_F(HotPathDetectorTest, TimeTracking) {
    // 시간 추적
    detector.trackFunctionCall("func1", 1, 100); // 100μs
    detector.trackFunctionCall("func1", 1, 200); // 200μs

    auto profile = detector.getProfile(1, HotPathType::FUNCTION);
    ASSERT_NE(profile, nullptr);
    EXPECT_EQ(profile->executionCount, 2);
    EXPECT_EQ(profile->totalTime, 300);
    EXPECT_EQ(profile->avgTime, 150);
}

TEST_F(HotPathDetectorTest, Reset) {
    // 여러 추적 수행
    detector.trackFunctionCall("func1", 1);
    detector.trackFunctionCall("func2", 2);
    detector.trackLoopBackedge(1);

    EXPECT_EQ(detector.getFunctionProfiles().size(), 2);
    EXPECT_EQ(detector.getLoopProfiles().size(), 1);

    // 리셋
    detector.reset();

    EXPECT_EQ(detector.getFunctionProfiles().size(), 0);
    EXPECT_EQ(detector.getLoopProfiles().size(), 0);
}

TEST_F(HotPathDetectorTest, EnableDisable) {
    detector.setEnabled(false);

    // 비활성화 상태에서 추적
    detector.trackFunctionCall("func1", 1);

    // 추적되지 않아야 함
    EXPECT_EQ(detector.getFunctionProfiles().size(), 0);

    // 다시 활성화
    detector.setEnabled(true);
    detector.trackFunctionCall("func1", 1);

    EXPECT_EQ(detector.getFunctionProfiles().size(), 1);
}

TEST_F(HotPathDetectorTest, ThresholdConfiguration) {
    detector.setFunctionThreshold(500);
    detector.setLoopThreshold(5000);

    EXPECT_EQ(detector.getFunctionThreshold(), 500);
    EXPECT_EQ(detector.getLoopThreshold(), 5000);

    // 새 임계값으로 핫 감지
    for (int i = 0; i < 499; i++) {
        detector.trackFunctionCall("func1", 1);
    }
    EXPECT_FALSE(detector.isHot(1, HotPathType::FUNCTION));

    detector.trackFunctionCall("func1", 1);
    EXPECT_TRUE(detector.isHot(1, HotPathType::FUNCTION));
}

// ScopedProfiler 테스트
TEST_F(HotPathDetectorTest, ScopedProfiler) {
    {
        ScopedProfiler profiler(detector, 1, HotPathType::FUNCTION, "scoped_func");
        // 약간 대기
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    } // 소멸자에서 자동으로 추적

    auto profile = detector.getProfile(1, HotPathType::FUNCTION);
    ASSERT_NE(profile, nullptr);
    EXPECT_EQ(profile->executionCount, 1);
    EXPECT_EQ(profile->name, "scoped_func");
    EXPECT_GT(profile->totalTime, 0); // 시간이 기록되어야 함
}

TEST_F(HotPathDetectorTest, MultipleScopedProfilers) {
    // 여러 번 실행
    for (int i = 0; i < 10; i++) {
        ScopedProfiler profiler(detector, 1, HotPathType::FUNCTION, "test_func");
        // 약간 작업
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

    auto profile = detector.getProfile(1, HotPathType::FUNCTION);
    ASSERT_NE(profile, nullptr);
    EXPECT_EQ(profile->executionCount, 10);
    EXPECT_GT(profile->avgTime, 0);
}

// 통계 출력 테스트 (출력 검증은 하지 않음, 크래시만 확인)
TEST_F(HotPathDetectorTest, PrintStatistics) {
    detector.trackFunctionCall("func1", 1);
    detector.trackFunctionCall("func2", 2);

    // 크래시 없이 실행되어야 함
    EXPECT_NO_THROW(detector.printStatistics());
}

TEST_F(HotPathDetectorTest, PrintHotPaths) {
    for (int i = 0; i < 200; i++) detector.trackFunctionCall("func1", 1);
    for (int i = 0; i < 150; i++) detector.trackFunctionCall("func2", 2);

    // 크래시 없이 실행되어야 함
    EXPECT_NO_THROW(detector.printHotPaths(5));
}

// 엣지 케이스
TEST_F(HotPathDetectorTest, NonExistentProfile) {
    auto profile = detector.getProfile(999, HotPathType::FUNCTION);
    EXPECT_EQ(profile, nullptr);

    EXPECT_FALSE(detector.isHot(999, HotPathType::FUNCTION));
    EXPECT_FALSE(detector.isVeryHot(999, HotPathType::FUNCTION));
}

TEST_F(HotPathDetectorTest, ZeroThreshold) {
    detector.setFunctionThreshold(0);

    // 1번만 실행해도 핫으로 간주되어야 함
    detector.trackFunctionCall("func1", 1);
    EXPECT_TRUE(detector.isHot(1, HotPathType::FUNCTION));
}

TEST_F(HotPathDetectorTest, LargeExecutionCount) {
    // 매우 많은 실행 횟수
    for (int i = 0; i < 1000000; i++) {
        detector.trackFunctionCall("func1", 1);
    }

    auto profile = detector.getProfile(1, HotPathType::FUNCTION);
    ASSERT_NE(profile, nullptr);
    EXPECT_EQ(profile->executionCount, 1000000);
}

TEST_F(HotPathDetectorTest, ManyDifferentFunctions) {
    // 많은 다른 함수
    for (int i = 0; i < 1000; i++) {
        detector.trackFunctionCall("func_" + std::to_string(i), i);
    }

    EXPECT_EQ(detector.getFunctionProfiles().size(), 1000);
}
