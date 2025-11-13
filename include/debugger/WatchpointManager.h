#pragma once

/**
 * @file WatchpointManager.h
 * @brief 와치포인트 관리 시스템
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * 변수 값 변경을 감지하여 자동으로 중단합니다.
 */

#include "evaluator/Environment.h"
#include "evaluator/Value.h"
#include <map>
#include <string>
#include <vector>
#include <memory>

namespace kingsejong {
namespace debugger {

/**
 * @class WatchpointManager
 * @brief 와치포인트 관리자
 *
 * 변수 값 변경을 감지하여 자동으로 중단합니다.
 *
 * Thread Safety: NOT thread-safe. 외부 동기화 필요.
 * Memory: RAII - map으로 자동 관리
 *
 * @invariant 모든 Watchpoint는 유효한 변수명을 가짐
 * @invariant lastValue는 항상 최신 값을 유지
 *
 * Example:
 * ```cpp
 * WatchpointManager mgr;
 * auto env = std::make_shared<Environment>();
 * env->set("x", Value::createInteger(10));
 *
 * // 와치포인트 설정
 * mgr.add("x", *env);
 *
 * // 변수 값 변경
 * env->set("x", Value::createInteger(20));
 *
 * // 변경 감지
 * auto changed = mgr.checkChanges(*env);
 * // changed == {"x"}
 * ```
 */
class WatchpointManager {
public:
    /**
     * @struct Watchpoint
     * @brief 와치포인트 정보
     */
    struct Watchpoint {
        std::string variableName;            ///< 변수 이름
        evaluator::Value lastValue;          ///< 마지막으로 기록된 값
        bool enabled;                        ///< 활성화 여부
        int changeCount;                     ///< 변경 횟수

        /**
         * @brief 와치포인트 생성자
         * @param name 변수 이름
         * @param value 현재 값
         */
        Watchpoint(const std::string& name, const evaluator::Value& value)
            : variableName(name)
            , lastValue(value)
            , enabled(true)
            , changeCount(0)
        {}
    };

    // 복사 금지, 이동 허용
    WatchpointManager() = default;
    WatchpointManager(const WatchpointManager&) = delete;
    WatchpointManager& operator=(const WatchpointManager&) = delete;
    WatchpointManager(WatchpointManager&&) noexcept = default;
    WatchpointManager& operator=(WatchpointManager&&) noexcept = default;
    ~WatchpointManager() = default;

    /**
     * @brief 와치포인트 추가
     * @param variableName 변수 이름
     * @param env 현재 환경
     * @return 추가 성공 여부 (false: 이미 존재 또는 변수 없음)
     * @throws std::invalid_argument if variableName is empty
     *
     * Preconditions:
     * - variableName이 비어있지 않음
     * - env에 변수가 존재함
     *
     * Postconditions:
     * - 성공 시 getAll().size() 증가
     * - 실패 시 상태 변화 없음
     */
    bool add(const std::string& variableName, const evaluator::Environment& env);

    /**
     * @brief 와치포인트 제거
     * @param variableName 변수 이름
     * @return 제거 성공 여부 (false: 존재하지 않음)
     *
     * Postconditions:
     * - 성공 시 getAll().size() 감소
     * - 실패 시 상태 변화 없음
     */
    bool remove(const std::string& variableName);

    /**
     * @brief 변수 값 변경 확인
     * @param env 현재 환경
     * @return 변경된 변수 목록
     *
     * 각 활성화된 와치포인트의 변수 값을 확인하여
     * lastValue와 다른 경우 변경된 것으로 간주합니다.
     *
     * Side Effects:
     * - changeCount 증가 (변경 감지 시)
     * - lastValue 업데이트 (변경 감지 시)
     *
     * Exception Safety: Strong guarantee
     * - 변수가 환경에 없는 경우 해당 와치포인트는 무시
     */
    std::vector<std::string> checkChanges(const evaluator::Environment& env);

    /**
     * @brief 모든 와치포인트 조회
     * @return 와치포인트 목록 (const 참조)
     *
     * Complexity: O(1)
     */
    const std::map<std::string, Watchpoint>& getAll() const {
        return watchpoints_;
    }

    /**
     * @brief 와치포인트 활성화/비활성화
     * @param variableName 변수 이름
     * @param enabled 활성화 여부
     * @return 성공 여부 (false: 존재하지 않음)
     */
    bool setEnabled(const std::string& variableName, bool enabled);

    /**
     * @brief 와치포인트 활성화 상태 확인
     * @param variableName 변수 이름
     * @return 활성화 여부
     * @throws std::out_of_range if watchpoint not found
     */
    bool isEnabled(const std::string& variableName) const;

    /**
     * @brief 와치포인트 변경 횟수 조회
     * @param variableName 변수 이름
     * @return 변경 횟수
     * @throws std::out_of_range if watchpoint not found
     */
    int getChangeCount(const std::string& variableName) const;

    /**
     * @brief 모든 와치포인트 제거
     *
     * Postconditions:
     * - getAll().empty() == true
     */
    void clear() {
        watchpoints_.clear();
    }

private:
    /// 변수명별 와치포인트 맵 (자동 정렬)
    std::map<std::string, Watchpoint> watchpoints_;

    /**
     * @brief 변수명 유효성 검증
     * @param variableName 변수 이름
     * @throws std::invalid_argument if variableName is empty
     */
    void validateVariableName(const std::string& variableName) const;
};

} // namespace debugger
} // namespace kingsejong
