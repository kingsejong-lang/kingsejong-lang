/**
 * @file DebuggerTest.cpp
 * @brief Debugger 테스트
 * @author KingSejong Team
 * @date 2025-11-11
 *
 * TDD 방식: 테스트를 먼저 작성하고 구현합니다.
 * Week 3-4: 기본 디버거 구조만 구현 (명령어 인터페이스는 Week 5-6)
 */

#include <gtest/gtest.h>
#include "debugger/Debugger.h"
#include "error/Error.h"
#include "evaluator/Environment.h"
#include "evaluator/Value.h"

using namespace kingsejong::debugger;
using namespace kingsejong::error;
using namespace kingsejong::evaluator;

/**
 * @class DebuggerTest
 * @brief Debugger 테스트 픽스처
 */
class DebuggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        debugger = std::make_unique<Debugger>();
    }

    std::unique_ptr<Debugger> debugger;
};

// ============================================================================
// 기본 생성 및 초기화 테스트
// ============================================================================

TEST_F(DebuggerTest, ShouldCreateDebugger) {
    // Assert
    EXPECT_NE(debugger, nullptr);
}

TEST_F(DebuggerTest, ShouldProvideBreakpointManager) {
    // Act
    BreakpointManager& bpMgr = debugger->getBreakpoints();

    // Assert - 초기에는 브레이크포인트 없음
    EXPECT_EQ(bpMgr.getAll().size(), 0);
}

TEST_F(DebuggerTest, ShouldProvideCallStack) {
    // Act
    CallStack& stack = debugger->getCallStack();

    // Assert - 초기에는 스택 비어있음
    EXPECT_TRUE(stack.empty());
}

// ============================================================================
// 브레이크포인트 통합 테스트
// ============================================================================

TEST_F(DebuggerTest, ShouldAddBreakpointThroughDebugger) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);

    // Act
    BreakpointManager& bpMgr = debugger->getBreakpoints();
    bool result = bpMgr.add(loc);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(bpMgr.getAll().size(), 1);
}

TEST_F(DebuggerTest, ShouldManageMultipleBreakpoints) {
    // Arrange
    SourceLocation loc1("test.ksj", 10, 1);
    SourceLocation loc2("test.ksj", 20, 1);
    SourceLocation loc3("other.ksj", 15, 1);

    // Act
    BreakpointManager& bpMgr = debugger->getBreakpoints();
    bpMgr.add(loc1);
    bpMgr.add(loc2);
    bpMgr.add(loc3);

    // Assert
    EXPECT_EQ(bpMgr.getAll().size(), 3);
}

TEST_F(DebuggerTest, ShouldRemoveBreakpointThroughDebugger) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    BreakpointManager& bpMgr = debugger->getBreakpoints();
    bpMgr.add(loc);

    // Act
    bool result = bpMgr.remove(loc);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(bpMgr.getAll().size(), 0);
}

// ============================================================================
// 콜 스택 통합 테스트
// ============================================================================

TEST_F(DebuggerTest, ShouldManageCallStackThroughDebugger) {
    // Arrange
    auto env = std::make_shared<Environment>();
    SourceLocation loc("test.ksj", 10, 1);

    // Act
    CallStack& stack = debugger->getCallStack();
    stack.push(CallStack::StackFrame("함수1", loc, env));

    // Assert
    EXPECT_EQ(stack.depth(), 1);
    EXPECT_EQ(stack.current().functionName, "함수1");
}

TEST_F(DebuggerTest, ShouldTrackCallHierarchy) {
    // Arrange
    auto env = std::make_shared<Environment>();
    CallStack& stack = debugger->getCallStack();

    // Act - main -> foo -> bar 호출 체인
    stack.push(CallStack::StackFrame("main", SourceLocation("main.ksj", 5, 1), env));
    stack.push(CallStack::StackFrame("foo", SourceLocation("foo.ksj", 10, 1), env));
    stack.push(CallStack::StackFrame("bar", SourceLocation("bar.ksj", 15, 1), env));

    // Assert
    EXPECT_EQ(stack.depth(), 3);
    EXPECT_EQ(stack.current().functionName, "bar");

    const auto& backtrace = stack.getAll();
    EXPECT_EQ(backtrace[0].functionName, "main");
    EXPECT_EQ(backtrace[1].functionName, "foo");
    EXPECT_EQ(backtrace[2].functionName, "bar");
}

