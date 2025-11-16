#pragma once

/**
 * @file Profiler.h
 * @brief KingSejong 언어 프로파일러
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <memory>

namespace kingsejong {
namespace profiler {

/**
 * @struct FunctionStats
 * @brief 함수 실행 통계
 */
struct FunctionStats
{
    std::string name;           ///< 함수 이름
    uint64_t callCount;         ///< 호출 횟수
    double totalTime;           ///< 총 실행 시간 (ms)
    double minTime;             ///< 최소 실행 시간 (ms)
    double maxTime;             ///< 최대 실행 시간 (ms)
    double avgTime;             ///< 평균 실행 시간 (ms)

    FunctionStats()
        : callCount(0), totalTime(0.0), minTime(1e9), maxTime(0.0), avgTime(0.0)
    {
    }

    void addCall(double duration)
    {
        callCount++;
        totalTime += duration;
        minTime = std::min(minTime, duration);
        maxTime = std::max(maxTime, duration);
        avgTime = totalTime / callCount;
    }
};

/**
 * @class Profiler
 * @brief 프로파일러 - 함수 실행 시간 및 호출 횟수 측정
 */
class Profiler
{
public:
    /**
     * @brief 프로파일러 생성자
     */
    Profiler();

    /**
     * @brief 프로파일러 소멸자
     */
    ~Profiler();

    /**
     * @brief 프로파일링 시작
     */
    void start();

    /**
     * @brief 프로파일링 종료
     */
    void stop();

    /**
     * @brief 프로파일링 활성화 여부
     */
    bool isEnabled() const { return enabled_; }

    /**
     * @brief 함수 진입
     * @param functionName 함수 이름
     */
    void enterFunction(const std::string& functionName);

    /**
     * @brief 함수 탈출
     * @param functionName 함수 이름
     */
    void exitFunction(const std::string& functionName);

    /**
     * @brief 모든 통계 가져오기
     * @return 함수별 통계 맵
     */
    const std::unordered_map<std::string, FunctionStats>& getStats() const
    {
        return stats_;
    }

    /**
     * @brief 통계 초기화
     */
    void reset();

    /**
     * @brief 보고서 출력
     * @param detailed 상세 보고서 여부
     */
    void printReport(bool detailed = false) const;

    /**
     * @brief JSON 형식으로 보고서 출력
     */
    std::string getReportJSON() const;

private:
    bool enabled_;                                              ///< 프로파일링 활성화 여부
    std::unordered_map<std::string, FunctionStats> stats_;      ///< 함수별 통계

    // 함수 호출 스택 (중첩 호출 추적)
    struct CallInfo
    {
        std::string functionName;
        std::chrono::high_resolution_clock::time_point startTime;
    };
    std::vector<CallInfo> callStack_;                           ///< 호출 스택
};

/**
 * @class ScopedProfiler
 * @brief RAII 기반 함수 프로파일링
 *
 * 함수 진입 시 자동으로 타이머 시작, 탈출 시 자동으로 종료
 */
class ScopedProfiler
{
public:
    ScopedProfiler(Profiler* profiler, const std::string& functionName)
        : profiler_(profiler), functionName_(functionName)
    {
        if (profiler_ && profiler_->isEnabled())
        {
            profiler_->enterFunction(functionName_);
        }
    }

    ~ScopedProfiler()
    {
        if (profiler_ && profiler_->isEnabled())
        {
            profiler_->exitFunction(functionName_);
        }
    }

private:
    Profiler* profiler_;
    std::string functionName_;
};

} // namespace profiler
} // namespace kingsejong
