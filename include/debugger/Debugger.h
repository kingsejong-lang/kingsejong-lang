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
#include "debugger/WatchpointManager.h"
#include "debugger/CommandParser.h"
#include "debugger/SourceCodeViewer.h"
#include "error/Error.h"
#include "evaluator/Environment.h"
#include "evaluator/Evaluator.h"
#include "ast/Node.h"
#include <memory>
#include <string>
#include <iostream>

namespace kingsejong {
namespace debugger {

/**
 * @enum DebuggerState
 * @brief 디버거 실행 상태
 */
enum class DebuggerState {
    IDLE,           ///< 대기 상태 (프로그램 실행 전)
    RUNNING,        ///< 실행 중 (다음 브레이크포인트까지)
    STEPPING,       ///< 단계 실행 중 (step into)
    STEPPING_OVER,  ///< 단계 건너뛰기 중 (step over)
    PAUSED          ///< 일시 정지 (브레이크포인트 또는 사용자 요청)
};

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

    /**
     * @brief 와치포인트 관리자 접근
     * @return WatchpointManager 참조
     *
     * Complexity: O(1)
     */
    WatchpointManager& getWatchpoints() {
        return *watchpoints_;
    }

    /**
     * @brief 와치포인트 관리자 접근 (const)
     * @return WatchpointManager const 참조
     *
     * Complexity: O(1)
     */
    const WatchpointManager& getWatchpoints() const {
        return *watchpoints_;
    }

    /**
     * @brief 현재 디버거 상태 조회
     * @return 현재 DebuggerState
     *
     * Complexity: O(1)
     */
    DebuggerState getState() const {
        return state_;
    }

    /**
     * @brief 단계 실행 (step into)
     *
     * 한 줄 실행 후 멈춥니다. 함수 호출이 있으면 함수 안으로 들어갑니다.
     * 상태를 STEPPING으로 변경합니다.
     *
     * Complexity: O(1)
     */
    void step();

    /**
     * @brief 다음 줄 실행 (step over)
     *
     * 한 줄 실행 후 멈춥니다. 함수 호출이 있어도 건너뜁니다.
     * 상태를 STEPPING_OVER로 변경하고 현재 스택 깊이를 기록합니다.
     *
     * Complexity: O(1)
     */
    void next();

    /**
     * @brief 계속 실행 (continue)
     *
     * 다음 브레이크포인트까지 실행을 계속합니다.
     * 상태를 RUNNING으로 변경합니다.
     *
     * Complexity: O(1)
     */
    void continueExecution();

    /**
     * @brief 실행 일시 정지
     *
     * 실행을 일시 정지하고 상태를 PAUSED로 변경합니다.
     *
     * Complexity: O(1)
     */
    void pause();

    /**
     * @brief 특정 위치에서 멈춰야 하는지 판단
     * @param location 현재 실행 위치
     * @param env 현재 환경
     * @return true if 멈춰야 함, false otherwise
     *
     * 다음 조건에서 true를 반환합니다:
     * - 브레이크포인트가 설정되어 있는 경우
     * - STEPPING 상태인 경우
     * - STEPPING_OVER 상태이고 스택 깊이가 stepOverDepth_ 이하인 경우
     * - 와치포인트가 트리거된 경우
     *
     * Complexity: O(B + W) where B = 브레이크포인트 수, W = 와치포인트 수
     */
    bool shouldPause(const error::SourceLocation& location, const evaluator::Environment& env);

    /**
     * @brief 소스 코드 뷰어 접근
     * @return SourceCodeViewer 참조
     *
     * Complexity: O(1)
     */
    SourceCodeViewer& getSourceViewer() {
        return *sourceViewer_;
    }

    /**
     * @brief 소스 코드 뷰어 접근 (const)
     * @return SourceCodeViewer const 참조
     *
     * Complexity: O(1)
     */
    const SourceCodeViewer& getSourceViewer() const {
        return *sourceViewer_;
    }

    /**
     * @brief 디버거 REPL 시작
     * @param program 실행할 AST 프로그램
     * @param env 초기 환경
     * @param input 입력 스트림 (기본값: std::cin)
     * @param output 출력 스트림 (기본값: std::cout)
     *
     * 대화형 디버깅 세션을 시작합니다.
     * 사용자 명령어를 받아서 실행하고, 결과를 출력합니다.
     *
     * 지원 명령어:
     * - break <file>:<line> [condition] : 브레이크포인트 설정
     * - delete <id> : 브레이크포인트 삭제
     * - step/s : 단계 실행
     * - next/n : 다음 줄
     * - continue/c : 계속 실행
     * - print/p <expr> : 표현식 출력
     * - backtrace/bt : 호출 스택 출력
     * - list/l [line] : 소스 코드 표시
     * - watch/w <var> : 와치포인트 설정
     * - unwatch/uw <var> : 와치포인트 삭제
     * - help/h : 도움말
     * - quit/q : 종료
     */
    void repl(
        ast::Program* program,
        std::shared_ptr<evaluator::Environment> env,
        std::istream& input = std::cin,
        std::ostream& output = std::cout
    );

private:
    /**
     * @brief 명령어 실행 핸들러
     * @param cmd 파싱된 명령어
     * @param env 현재 환경
     * @param output 출력 스트림
     * @return true if 계속 실행, false if 종료
     */
    bool handleCommand(
        const Command& cmd,
        std::shared_ptr<evaluator::Environment> env,
        std::ostream& output
    );

    /**
     * @brief 도움말 출력
     * @param output 출력 스트림
     */
    void printHelp(std::ostream& output) const;

    /**
     * @brief 현재 상태 출력
     * @param output 출력 스트림
     */
    void printStatus(std::ostream& output) const;

    /// 브레이크포인트 관리자 (RAII)
    std::unique_ptr<BreakpointManager> breakpoints_;

    /// 콜 스택 (RAII)
    std::unique_ptr<CallStack> callStack_;

    /// 와치포인트 관리자 (RAII)
    std::unique_ptr<WatchpointManager> watchpoints_;

    /// 명령어 파서 (RAII)
    std::unique_ptr<CommandParser> parser_;

    /// 소스 코드 뷰어 (RAII)
    std::unique_ptr<SourceCodeViewer> sourceViewer_;

    /// 현재 디버거 상태
    DebuggerState state_;

    /// step over 시작 시점의 스택 깊이
    size_t stepOverDepth_;
};

} // namespace debugger
} // namespace kingsejong
