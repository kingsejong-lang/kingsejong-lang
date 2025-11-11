#pragma once

/**
 * @file CallStack.h
 * @brief 함수 호출 스택 추적 시스템
 * @author KingSejong Team
 * @date 2025-11-11
 *
 * 함수 호출 체인을 추적하여 backtrace를 제공합니다.
 */

#include "error/Error.h"
#include "evaluator/Environment.h"
#include <vector>
#include <memory>
#include <string>

namespace kingsejong {
namespace debugger {

/**
 * @class CallStack
 * @brief 함수 호출 스택 추적
 *
 * 함수 호출 체인을 추적하여 backtrace를 제공합니다.
 *
 * Thread Safety: NOT thread-safe. 외부 동기화 필요.
 * Memory: RAII - vector로 자동 관리
 *
 * @invariant frames_는 항상 유효한 스택 프레임만 포함
 * @invariant top 프레임이 현재 실행 중인 함수
 *
 * Example:
 * ```cpp
 * CallStack stack;
 * auto env = std::make_shared<Environment>();
 *
 * // 함수 진입
 * stack.push(CallStack::StackFrame("main", loc, env));
 *
 * // 함수 호출
 * stack.push(CallStack::StackFrame("foo", loc2, childEnv));
 *
 * // 현재 함수 확인
 * std::cout << stack.current().functionName << std::endl;
 *
 * // 함수 종료
 * stack.pop();
 *
 * // Backtrace
 * for (const auto& frame : stack.getAll()) {
 *     std::cout << frame.functionName << " at " << frame.location.toString() << std::endl;
 * }
 * ```
 */
class CallStack {
public:
    /**
     * @struct StackFrame
     * @brief 스택 프레임 정보
     */
    struct StackFrame {
        std::string functionName;                    ///< 함수 이름
        error::SourceLocation location;              ///< 호출 위치
        std::shared_ptr<evaluator::Environment> env; ///< 환경 (변수 검사용)

        /**
         * @brief 스택 프레임 생성자
         * @param name 함수 이름
         * @param loc 호출 위치
         * @param environment 환경
         */
        StackFrame(const std::string& name,
                   const error::SourceLocation& loc,
                   std::shared_ptr<evaluator::Environment> environment)
            : functionName(name)
            , location(loc)
            , env(std::move(environment))
        {}
    };

    // 복사 금지, 이동 허용
    CallStack() = default;
    CallStack(const CallStack&) = delete;
    CallStack& operator=(const CallStack&) = delete;
    CallStack(CallStack&&) noexcept = default;
    CallStack& operator=(CallStack&&) noexcept = default;
    ~CallStack() = default;

    /**
     * @brief 스택 프레임 추가
     * @param frame 스택 프레임
     *
     * Postconditions:
     * - depth() 증가
     * - current()는 추가된 프레임 반환
     *
     * Complexity: O(1) amortized
     */
    void push(StackFrame frame) {
        frames_.push_back(std::move(frame));
    }

    /**
     * @brief 스택 프레임 제거
     * @throws std::runtime_error if stack is empty
     *
     * Preconditions:
     * - !empty()
     *
     * Postconditions:
     * - depth() 감소
     *
     * Complexity: O(1)
     */
    void pop();

    /**
     * @brief 현재 스택 프레임 조회
     * @return 현재 프레임 참조
     * @throws std::runtime_error if stack is empty
     *
     * Preconditions:
     * - !empty()
     *
     * Complexity: O(1)
     */
    StackFrame& current();

    /**
     * @brief 현재 스택 프레임 조회 (const)
     * @return 현재 프레임 const 참조
     * @throws std::runtime_error if stack is empty
     *
     * Preconditions:
     * - !empty()
     *
     * Complexity: O(1)
     */
    const StackFrame& current() const;

    /**
     * @brief 모든 스택 프레임 조회 (backtrace)
     * @return 스택 프레임 목록 (const 참조)
     *
     * frames_[0]가 가장 먼저 호출된 함수 (main)
     * frames_[n-1]이 현재 실행 중인 함수
     *
     * Complexity: O(1)
     */
    const std::vector<StackFrame>& getAll() const {
        return frames_;
    }

    /**
     * @brief 스택 깊이
     * @return 스택 프레임 개수
     *
     * Complexity: O(1)
     */
    size_t depth() const {
        return frames_.size();
    }

    /**
     * @brief 스택이 비어있는지 확인
     * @return true if empty
     *
     * Complexity: O(1)
     */
    bool empty() const {
        return frames_.empty();
    }

    /**
     * @brief 모든 스택 프레임 제거
     *
     * Postconditions:
     * - empty() == true
     * - depth() == 0
     *
     * Complexity: O(n)
     */
    void clear() {
        frames_.clear();
    }

private:
    /// 스택 프레임 목록 (RAII)
    std::vector<StackFrame> frames_;
};

} // namespace debugger
} // namespace kingsejong
