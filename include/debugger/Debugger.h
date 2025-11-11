#pragma once

/**
 * @file Debugger.h
 * @brief KingSejong 언어 대화형 디버거
 * @author KingSejong Team
 * @date 2025-11-11
 *
 * GDB 스타일의 디버깅 인터페이스를 제공합니다.
 * Week 3-4: 기본 구조 (BreakpointManager, CallStack 통합)
 * Week 5-6: 고급 기능 (WatchpointManager, CommandParser, run())
 */

#include "debugger/BreakpointManager.h"
#include "debugger/CallStack.h"
#include <memory>

namespace kingsejong {
namespace debugger {

/**
 * @class Debugger
 * @brief KingSejong 언어 대화형 디버거
 *
 * GDB 스타일의 디버깅 인터페이스를 제공합니다.
 * 브레이크포인트, 단계별 실행, 변수 검사 등을 지원합니다.
 *
 * Thread Safety: NOT thread-safe. 외부 동기화 필요.
 * Memory: RAII - 모든 리소스 자동 관리
 *
 * @invariant breakpoints_ != nullptr
 * @invariant callStack_ != nullptr
 *
 * Example (Week 3-4 범위):
 * ```cpp
 * Debugger debugger;
 *
 * // 브레이크포인트 설정
 * debugger.getBreakpoints().add(SourceLocation("test.ksj", 10, 1));
 *
 * // 콜 스택 관리
 * auto env = std::make_shared<Environment>();
 * debugger.getCallStack().push(CallStack::StackFrame("main", loc, env));
 *
 * // 브레이크포인트 체크
 * if (debugger.getBreakpoints().shouldBreak(loc, *env)) {
 *     // 디버깅 중단
 * }
 * ```
 *
 * TODO (Week 5-6):
 * - WatchpointManager 추가
 * - CommandParser 추가
 * - run() 메서드 (Evaluator 통합)
 * - step/next/continue 명령
 */
class Debugger {
public:
    // 복사 금지, 이동 허용
    Debugger();
    Debugger(const Debugger&) = delete;
    Debugger& operator=(const Debugger&) = delete;
    Debugger(Debugger&&) noexcept = default;
    Debugger& operator=(Debugger&&) noexcept = default;
    ~Debugger() = default;

    /**
     * @brief 브레이크포인트 관리자 접근
     * @return BreakpointManager 참조
     *
     * Complexity: O(1)
     */
    BreakpointManager& getBreakpoints() {
        return *breakpoints_;
    }

    /**
     * @brief 브레이크포인트 관리자 접근 (const)
     * @return BreakpointManager const 참조
     *
     * Complexity: O(1)
     */
    const BreakpointManager& getBreakpoints() const {
        return *breakpoints_;
    }

    /**
     * @brief 콜 스택 접근
     * @return CallStack 참조
     *
     * Complexity: O(1)
     */
    CallStack& getCallStack() {
        return *callStack_;
    }

    /**
     * @brief 콜 스택 접근 (const)
     * @return CallStack const 참조
     *
     * Complexity: O(1)
     */
    const CallStack& getCallStack() const {
        return *callStack_;
    }

private:
    /// 브레이크포인트 관리자 (RAII)
    std::unique_ptr<BreakpointManager> breakpoints_;

    /// 콜 스택 (RAII)
    std::unique_ptr<CallStack> callStack_;

    // TODO (Week 5-6): WatchpointManager 추가
    // std::unique_ptr<WatchpointManager> watchpoints_;

    // TODO (Week 5-6): Evaluator 추가 (run 메서드용)
    // std::shared_ptr<evaluator::Evaluator> evaluator_;
};

} // namespace debugger
} // namespace kingsejong