// ============================================================================
// 통합 시나리오 테스트
// ============================================================================

TEST_F(DebuggerTest, ShouldHandleBreakpointsAndCallStackTogether) {
    // Arrange
    SourceLocation bp1("test.ksj", 10, 1);
    SourceLocation bp2("test.ksj", 20, 1);
    auto env = std::make_shared<Environment>();

    // Act - 브레이크포인트 설정
    BreakpointManager& bpMgr = debugger->getBreakpoints();
    bpMgr.add(bp1);
    bpMgr.add(bp2);

    // Act - 함수 호출
    CallStack& stack = debugger->getCallStack();
    stack.push(CallStack::StackFrame("main", bp1, env));

    // Assert
    EXPECT_EQ(bpMgr.getAll().size(), 2);
    EXPECT_EQ(stack.depth(), 1);

    // Act - 브레이크포인트에서 중단 여부 확인
    EXPECT_TRUE(bpMgr.shouldBreak(bp1, *env));
    EXPECT_TRUE(bpMgr.shouldBreak(bp2, *env));
}

TEST_F(DebuggerTest, ShouldSimulateFunctionCallAndReturn) {
    // Arrange
    auto env1 = std::make_shared<Environment>();
    auto env2 = std::make_shared<Environment>();
    CallStack& stack = debugger->getCallStack();

    // Act - main 호출
    stack.push(CallStack::StackFrame("main", SourceLocation("test.ksj", 1, 1), env1));
    EXPECT_EQ(stack.depth(), 1);

    // Act - foo 호출
    stack.push(CallStack::StackFrame("foo", SourceLocation("test.ksj", 10, 1), env2));
    EXPECT_EQ(stack.depth(), 2);
    EXPECT_EQ(stack.current().functionName, "foo");

    // Act - foo 반환
    stack.pop();
    EXPECT_EQ(stack.depth(), 1);
    EXPECT_EQ(stack.current().functionName, "main");

    // Act - main 반환
    stack.pop();
    EXPECT_TRUE(stack.empty());
}

// ============================================================================
// RAII 및 메모리 안전성 테스트
// ============================================================================

TEST_F(DebuggerTest, ShouldNotLeakOnDestruction) {
    // Arrange
    auto env = std::make_shared<Environment>();

    // Act - 여러 리소스 생성
    {
        auto tempDebugger = std::make_unique<Debugger>();
        BreakpointManager& bpMgr = tempDebugger->getBreakpoints();
        CallStack& stack = tempDebugger->getCallStack();

        for (int i = 1; i <= 100; i++) {
            bpMgr.add(SourceLocation("test.ksj", i, 1));
            stack.push(CallStack::StackFrame("func" + std::to_string(i),
                                            SourceLocation("test.ksj", i, 1),
                                            env));
        }

        // tempDebugger가 스코프를 벗어나면서 자동 소멸
    }

    // Assert - AddressSanitizer로 메모리 누수 검증
    SUCCEED();
}

TEST_F(DebuggerTest, ShouldHandleMoveSemantics) {
    // Arrange
    auto env = std::make_shared<Environment>();
    debugger->getBreakpoints().add(SourceLocation("test.ksj", 10, 1));
    debugger->getCallStack().push(CallStack::StackFrame("main", SourceLocation("test.ksj", 1, 1), env));

    // Act - move assignment
    auto debugger2 = std::make_unique<Debugger>();
    *debugger2 = std::move(*debugger);

    // Assert
    EXPECT_EQ(debugger2->getBreakpoints().getAll().size(), 1);
    EXPECT_EQ(debugger2->getCallStack().depth(), 1);
}

// ============================================================================
// 상수 메서드 테스트
// ============================================================================

