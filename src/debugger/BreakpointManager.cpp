/**
 * @file BreakpointManager.cpp
 * @brief 브레이크포인트 관리 시스템 구현
 * @author KingSejong Team
 * @date 2025-11-11
 */

#include "debugger/BreakpointManager.h"
#include <stdexcept>

namespace kingsejong {
namespace debugger {

// ============================================================================
// Public Methods
// ============================================================================

bool BreakpointManager::add(
    const error::SourceLocation& location,
    const std::optional<std::string>& condition)
{
    // 위치 유효성 검증
    validateLocation(location);

    // 이미 존재하는지 확인
    if (breakpoints_.find(location) != breakpoints_.end()) {
        return false;  // 중복
    }

    // 브레이크포인트 추가
    if (condition.has_value()) {
        breakpoints_.emplace(location, Breakpoint(location, condition.value()));
    } else {
        breakpoints_.emplace(location, Breakpoint(location));
    }

    return true;
}

bool BreakpointManager::remove(const error::SourceLocation& location)
{
    auto it = breakpoints_.find(location);
    if (it == breakpoints_.end()) {
        return false;  // 존재하지 않음
    }

    breakpoints_.erase(it);
    return true;
}

bool BreakpointManager::shouldBreak(
    const error::SourceLocation& location,
    const evaluator::Environment& env)
{
    // 브레이크포인트 존재 확인
    auto it = breakpoints_.find(location);
    if (it == breakpoints_.end()) {
        return false;  // 브레이크포인트 없음
    }

    Breakpoint& bp = it->second;

    // 비활성화된 브레이크포인트는 무시
    if (!bp.enabled) {
        return false;
    }

    // 조건식 평가
    if (bp.condition.has_value()) {
        bool conditionMet = evaluateCondition(bp.condition.value(), env);
        if (!conditionMet) {
            return false;  // 조건 불만족
        }
    }

    // 도달 횟수 증가
    bp.hitCount++;

    return true;  // 중단해야 함
}

bool BreakpointManager::setEnabled(const error::SourceLocation& location, bool enabled)
{
    auto it = breakpoints_.find(location);
    if (it == breakpoints_.end()) {
        return false;  // 존재하지 않음
    }

    it->second.enabled = enabled;
    return true;
}

bool BreakpointManager::isEnabled(const error::SourceLocation& location) const
{
    auto it = breakpoints_.find(location);
    if (it == breakpoints_.end()) {
        throw std::out_of_range("Breakpoint not found at: " + location.toString());
    }

    return it->second.enabled;
}

int BreakpointManager::getHitCount(const error::SourceLocation& location) const
{
    auto it = breakpoints_.find(location);
    if (it == breakpoints_.end()) {
        throw std::out_of_range("Breakpoint not found at: " + location.toString());
    }

    return it->second.hitCount;
}

// ============================================================================
// Private Methods
// ============================================================================

void BreakpointManager::validateLocation(const error::SourceLocation& location) const
{
    if (location.filename.empty()) {
        throw std::invalid_argument("BreakpointManager::add() - filename is empty");
    }

    if (location.line < 1) {
        throw std::invalid_argument(
            "BreakpointManager::add() - invalid line number: " +
            std::to_string(location.line)
        );
    }
}

bool BreakpointManager::evaluateCondition(
    const std::string& condition,
    const evaluator::Environment& env) const
{
    // TODO: 조건식 평가 구현
    // 현재는 단순화를 위해 항상 true 반환
    // Week 5-6에 고급 기능으로 구현 예정
    //
    // 구현 계획:
    // 1. Lexer로 토큰화
    // 2. Parser로 표현식 파싱
    // 3. Evaluator로 평가
    // 4. isTruthy() 판별
    (void)condition;  // 미사용 경고 제거
    (void)env;        // 미사용 경고 제거
    return true;
}

} // namespace debugger
} // namespace kingsejong
