/**
 * @file HotPathDetector.cpp
 * @brief JIT 컴파일을 위한 핫 패스 감지기 구현
 * @author KingSejong Team
 * @date 2025-11-10
 */

#include "jit/HotPathDetector.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace kingsejong {
namespace jit {

HotPathDetector::HotPathDetector()
    : enabled_(true),
      functionThreshold_(1000),
      loopThreshold_(10000),
      basicBlockThreshold_(50000) {
}

void HotPathDetector::trackFunctionCall(const std::string& name, size_t id, uint64_t duration) {
    if (!enabled_) return;

    auto& profile = functionProfiles_[id];
    if (profile.name.empty()) {
        profile.name = name;
        profile.type = HotPathType::FUNCTION;
    }

    if (duration > 0) {
        profile.track(duration);
    } else {
        profile.executionCount++;
    }
}

void HotPathDetector::trackLoopBackedge(size_t loopId, uint64_t duration) {
    if (!enabled_) return;

    auto& profile = loopProfiles_[loopId];
    profile.type = HotPathType::LOOP;

    if (duration > 0) {
        profile.track(duration);
    } else {
        profile.executionCount++;
    }
}

void HotPathDetector::trackBasicBlock(size_t blockId) {
    if (!enabled_) return;

    auto& profile = blockProfiles_[blockId];
    profile.type = HotPathType::BASIC_BLOCK;
    profile.executionCount++;
}

bool HotPathDetector::isHot(size_t id, HotPathType type) const {
    if (!enabled_) return false;

    auto& profiles = getProfileMap(type);
    auto it = profiles.find(id);
    if (it == profiles.end()) return false;

    return it->second.isHot(getThreshold(type));
}

bool HotPathDetector::isVeryHot(size_t id, HotPathType type) const {
    if (!enabled_) return false;

    auto& profiles = getProfileMap(type);
    auto it = profiles.find(id);
    if (it == profiles.end()) return false;

    return it->second.isVeryHot(getThreshold(type));
}

void HotPathDetector::markJITCompiled(size_t id, HotPathType type, JITTier tier) {
    auto& profiles = getProfileMap(type);
    auto it = profiles.find(id);
    if (it != profiles.end()) {
        it->second.jitTier = tier;
    }
}

std::vector<size_t> HotPathDetector::getHotFunctions() const {
    std::vector<size_t> hotFunctions;

    for (const auto& [id, profile] : functionProfiles_) {
        if (profile.isHot(functionThreshold_)) {
            hotFunctions.push_back(id);
        }
    }

    // 실행 횟수로 정렬
    std::sort(hotFunctions.begin(), hotFunctions.end(),
        [this](size_t a, size_t b) {
            return functionProfiles_.at(a).executionCount >
                   functionProfiles_.at(b).executionCount;
        });

    return hotFunctions;
}

std::vector<size_t> HotPathDetector::getHotLoops() const {
    std::vector<size_t> hotLoops;

    for (const auto& [id, profile] : loopProfiles_) {
        if (profile.isHot(loopThreshold_)) {
            hotLoops.push_back(id);
        }
    }

    // 실행 횟수로 정렬
    std::sort(hotLoops.begin(), hotLoops.end(),
        [this](size_t a, size_t b) {
            return loopProfiles_.at(a).executionCount >
                   loopProfiles_.at(b).executionCount;
        });

    return hotLoops;
}

const ExecutionProfile* HotPathDetector::getProfile(size_t id, HotPathType type) const {
    auto& profiles = getProfileMap(type);
    auto it = profiles.find(id);
    return (it != profiles.end()) ? &it->second : nullptr;
}

void HotPathDetector::reset() {
    functionProfiles_.clear();
    loopProfiles_.clear();
    blockProfiles_.clear();
}

void HotPathDetector::printStatistics() const {
    std::cout << "\n=== HotPath Detector Statistics ===\n";
    std::cout << "Enabled: " << (enabled_ ? "Yes" : "No") << "\n";
    std::cout << "Function Threshold: " << functionThreshold_ << "\n";
    std::cout << "Loop Threshold: " << loopThreshold_ << "\n";
    std::cout << "\n";

    std::cout << "Total Functions Tracked: " << functionProfiles_.size() << "\n";
    std::cout << "Hot Functions: " << getHotFunctions().size() << "\n";
    std::cout << "\n";

    std::cout << "Total Loops Tracked: " << loopProfiles_.size() << "\n";
    std::cout << "Hot Loops: " << getHotLoops().size() << "\n";
    std::cout << "\n";

    std::cout << "Total Basic Blocks Tracked: " << blockProfiles_.size() << "\n";
    std::cout << "==================================\n\n";
}

void HotPathDetector::printHotPaths(size_t topN) const {
    std::cout << "\n=== Top " << topN << " Hot Paths ===\n\n";

    // 핫 함수
    std::cout << "Hot Functions:\n";
    std::cout << std::left << std::setw(15) << "ID"
              << std::setw(30) << "Name"
              << std::setw(15) << "Count"
              << std::setw(15) << "Avg Time (μs)"
              << std::setw(10) << "JIT Tier"
              << "\n";
    std::cout << std::string(85, '-') << "\n";

    auto hotFunctions = getHotFunctions();
    for (size_t i = 0; i < std::min(topN, hotFunctions.size()); i++) {
        size_t id = hotFunctions[i];
        const auto& profile = functionProfiles_.at(id);

        std::cout << std::left << std::setw(15) << id
                  << std::setw(30) << profile.name
                  << std::setw(15) << profile.executionCount
                  << std::setw(15) << profile.avgTime;

        switch (profile.jitTier) {
            case JITTier::NONE:
                std::cout << std::setw(10) << "None";
                break;
            case JITTier::TIER_1:
                std::cout << std::setw(10) << "Tier 1";
                break;
            case JITTier::TIER_2:
                std::cout << std::setw(10) << "Tier 2";
                break;
        }

        std::cout << "\n";
    }

    std::cout << "\n";

    // 핫 루프
    std::cout << "Hot Loops:\n";
    std::cout << std::left << std::setw(15) << "ID"
              << std::setw(15) << "Count"
              << std::setw(15) << "Avg Time (μs)"
              << std::setw(10) << "JIT Tier"
              << "\n";
    std::cout << std::string(55, '-') << "\n";

    auto hotLoops = getHotLoops();
    for (size_t i = 0; i < std::min(topN, hotLoops.size()); i++) {
        size_t id = hotLoops[i];
        const auto& profile = loopProfiles_.at(id);

        std::cout << std::left << std::setw(15) << id
                  << std::setw(15) << profile.executionCount
                  << std::setw(15) << profile.avgTime;

        switch (profile.jitTier) {
            case JITTier::NONE:
                std::cout << std::setw(10) << "None";
                break;
            case JITTier::TIER_1:
                std::cout << std::setw(10) << "Tier 1";
                break;
            case JITTier::TIER_2:
                std::cout << std::setw(10) << "Tier 2";
                break;
        }

        std::cout << "\n";
    }

    std::cout << "\n==========================\n\n";
}

std::unordered_map<size_t, ExecutionProfile>&
HotPathDetector::getProfileMap(HotPathType type) {
    switch (type) {
        case HotPathType::FUNCTION:
            return functionProfiles_;
        case HotPathType::LOOP:
            return loopProfiles_;
        case HotPathType::BASIC_BLOCK:
            return blockProfiles_;
    }
    return functionProfiles_; // fallback
}

const std::unordered_map<size_t, ExecutionProfile>&
HotPathDetector::getProfileMap(HotPathType type) const {
    switch (type) {
        case HotPathType::FUNCTION:
            return functionProfiles_;
        case HotPathType::LOOP:
            return loopProfiles_;
        case HotPathType::BASIC_BLOCK:
            return blockProfiles_;
    }
    return functionProfiles_; // fallback
}

uint64_t HotPathDetector::getThreshold(HotPathType type) const {
    switch (type) {
        case HotPathType::FUNCTION:
            return functionThreshold_;
        case HotPathType::LOOP:
            return loopThreshold_;
        case HotPathType::BASIC_BLOCK:
            return basicBlockThreshold_;
    }
    return functionThreshold_; // fallback
}

// ScopedProfiler

ScopedProfiler::ScopedProfiler(HotPathDetector& detector, size_t id,
                               HotPathType type, const std::string& name)
    : detector_(detector),
      id_(id),
      type_(type),
      name_(name),
      startTime_(std::chrono::steady_clock::now()) {
}

ScopedProfiler::~ScopedProfiler() {
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - startTime_
    ).count();

    switch (type_) {
        case HotPathType::FUNCTION:
            detector_.trackFunctionCall(name_, id_, duration);
            break;
        case HotPathType::LOOP:
            detector_.trackLoopBackedge(id_, duration);
            break;
        case HotPathType::BASIC_BLOCK:
            detector_.trackBasicBlock(id_);
            break;
    }
}

} // namespace jit
} // namespace kingsejong