TEST_F(DebuggerTest, ShouldProvideConstAccess) {
    // Arrange
    auto env = std::make_shared<Environment>();
    debugger->getBreakpoints().add(SourceLocation("test.ksj", 10, 1));
    debugger->getCallStack().push(CallStack::StackFrame("main", SourceLocation("test.ksj", 1, 1), env));

    // Act - const 참조로 접근
    const Debugger& constDebugger = *debugger;

    // Assert
    EXPECT_EQ(constDebugger.getBreakpoints().getAll().size(), 1);
    EXPECT_EQ(constDebugger.getCallStack().depth(), 1);
}

// ============================================================================
// 경계 조건 테스트
// ============================================================================

TEST_F(DebuggerTest, ShouldHandleEmptyState) {
    // Assert - 초기 상태
    EXPECT_EQ(debugger->getBreakpoints().getAll().size(), 0);
    EXPECT_TRUE(debugger->getCallStack().empty());
}

TEST_F(DebuggerTest, ShouldHandleClearingAllState) {
    // Arrange
    auto env = std::make_shared<Environment>();
    BreakpointManager& bpMgr = debugger->getBreakpoints();
    CallStack& stack = debugger->getCallStack();

    bpMgr.add(SourceLocation("test.ksj", 10, 1));
    bpMgr.add(SourceLocation("test.ksj", 20, 1));
    stack.push(CallStack::StackFrame("main", SourceLocation("test.ksj", 1, 1), env));
    stack.push(CallStack::StackFrame("foo", SourceLocation("test.ksj", 10, 1), env));

    // Act - 모두 제거
    bpMgr.clear();
    stack.clear();

    // Assert
    EXPECT_EQ(bpMgr.getAll().size(), 0);
    EXPECT_TRUE(stack.empty());
}

// ============================================================================
// 와치포인트 통합 테스트
// ============================================================================

TEST_F(DebuggerTest, ShouldProvideWatchpointManager) {
    // Act
    WatchpointManager& wpMgr = debugger->getWatchpoints();

    // Assert - 초기에는 와치포인트 없음
    EXPECT_EQ(wpMgr.getAll().size(), 0u);
}

TEST_F(DebuggerTest, ShouldAddWatchpointThroughDebugger) {
    // Arrange
    auto env = std::make_shared<Environment>();
    env->set("x", Value::createInteger(10));

    // Act
    WatchpointManager& wpMgr = debugger->getWatchpoints();
    bool result = wpMgr.add("x", *env);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(wpMgr.getAll().size(), 1u);
}

// ============================================================================
// 디버거 상태 관리 테스트
// ============================================================================

TEST_F(DebuggerTest, ShouldStartInIdleState) {
    // Assert
    EXPECT_EQ(debugger->getState(), DebuggerState::IDLE);
}

TEST_F(DebuggerTest, ShouldEnterSteppingState) {
    // Act
    debugger->step();

    // Assert
    EXPECT_EQ(debugger->getState(), DebuggerState::STEPPING);
}

TEST_F(DebuggerTest, ShouldEnterSteppingOverState) {
    // Act
    debugger->next();

    // Assert
    EXPECT_EQ(debugger->getState(), DebuggerState::STEPPING_OVER);
}

TEST_F(DebuggerTest, ShouldEnterRunningState) {
    // Act
    debugger->continueExecution();

    // Assert
    EXPECT_EQ(debugger->getState(), DebuggerState::RUNNING);
}

TEST_F(DebuggerTest, ShouldEnterPausedState) {
    // Arrange
    debugger->continueExecution();  // 먼저 RUNNING으로

    // Act
    debugger->pause();

    // Assert
    EXPECT_EQ(debugger->getState(), DebuggerState::PAUSED);
}

// ============================================================================
// shouldPause() 테스트
// ============================================================================

TEST_F(DebuggerTest, ShouldPauseOnBreakpoint) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    auto env = std::make_shared<Environment>();

    debugger->getBreakpoints().add(loc);
    debugger->continueExecution();  // RUNNING 상태로

    // Act
    bool shouldPause = debugger->shouldPause(loc, *env);

    // Assert
    EXPECT_TRUE(shouldPause);
    EXPECT_EQ(debugger->getState(), DebuggerState::PAUSED);
}

