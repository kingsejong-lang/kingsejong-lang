#pragma once

/**
 * @file HotPathDetector.h
 * @brief JIT 컴파일을 위한 핫 패스 감지기
 * @author KingSejong Team
 * @date 2025-11-10
 *
 * 자주 실행되는 코드 경로를 감지하여 JIT 컴파일 대상을 식별합니다.
 * F4.8 JIT 컴파일러 연구의 일환으로 구현되었습니다.
 */

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
#include <chrono>

namespace kingsejong {
namespace jit {

/**
 * @enum HotPathType
 * @brief 핫 패스 타입
 */
enum class HotPathType {
    FUNCTION,       ///< 함수 호출
    LOOP,           ///< 루프 백엣지
    BASIC_BLOCK     ///< 기본 블록
};

/**
 * @enum JITTier
 * @brief JIT 컴파일 티어
 */
enum class JITTier {
    NONE,           ///< 컴파일되지 않음
    TIER_1,         ///< 빠른 컴파일 (템플릿 JIT)
    TIER_2          ///< 고급 컴파일 (LLVM JIT)
};

/**
 * @struct ExecutionProfile
 * @brief 실행 프로파일 정보
 */
struct ExecutionProfile {
    uint64_t executionCount = 0;        ///< 실행 횟수
    uint64_t totalTime = 0;             ///< 총 실행 시간 (microseconds)
    uint64_t avgTime = 0;               ///< 평균 실행 시간 (microseconds)
    HotPathType type;                   ///< 타입
    JITTier jitTier = JITTier::NONE;   ///< JIT 티어
    std::string name;                   ///< 이름 (디버깅용)

    /**
     * @brief 실행 추적
     * @param duration 실행 시간 (microseconds)
     */
    void track(uint64_t duration) {
        executionCount++;
        totalTime += duration;
        avgTime = totalTime / executionCount;
    }

    /**
     * @brief 핫 패스인지 확인
     * @param threshold 임계값
     * @return 핫 패스 여부
     */
    bool isHot(uint64_t threshold) const {
        return executionCount >= threshold;
    }

    /**
     * @brief 매우 핫한 패스인지 확인
     * @param threshold 임계값
     * @return 매우 핫한 패스 여부
     */
    bool isVeryHot(uint64_t threshold) const {
        return executionCount >= threshold * 10;
    }
};

/**
 * @class HotPathDetector
 * @brief 핫 패스 감지기
 *
 * 자주 실행되는 코드 경로를 추적하고 JIT 컴파일 대상을 식별합니다.
 *
 * 사용 예시:
 * @code
 * HotPathDetector detector;
 * detector.setFunctionThreshold(1000);
 * detector.setLoopThreshold(10000);
 *
 * // VM 실행 중
 * detector.trackFunctionCall("fibonacci", functionId);
 * if (detector.isHot(functionId, HotPathType::FUNCTION)) {
 *     // JIT 컴파일 트리거
 * }
 * @endcode
 */
class HotPathDetector {
public:
    /**
     * @brief 핫 패스 감지기 생성자
     */
    HotPathDetector();

    /**
     * @brief 함수 호출 추적
     * @param name 함수 이름
     * @param id 함수 ID (바이트코드 오프셋)
     * @param duration 실행 시간 (microseconds, 기본값 0)
     */
    void trackFunctionCall(const std::string& name, size_t id, uint64_t duration = 0);

    /**
     * @brief 루프 백엣지 추적
     * @param loopId 루프 ID
     * @param duration 실행 시간 (microseconds, 기본값 0)
     */
    void trackLoopBackedge(size_t loopId, uint64_t duration = 0);

    /**
     * @brief 기본 블록 추적
     * @param blockId 블록 ID
     */
    void trackBasicBlock(size_t blockId);

    /**
     * @brief 핫 패스인지 확인
     * @param id ID
     * @param type 타입
     * @return 핫 패스 여부
     */
    bool isHot(size_t id, HotPathType type) const;

    /**
     * @brief 매우 핫한 패스인지 확인
     * @param id ID
     * @param type 타입
     * @return 매우 핫한 패스 여부
     */
    bool isVeryHot(size_t id, HotPathType type) const;

    /**
     * @brief JIT 컴파일 완료 표시
     * @param id ID
     * @param type 타입
     * @param tier JIT 티어
     */
    void markJITCompiled(size_t id, HotPathType type, JITTier tier);

    /**
     * @brief 핫 함수 목록 반환
     * @return 핫 함수 ID 목록
     */
    std::vector<size_t> getHotFunctions() const;

