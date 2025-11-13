/**
 * @file Debugger.cpp
 * @brief KingSejong 언어 대화형 디버거 구현
 * @author KingSejong Team
 * @date 2025-11-14
 */

#include "debugger/Debugger.h"

namespace kingsejong {
namespace debugger {

// ============================================================================
// Public Methods
// ============================================================================

Debugger::Debugger()
    : breakpoints_(std::make_unique<BreakpointManager>())
    , callStack_(std::make_unique<CallStack>())
    , watchpoints_(std::make_unique<WatchpointManager>())
    , state_(DebuggerState::IDLE)
    , stepOverDepth_(0)
{
}

void Debugger::step()
{
    state_ = DebuggerState::STEPPING;
}

void Debugger::next()
{
    state_ = DebuggerState::STEPPING_OVER;
    stepOverDepth_ = callStack_->depth();
}

void Debugger::continueExecution()
{
    state_ = DebuggerState::RUNNING;
}

void Debugger::pause()
{
    state_ = DebuggerState::PAUSED;
}

bool Debugger::shouldPause(const error::SourceLocation& location, const evaluator::Environment& env)
{
    // 1. 브레이크포인트 체크
    if (breakpoints_->shouldBreak(location, env)) {
        state_ = DebuggerState::PAUSED;
        return true;
    }

    // 2. STEPPING 상태 - 항상 멈춤
    if (state_ == DebuggerState::STEPPING) {
        state_ = DebuggerState::PAUSED;
        return true;
    }

    // 3. STEPPING_OVER 상태 - 스택 깊이 체크
    if (state_ == DebuggerState::STEPPING_OVER) {
        // 현재 스택 깊이가 step over 시작 깊이 이하면 멈춤
        // (함수에서 돌아왔거나 같은 레벨)
        if (callStack_->depth() <= stepOverDepth_) {
            state_ = DebuggerState::PAUSED;
            return true;
        }
    }

    // 4. 와치포인트 체크
    auto changedVars = watchpoints_->checkChanges(env);
    if (!changedVars.empty()) {
        state_ = DebuggerState::PAUSED;
        return true;
    }

    return false;
}

} // namespace debugger
} // namespace kingsejong