TEST_F(DebuggerTest, ShouldPauseWhenStepping) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    auto env = std::make_shared<Environment>();

    debugger->step();  // STEPPING 상태로

    // Act
    bool shouldPause = debugger->shouldPause(loc, *env);

    // Assert
    EXPECT_TRUE(shouldPause);
    EXPECT_EQ(debugger->getState(), DebuggerState::PAUSED);
}

TEST_F(DebuggerTest, ShouldPauseWhenSteppingOver) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    auto env = std::make_shared<Environment>();

    // 스택 설정
    debugger->getCallStack().push(CallStack::StackFrame("main", loc, env));

    // next() 호출 (현재 깊이 = 1)
    debugger->next();

    // Act - 같은 깊이에서 다음 줄 실행
    bool shouldPause = debugger->shouldPause(SourceLocation("test.ksj", 11, 1), *env);

    // Assert
    EXPECT_TRUE(shouldPause);
    EXPECT_EQ(debugger->getState(), DebuggerState::PAUSED);
}

TEST_F(DebuggerTest, ShouldNotPauseWhenSteppingOverInDeepFunction) {
    // Arrange
    SourceLocation loc1("test.ksj", 10, 1);
    SourceLocation loc2("test.ksj", 20, 1);
    auto env = std::make_shared<Environment>();

    // 스택 설정 (깊이 = 1)
    debugger->getCallStack().push(CallStack::StackFrame("main", loc1, env));

    // next() 호출 (stepOverDepth_ = 1)
    debugger->next();

    // 함수 호출로 스택 깊이 증가 (깊이 = 2)
    debugger->getCallStack().push(CallStack::StackFrame("foo", loc2, env));

    // Act - 더 깊은 함수 안에서는 멈추지 않음
    bool shouldPause = debugger->shouldPause(loc2, *env);

    // Assert
    EXPECT_FALSE(shouldPause);
    EXPECT_EQ(debugger->getState(), DebuggerState::STEPPING_OVER);
}

TEST_F(DebuggerTest, ShouldPauseWhenSteppingOverAfterFunctionReturn) {
    // Arrange
    SourceLocation loc1("test.ksj", 10, 1);
    SourceLocation loc2("test.ksj", 20, 1);
    auto env = std::make_shared<Environment>();

    // 스택 설정 (깊이 = 1)
    debugger->getCallStack().push(CallStack::StackFrame("main", loc1, env));

    // next() 호출 (stepOverDepth_ = 1)
    debugger->next();

    // 함수 호출
    debugger->getCallStack().push(CallStack::StackFrame("foo", loc2, env));

    // 함수 반환
    debugger->getCallStack().pop();

    // Act - 함수에서 돌아온 후에는 멈춤
    bool shouldPause = debugger->shouldPause(SourceLocation("test.ksj", 11, 1), *env);

    // Assert
    EXPECT_TRUE(shouldPause);
    EXPECT_EQ(debugger->getState(), DebuggerState::PAUSED);
}

TEST_F(DebuggerTest, ShouldPauseOnWatchpointTrigger) {
    // Arrange
    auto env = std::make_shared<Environment>();
    env->set("x", Value::createInteger(10));

    debugger->getWatchpoints().add("x", *env);
    debugger->continueExecution();  // RUNNING 상태로

    // Act - 변수 값 변경
    env->set("x", Value::createInteger(20));
    bool shouldPause = debugger->shouldPause(SourceLocation("test.ksj", 10, 1), *env);

    // Assert
    EXPECT_TRUE(shouldPause);
    EXPECT_EQ(debugger->getState(), DebuggerState::PAUSED);
}

TEST_F(DebuggerTest, ShouldNotPauseWhenRunning) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    auto env = std::make_shared<Environment>();

    debugger->continueExecution();  // RUNNING 상태로

    // Act - 브레이크포인트 없음
    bool shouldPause = debugger->shouldPause(loc, *env);

    // Assert
    EXPECT_FALSE(shouldPause);
}

TEST_F(DebuggerTest, ShouldNotPauseWhenIdle) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    auto env = std::make_shared<Environment>();

    // Act - IDLE 상태 (초기 상태)
    bool shouldPause = debugger->shouldPause(loc, *env);

    // Assert
    EXPECT_FALSE(shouldPause);
}