    /**
     * @brief 핫 루프 목록 반환
     * @return 핫 루프 ID 목록
     */
    std::vector<size_t> getHotLoops() const;

    /**
     * @brief 실행 프로파일 반환
     * @param id ID
     * @param type 타입
     * @return 실행 프로파일
     */
    const ExecutionProfile* getProfile(size_t id, HotPathType type) const;

    /**
     * @brief 모든 프로파일 반환
     * @return 프로파일 맵
     */
    const std::unordered_map<size_t, ExecutionProfile>& getFunctionProfiles() const {
        return functionProfiles_;
    }

    const std::unordered_map<size_t, ExecutionProfile>& getLoopProfiles() const {
        return loopProfiles_;
    }

    /**
     * @brief 함수 임계값 설정
     * @param threshold 임계값 (기본값: 1000)
     */
    void setFunctionThreshold(uint64_t threshold) {
        functionThreshold_ = threshold;
    }

    /**
     * @brief 루프 임계값 설정
     * @param threshold 임계값 (기본값: 10000)
     */
    void setLoopThreshold(uint64_t threshold) {
        loopThreshold_ = threshold;
    }

    /**
     * @brief 함수 임계값 반환
     * @return 임계값
     */
    uint64_t getFunctionThreshold() const { return functionThreshold_; }

    /**
     * @brief 루프 임계값 반환
     * @return 임계값
     */
    uint64_t getLoopThreshold() const { return loopThreshold_; }

    /**
     * @brief 프로파일 리셋
     */
    void reset();

    /**
     * @brief 통계 출력
     */
    void printStatistics() const;

    /**
     * @brief 핫 패스 보고서 출력
     * @param topN 상위 N개 (기본값: 10)
     */
    void printHotPaths(size_t topN = 10) const;

    /**
     * @brief 프로파일 활성화 여부
     * @param enabled true면 프로파일링 활성화
     */
    void setEnabled(bool enabled) { enabled_ = enabled; }

    /**
     * @brief 프로파일 활성화 여부 반환
     * @return 활성화 여부
     */
    bool isEnabled() const { return enabled_; }

private:
    bool enabled_;                                              ///< 프로파일링 활성화 여부
    uint64_t functionThreshold_;                                ///< 함수 핫 패스 임계값
    uint64_t loopThreshold_;                                    ///< 루프 핫 패스 임계값
    uint64_t basicBlockThreshold_;                              ///< 기본 블록 임계값

    std::unordered_map<size_t, ExecutionProfile> functionProfiles_;   ///< 함수 프로파일
    std::unordered_map<size_t, ExecutionProfile> loopProfiles_;       ///< 루프 프로파일
    std::unordered_map<size_t, ExecutionProfile> blockProfiles_;      ///< 블록 프로파일

    /**
     * @brief 프로파일 맵 반환
     * @param type 타입
     * @return 프로파일 맵
     */
    std::unordered_map<size_t, ExecutionProfile>& getProfileMap(HotPathType type);

    /**
     * @brief 프로파일 맵 반환 (const)
     * @param type 타입
     * @return 프로파일 맵
     */
    const std::unordered_map<size_t, ExecutionProfile>& getProfileMap(HotPathType type) const;

    /**
     * @brief 임계값 반환
     * @param type 타입
     * @return 임계값
     */
    uint64_t getThreshold(HotPathType type) const;
};

/**
 * @class ScopedProfiler
 * @brief 스코프 기반 프로파일러
 *
 * RAII 패턴을 사용하여 함수/루프 실행 시간을 자동으로 추적합니다.
 *
 * 사용 예시:
 * @code
 * {
 *     ScopedProfiler profiler(detector, functionId, HotPathType::FUNCTION);
 *     // 함수 실행
 * } // 자동으로 실행 시간이 기록됨
 * @endcode
 */
class ScopedProfiler {
public:
    /**
     * @brief 스코프 프로파일러 생성자
     * @param detector 핫 패스 감지기
     * @param id ID
     * @param type 타입
     * @param name 이름 (선택사항)
     */
    ScopedProfiler(HotPathDetector& detector, size_t id, HotPathType type,
                   const std::string& name = "");

    /**
     * @brief 스코프 프로파일러 소멸자
     */
    ~ScopedProfiler();

    // 복사 방지
    ScopedProfiler(const ScopedProfiler&) = delete;
    ScopedProfiler& operator=(const ScopedProfiler&) = delete;

private:
    HotPathDetector& detector_;
    size_t id_;
    HotPathType type_;
    std::string name_;
    std::chrono::steady_clock::time_point startTime_;
};

} // namespace jit
} // namespace kingsejong
