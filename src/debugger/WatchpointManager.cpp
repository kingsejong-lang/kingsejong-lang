/**
 * @file WatchpointManager.cpp
 * @brief 와치포인트 관리 시스템 구현
 * @author KingSejong Team
 * @date 2025-11-14
 */

#include "debugger/WatchpointManager.h"
#include <stdexcept>

namespace kingsejong {
namespace debugger {

// ============================================================================
// Public Methods
// ============================================================================

bool WatchpointManager::add(
    const std::string& variableName,
    const evaluator::Environment& env)
{
    // 변수명 유효성 검증
    validateVariableName(variableName);

    // 이미 존재하는지 확인
    if (watchpoints_.find(variableName) != watchpoints_.end()) {
        return false;  // 중복
    }

    // 변수가 환경에 존재하는지 확인
    try {
        evaluator::Value currentValue = env.get(variableName);

        // 와치포인트 추가
        watchpoints_.emplace(variableName, Watchpoint(variableName, currentValue));
        return true;

    } catch (const std::exception&) {
        // 변수가 존재하지 않음
        return false;
    }
}

bool WatchpointManager::remove(const std::string& variableName)
{
    auto it = watchpoints_.find(variableName);
    if (it == watchpoints_.end()) {
        return false;  // 존재하지 않음
    }

    watchpoints_.erase(it);
    return true;
}

std::vector<std::string> WatchpointManager::checkChanges(
    const evaluator::Environment& env)
{
    std::vector<std::string> changedVariables;

    // Strong Exception Safety: 모든 변경을 먼저 계산한 후 일괄 적용
    std::vector<std::pair<std::string, evaluator::Value>> updates;

    for (auto& [varName, watchpoint] : watchpoints_) {
        // 비활성화된 와치포인트는 무시
        if (!watchpoint.enabled) {
            continue;
        }

        try {
            // 현재 값 조회
            evaluator::Value currentValue = env.get(varName);

            // 값이 변경되었는지 확인
            if (!currentValue.equals(watchpoint.lastValue)) {
                changedVariables.push_back(varName);
                updates.emplace_back(varName, currentValue);
            }

        } catch (const std::exception&) {
            // 변수가 환경에 없는 경우 무시
            // Strong Exception Safety: 상태 변경 없음
            continue;
        }
    }

    // 변경 사항 일괄 적용
    for (const auto& [varName, newValue] : updates) {
        auto it = watchpoints_.find(varName);
        if (it != watchpoints_.end()) {
            it->second.lastValue = newValue;
            it->second.changeCount++;
        }
    }

    return changedVariables;
}

bool WatchpointManager::setEnabled(const std::string& variableName, bool enabled)
{
    auto it = watchpoints_.find(variableName);
    if (it == watchpoints_.end()) {
        return false;  // 존재하지 않음
    }

    it->second.enabled = enabled;
    return true;
}

bool WatchpointManager::isEnabled(const std::string& variableName) const
{
    auto it = watchpoints_.find(variableName);
    if (it == watchpoints_.end()) {
        throw std::out_of_range("Watchpoint not found: " + variableName);
    }

    return it->second.enabled;
}

int WatchpointManager::getChangeCount(const std::string& variableName) const
{
    auto it = watchpoints_.find(variableName);
    if (it == watchpoints_.end()) {
        throw std::out_of_range("Watchpoint not found: " + variableName);
    }

    return it->second.changeCount;
}

// ============================================================================
// Private Methods
// ============================================================================

void WatchpointManager::validateVariableName(const std::string& variableName) const
{
    if (variableName.empty()) {
        throw std::invalid_argument(
            "WatchpointManager::add() - variableName is empty"
        );
    }
}

} // namespace debugger
} // namespace kingsejong
