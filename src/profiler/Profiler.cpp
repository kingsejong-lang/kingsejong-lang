/**
 * @file Profiler.cpp
 * @brief Profiler 구현
 * @author KingSejong Team
 * @date 2025-11-16
 */

#include "profiler/Profiler.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

namespace kingsejong {
namespace profiler {

Profiler::Profiler()
    : enabled_(false)
{
}

Profiler::~Profiler()
{
}

void Profiler::start()
{
    enabled_ = true;
    reset();
}

void Profiler::stop()
{
    enabled_ = false;
}

void Profiler::enterFunction(const std::string& functionName)
{
    if (!enabled_) return;

    CallInfo info;
    info.functionName = functionName;
    info.startTime = std::chrono::high_resolution_clock::now();
    callStack_.push_back(info);
}

void Profiler::exitFunction(const std::string& functionName)
{
    if (!enabled_) return;
    if (callStack_.empty()) return;

    // 호출 스택에서 매칭되는 함수 찾기
    auto it = callStack_.rbegin();
    for (; it != callStack_.rend(); ++it)
    {
        if (it->functionName == functionName)
        {
            break;
        }
    }

    if (it == callStack_.rend()) return;

    // 실행 시간 계산
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(endTime - it->startTime);
    double durationMs = duration.count();

    // 통계 업데이트
    stats_[functionName].name = functionName;
    stats_[functionName].addCall(durationMs);

    // 스택에서 제거
    callStack_.erase(std::next(it).base());
}

void Profiler::reset()
{
    stats_.clear();
    callStack_.clear();
}

void Profiler::printReport(bool detailed) const
{
    if (stats_.empty())
    {
        std::cout << "프로파일링 데이터가 없습니다." << std::endl;
        return;
    }

    // 통계를 벡터로 변환 (정렬을 위해)
    std::vector<FunctionStats> sortedStats;
    sortedStats.reserve(stats_.size());
    for (const auto& pair : stats_)
    {
        sortedStats.push_back(pair.second);
    }

    // 총 시간 기준 내림차순 정렬
    std::sort(sortedStats.begin(), sortedStats.end(),
        [](const FunctionStats& a, const FunctionStats& b) {
            return a.totalTime > b.totalTime;
        });

    // 헤더 출력
    std::cout << "\n";
    std::cout << "========================================" << std::endl;
    std::cout << "프로파일링 보고서" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\n";

    // 테이블 헤더
    std::cout << std::left << std::setw(30) << "함수"
              << std::right << std::setw(12) << "호출 횟수"
              << std::right << std::setw(15) << "총 시간 (ms)"
              << std::right << std::setw(15) << "평균 (ms)";

    if (detailed)
    {
        std::cout << std::right << std::setw(15) << "최소 (ms)"
                  << std::right << std::setw(15) << "최대 (ms)";
    }
    std::cout << std::endl;

    std::cout << std::string(detailed ? 102 : 72, '-') << std::endl;

    // 각 함수 통계 출력
    for (const auto& stat : sortedStats)
    {
        std::cout << std::left << std::setw(30) << stat.name
                  << std::right << std::setw(12) << stat.callCount
                  << std::right << std::setw(15) << std::fixed << std::setprecision(3) << stat.totalTime
                  << std::right << std::setw(15) << std::fixed << std::setprecision(3) << stat.avgTime;

        if (detailed)
        {
            std::cout << std::right << std::setw(15) << std::fixed << std::setprecision(3) << stat.minTime
                      << std::right << std::setw(15) << std::fixed << std::setprecision(3) << stat.maxTime;
        }
        std::cout << std::endl;
    }

    std::cout << std::string(detailed ? 102 : 72, '-') << std::endl;

    // 총계
    double totalTime = 0.0;
    uint64_t totalCalls = 0;
    for (const auto& stat : sortedStats)
    {
        totalTime += stat.totalTime;
        totalCalls += stat.callCount;
    }

    std::cout << std::left << std::setw(30) << "총계"
              << std::right << std::setw(12) << totalCalls
              << std::right << std::setw(15) << std::fixed << std::setprecision(3) << totalTime
              << std::endl;

    std::cout << "\n";
}

std::string Profiler::getReportJSON() const
{
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"functions\": [\n";

    bool first = true;
    for (const auto& pair : stats_)
    {
        const FunctionStats& stat = pair.second;

        if (!first)
        {
            oss << ",\n";
        }
        first = false;

        oss << "    {\n";
        oss << "      \"name\": \"" << stat.name << "\",\n";
        oss << "      \"callCount\": " << stat.callCount << ",\n";
        oss << "      \"totalTime\": " << std::fixed << std::setprecision(3) << stat.totalTime << ",\n";
        oss << "      \"avgTime\": " << std::fixed << std::setprecision(3) << stat.avgTime << ",\n";
        oss << "      \"minTime\": " << std::fixed << std::setprecision(3) << stat.minTime << ",\n";
        oss << "      \"maxTime\": " << std::fixed << std::setprecision(3) << stat.maxTime << "\n";
        oss << "    }";
    }

    oss << "\n  ]\n";
    oss << "}\n";

    return oss.str();
}

} // namespace profiler
} // namespace kingsejong
