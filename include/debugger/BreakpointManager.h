#pragma once

/**
 * @file BreakpointManager.h
 * @brief 브레이크포인트 관리 시스템
 * @author KingSejong Team
 * @date 2025-11-11
 *
 * 파일:줄 위치 기반 브레이크포인트를 관리합니다.
 * 조건부 브레이크포인트를 지원합니다.
 */

#include "error/Error.h"
#include "evaluator/Environment.h"
#include <map>
#include <optional>
#include <string>
#include <memory>

namespace kingsejong {
namespace debugger {

/**
 * @class BreakpointManager
 * @brief 브레이크포인트 관리자
 *
 * 파일:줄 위치 기반 브레이크포인트를 관리합니다.
 * 조건부 브레이크포인트를 지원합니다.
 *
 * Thread Safety: NOT thread-safe. 외부 동기화 필요.
 * Memory: RAII - map으로 자동 관리
 *
 * @invariant 모든 Location은 유효한 파일명과 줄 번호를 가짐
 * @invariant 조건식이 있는 경우 유효한 표현식이어야 함
 *
 * Example:
 * ```cpp
 * BreakpointManager mgr;
 * SourceLocation loc("test.ksj", 10, 1);
 *
 * // 무조건 브레이크포인트
 * mgr.add(loc);
 *
 * // 조건부 브레이크포인트
 * mgr.add(loc, "x > 10");
 *
 * // 브레이크 여부 확인
 * if (mgr.shouldBreak(loc, env)) {
 *     // 중단
 * }
 * ```
 */
class BreakpointManager {
public:
    /**
     * @struct Breakpoint
     * @brief 브레이크포인트 정보
     */
    struct Breakpoint {
        error::SourceLocation location;            // 위치
        std::optional<std::string> condition;      // 조건식 (optional)
        bool enabled;                              // 활성화 여부
        int hitCount;                              // 도달 횟수

        /**
         * @brief 기본 생성자
         * @param loc 위치
         */
        explicit Breakpoint(const error::SourceLocation& loc)
            : location(loc)
            , condition(std::nullopt)
            , enabled(true)
            , hitCount(0)
        {}

        /**
         * @brief 조건부 브레이크포인트 생성자
         * @param loc 위치
         * @param cond 조건식
         */
        Breakpoint(const error::SourceLocation& loc, const std::string& cond)
            : location(loc)
            , condition(cond)
            , enabled(true)
            , hitCount(0)
        {}
    };

    // 복사 금지, 이동 허용
    BreakpointManager() = default;
    BreakpointManager(const BreakpointManager&) = delete;
    BreakpointManager& operator=(const BreakpointManager&) = delete;
    BreakpointManager(BreakpointManager&&) noexcept = default;
    BreakpointManager& operator=(BreakpointManager&&) noexcept = default;
    ~BreakpointManager() = default;

    /**
     * @brief 브레이크포인트 추가
     * @param location 위치 (파일:줄)
     * @param condition 조건식 (optional)
     * @return 추가 성공 여부 (false: 이미 존재)
     * @throws std::invalid_argument if location is invalid (빈 파일명, 음수 줄 번호)
     *
     * Preconditions:
     * - location.filename이 비어있지 않음
     * - location.line >= 1
     *
     * Postconditions:
     * - 성공 시 getAll().size() 증가
     * - 실패 시 상태 변화 없음
     */
    bool add(const error::SourceLocation& location,
             const std::optional<std::string>& condition = std::nullopt);

    /**
     * @brief 브레이크포인트 제거
     * @param location 위치
     * @return 제거 성공 여부 (false: 존재하지 않음)
     *
     * Postconditions:
     * - 성공 시 getAll().size() 감소
     * - 실패 시 상태 변화 없음
     */
    bool remove(const error::SourceLocation& location);

    /**
     * @brief 현재 위치에서 중단해야 하는지 확인
     * @param location 현재 위치
     * @param env 환경 (조건식 평가용)
     * @return 중단 여부
     *
     * 다음 조건을 모두 만족하면 true:
     * 1. location에 브레이크포인트가 설정됨
     * 2. 브레이크포인트가 활성화됨 (enabled == true)
     * 3. 조건식이 없거나, 조건식이 true로 평가됨
     *
     * Side Effects:
     * - hitCount 증가 (브레이크포인트가 활성화되고 조건이 만족되는 경우)
     *
     * Exception Safety: Strong guarantee
     * - 조건식 평가 실패 시 false 반환 (예외 전파 안 함)
     */
    bool shouldBreak(const error::SourceLocation& location,
                     const evaluator::Environment& env);

    /**
     * @brief 모든 브레이크포인트 조회
     * @return 브레이크포인트 목록 (const 참조)
     *
     * Complexity: O(1)
     */
    const std::map<error::SourceLocation, Breakpoint>& getAll() const {
        return breakpoints_;
    }

    /**
     * @brief 브레이크포인트 활성화/비활성화
     * @param location 위치
     * @param enabled 활성화 여부
     * @return 성공 여부 (false: 존재하지 않음)
     */
    bool setEnabled(const error::SourceLocation& location, bool enabled);

    /**
     * @brief 브레이크포인트 활성화 상태 확인
     * @param location 위치
     * @return 활성화 여부
     * @throws std::out_of_range if breakpoint not found
     */
    bool isEnabled(const error::SourceLocation& location) const;

    /**
     * @brief 브레이크포인트 도달 횟수 조회
     * @param location 위치
     * @return 도달 횟수
     * @throws std::out_of_range if breakpoint not found
     */
    int getHitCount(const error::SourceLocation& location) const;

    /**
     * @brief 모든 브레이크포인트 제거
     *
     * Postconditions:
     * - getAll().empty() == true
     */
    void clear() {
        breakpoints_.clear();
    }

private:
    /// 위치별 브레이크포인트 맵 (자동 정렬)
    std::map<error::SourceLocation, Breakpoint> breakpoints_;

    /**
     * @brief 위치 유효성 검증
     * @param location 위치
     * @throws std::invalid_argument if location is invalid
     */
    void validateLocation(const error::SourceLocation& location) const;

    /**
     * @brief 조건식 평가
     * @param condition 조건식
     * @param env 환경
     * @return 평가 결과 (true/false)
     *
     * 파싱 또는 평가 실패 시 false 반환 (예외 안 던짐)
     */
    bool evaluateCondition(const std::string& condition,
                           const evaluator::Environment& env) const;
};

} // namespace debugger
} // namespace kingsejong
