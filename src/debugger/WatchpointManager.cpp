/**
 * @file WatchpointManager.cpp
 * @brief 와치포인트 관리 시스템 구현
 * @author KingSejong Team
 * @date 2025-11-14
 */

#include "debugger/WatchpointManager.h"
#include <stdexcept>
#include <unordered_map>

namespace kingsejong {
namespace debugger {

namespace {

using Snapshots = std::unordered_map<const void*, evaluator::Value>;

/**
 * @brief 배열과 딕셔너리를 깊게 복사한다.
 *
 * 값 비교는 배열·딕셔너리를 내용으로 하므로, 같은 객체를 그 자리에서 고친 변화도 잡으려면
 * 기록 시점의 내용을 따로 떠 두어야 한다. 순환 참조는 이미 복사한 객체를 재사용해 끊는다.
 */
evaluator::Value snapshot(const evaluator::Value& value, Snapshots& done)
{
    if (value.isArray()) {
        const auto& src = value.asArray();
        if (auto it = done.find(&src); it != done.end()) {
            return it->second;
        }
        evaluator::Value copy = evaluator::Value::createArray({});
        done.emplace(&src, copy);
        auto& dst = copy.asArray();
        dst.reserve(src.size());
        for (const auto& item : src) {
            dst.push_back(snapshot(item, done));
        }
        return copy;
    }
    if (value.isDictionary()) {
        const auto& src = value.asDictionary();
        if (auto it = done.find(&src); it != done.end()) {
            return it->second;
        }
        evaluator::Value copy = evaluator::Value::createDictionary({});
        done.emplace(&src, copy);
        auto& dst = copy.asDictionary();
        for (const auto& [key, item] : src) {
            dst.emplace(key, snapshot(item, done));
        }
        return copy;
    }
    return value;
}

evaluator::Value snapshot(const evaluator::Value& value)
{
    Snapshots done;
    return snapshot(value, done);
}

} // namespace

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
        watchpoints_.emplace(variableName, Watchpoint(variableName, snapshot(currentValue)));
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
                updates.emplace_back(varName, snapshot(currentValue));
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
