/**
 * @file jit_hotpath_demo.cpp
 * @brief HotPathDetector 사용 예제
 * @author KingSejong Team
 * @date 2025-11-10
 *
 * 이 예제는 HotPathDetector를 사용하여 자주 실행되는 코드 경로를
 * 추적하고 JIT 컴파일 대상을 식별하는 방법을 보여줍니다.
 */

#include <iostream>
#include <chrono>
#include "jit/HotPathDetector.h"

using namespace kingsejong::jit;
using namespace std::chrono;

// 시뮬레이션: 함수 실행
void simulateFunctionExecution(HotPathDetector& detector,
                                const std::string& funcName,
                                size_t funcId,
                                int executionCount) {
    std::cout << "\n[" << funcName << "] 실행 시작 (횟수: " << executionCount << ")\n";

    for (int i = 0; i < executionCount; i++) {
        // 스코프 프로파일러 사용 - 자동으로 실행 시간 추적
        ScopedProfiler profiler(detector, funcId, HotPathType::FUNCTION, funcName);

        // 실제 함수 실행 시뮬레이션 (간단한 계산)
        volatile int result = 0;
        for (int j = 0; j < 1000; j++) {
            result += j * 2;
        }

        // 핫 패스 감지 확인
        if (i == 0 || i == executionCount / 2 || i == executionCount - 1) {
            if (detector.isHot(funcId, HotPathType::FUNCTION)) {
                std::cout << "  반복 " << (i + 1) << ": 🔥 HOT (JIT 컴파일 추천)\n";
            } else if (detector.isVeryHot(funcId, HotPathType::FUNCTION)) {
                std::cout << "  반복 " << (i + 1) << ": 🔥🔥 VERY HOT (고급 최적화 추천)\n";
            }
        }
    }

    // 최종 상태
    if (detector.isHot(funcId, HotPathType::FUNCTION)) {
        std::cout << "  최종 상태: 🔥 HOT - JIT 컴파일 대상\n";

        // JIT 컴파일 시뮬레이션
        if (detector.isVeryHot(funcId, HotPathType::FUNCTION)) {
            std::cout << "  → Tier 2 (LLVM JIT) 컴파일\n";
            detector.markJITCompiled(funcId, HotPathType::FUNCTION, JITTier::TIER_2);
        } else {
            std::cout << "  → Tier 1 (템플릿 JIT) 컴파일\n";
            detector.markJITCompiled(funcId, HotPathType::FUNCTION, JITTier::TIER_1);
        }
    } else {
        std::cout << "  최종 상태: ❄️ COLD - 인터프리터 실행\n";
    }
}

// 시뮬레이션: 루프 실행
void simulateLoopExecution(HotPathDetector& detector,
                            size_t loopId,
                            int iterations) {
    std::cout << "\n[루프 " << loopId << "] 실행 시작 (반복 횟수: " << iterations << ")\n";

    for (int i = 0; i < iterations; i++) {
        // 루프 백엣지 추적
        detector.trackLoopBackedge(loopId);

        // 간단한 계산
        volatile int x = i * 2;
        (void)x;
    }

    if (detector.isHot(loopId, HotPathType::LOOP)) {
        std::cout << "  최종 상태: 🔥 HOT - JIT 컴파일 대상\n";
        detector.markJITCompiled(loopId, HotPathType::LOOP, JITTier::TIER_1);
    } else {
        std::cout << "  최종 상태: ❄️ COLD - 인터프리터 실행\n";
    }
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "   KingSejong HotPath Detector 데모\n";
    std::cout << "===========================================\n\n";

    // HotPathDetector 생성
    HotPathDetector detector;

    // 임계값 설정
    detector.setFunctionThreshold(100);   // 함수: 100회 이상
    detector.setLoopThreshold(1000);      // 루프: 1000회 이상

    std::cout << "설정:\n";
    std::cout << "  함수 임계값: " << detector.getFunctionThreshold() << "\n";
    std::cout << "  루프 임계값: " << detector.getLoopThreshold() << "\n";

    // --- 시나리오 1: 자주 호출되는 함수 ---
    std::cout << "\n=== 시나리오 1: 자주 호출되는 함수 ===\n";
    simulateFunctionExecution(detector, "fibonacci", 1, 150);

    // --- 시나리오 2: 가끔 호출되는 함수 ---
    std::cout << "\n=== 시나리오 2: 가끔 호출되는 함수 ===\n";
    simulateFunctionExecution(detector, "print_hello", 2, 50);

    // --- 시나리오 3: 매우 자주 호출되는 함수 ---
    std::cout << "\n=== 시나리오 3: 매우 자주 호출되는 함수 ===\n";
    simulateFunctionExecution(detector, "inner_loop_calculation", 3, 2000);

    // --- 시나리오 4: 긴 루프 ---
    std::cout << "\n=== 시나리오 4: 긴 루프 ===\n";
    simulateLoopExecution(detector, 1, 50000);

    // --- 시나리오 5: 짧은 루프 ---
    std::cout << "\n=== 시나리오 5: 짧은 루프 ===\n";
    simulateLoopExecution(detector, 2, 500);

    // 통계 출력
    std::cout << "\n===========================================\n";
    detector.printStatistics();

    // 핫 패스 보고서
    detector.printHotPaths(10);

    // 핫 함수 목록
    std::cout << "핫 함수 ID 목록:\n";
    auto hotFunctions = detector.getHotFunctions();
    for (size_t id : hotFunctions) {
        auto profile = detector.getProfile(id, HotPathType::FUNCTION);
        std::cout << "  ID " << id << ": " << profile->name
                  << " (" << profile->executionCount << " 회)\n";
    }

    // 핫 루프 목록
    std::cout << "\n핫 루프 ID 목록:\n";
    auto hotLoops = detector.getHotLoops();
    for (size_t id : hotLoops) {
        auto profile = detector.getProfile(id, HotPathType::LOOP);
        std::cout << "  ID " << id << " (" << profile->executionCount << " 회)\n";
    }

    std::cout << "\n===========================================\n";
    std::cout << "   데모 완료\n";
    std::cout << "===========================================\n";

    return 0;
}
