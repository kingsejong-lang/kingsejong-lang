/**
 * @file CallStackTest.cpp
 * @brief CallStack 테스트
 * @author KingSejong Team
 * @date 2025-11-11
 *
 * TDD 방식: 테스트를 먼저 작성하고 구현합니다.
 */

#include <gtest/gtest.h>
#include "debugger/CallStack.h"
#include "error/Error.h"
#include "evaluator/Environment.h"
#include "evaluator/Value.h"

using namespace kingsejong::debugger;
using namespace kingsejong::error;
using namespace kingsejong::evaluator;

/**
 * @class CallStackTest
 * @brief CallStack 테스트 픽스처
 */
class CallStackTest : public ::testing::Test {
protected:
    void SetUp() override {
        stack = std::make_unique<CallStack>();
    }

    std::unique_ptr<CallStack> stack;
};

// ============================================================================
// 기본 기능 테스트
// ============================================================================

TEST_F(CallStackTest, ShouldStartEmpty) {
    // Assert
    EXPECT_TRUE(stack->empty());
    EXPECT_EQ(stack->depth(), 0);
}

TEST_F(CallStackTest, ShouldPushFrame) {
    // Arrange
    auto env = std::make_shared<Environment>();
    SourceLocation loc("test.ksj", 10, 1);

    // Act
    stack->push(CallStack::StackFrame("함수1", loc, env));

    // Assert
    EXPECT_FALSE(stack->empty());
    EXPECT_EQ(stack->depth(), 1);
}

TEST_F(CallStackTest, ShouldPushMultipleFrames) {
    // Arrange
    auto env1 = std::make_shared<Environment>();
    auto env2 = std::make_shared<Environment>();
    auto env3 = std::make_shared<Environment>();

    // Act
    stack->push(CallStack::StackFrame("함수1", SourceLocation("test.ksj", 10, 1), env1));
    stack->push(CallStack::StackFrame("함수2", SourceLocation("test.ksj", 20, 1), env2));
    stack->push(CallStack::StackFrame("함수3", SourceLocation("test.ksj", 30, 1), env3));

    // Assert
    EXPECT_EQ(stack->depth(), 3);
}

TEST_F(CallStackTest, ShouldPopFrame) {
    // Arrange
    auto env = std::make_shared<Environment>();
    stack->push(CallStack::StackFrame("함수1", SourceLocation("test.ksj", 10, 1), env));
    stack->push(CallStack::StackFrame("함수2", SourceLocation("test.ksj", 20, 1), env));

    // Act
    stack->pop();

    // Assert
    EXPECT_EQ(stack->depth(), 1);
    EXPECT_EQ(stack->current().functionName, "함수1");
}

TEST_F(CallStackTest, ShouldThrowOnPopEmptyStack) {
    // Act & Assert
    EXPECT_THROW(stack->pop(), std::runtime_error);
}

TEST_F(CallStackTest, ShouldGetCurrentFrame) {
    // Arrange
    auto env = std::make_shared<Environment>();
    env->set("x", Value::createInteger(42));
    SourceLocation loc("test.ksj", 10, 1);
    stack->push(CallStack::StackFrame("함수1", loc, env));

    // Act
    CallStack::StackFrame& frame = stack->current();

    // Assert
    EXPECT_EQ(frame.functionName, "함수1");
    EXPECT_EQ(frame.location.filename, "test.ksj");
    EXPECT_EQ(frame.location.line, 10);
    EXPECT_EQ(frame.env->get("x").asInteger(), 42);
}

TEST_F(CallStackTest, ShouldThrowOnCurrentWhenEmpty) {
    // Act & Assert
    EXPECT_THROW(stack->current(), std::runtime_error);
}

TEST_F(CallStackTest, ShouldGetAllFrames) {
    // Arrange
    auto env = std::make_shared<Environment>();
    stack->push(CallStack::StackFrame("함수1", SourceLocation("test.ksj", 10, 1), env));
    stack->push(CallStack::StackFrame("함수2", SourceLocation("test.ksj", 20, 1), env));
    stack->push(CallStack::StackFrame("함수3", SourceLocation("test.ksj", 30, 1), env));

    // Act
    const auto& frames = stack->getAll();

    // Assert
    EXPECT_EQ(frames.size(), 3);
    EXPECT_EQ(frames[0].functionName, "함수1");
    EXPECT_EQ(frames[1].functionName, "함수2");
    EXPECT_EQ(frames[2].functionName, "함수3");
}

// ============================================================================
// 스택 프레임 순서 테스트
// ============================================================================

TEST_F(CallStackTest, ShouldMaintainLIFOOrder) {
    // Arrange
    auto env = std::make_shared<Environment>();

    // Act - LIFO 순서로 push
    stack->push(CallStack::StackFrame("main", SourceLocation("test.ksj", 1, 1), env));
    stack->push(CallStack::StackFrame("foo", SourceLocation("test.ksj", 10, 1), env));
    stack->push(CallStack::StackFrame("bar", SourceLocation("test.ksj", 20, 1), env));

    // Assert - 최상위는 bar
    EXPECT_EQ(stack->current().functionName, "bar");

    // Act - pop
    stack->pop();

    // Assert - 최상위는 foo
    EXPECT_EQ(stack->current().functionName, "foo");

    // Act - pop
    stack->pop();

    // Assert - 최상위는 main
    EXPECT_EQ(stack->current().functionName, "main");
}

TEST_F(CallStackTest, ShouldProvideBacktrace) {
    // Arrange - 호출 체인: main -> foo -> bar
    auto env = std::make_shared<Environment>();
    stack->push(CallStack::StackFrame("main", SourceLocation("main.ksj", 5, 1), env));
    stack->push(CallStack::StackFrame("foo", SourceLocation("foo.ksj", 10, 1), env));
    stack->push(CallStack::StackFrame("bar", SourceLocation("bar.ksj", 15, 1), env));

    // Act
    const auto& backtrace = stack->getAll();

    // Assert - 호출 순서대로 저장됨
    ASSERT_EQ(backtrace.size(), 3);
    EXPECT_EQ(backtrace[0].functionName, "main");
    EXPECT_EQ(backtrace[0].location.filename, "main.ksj");
    EXPECT_EQ(backtrace[1].functionName, "foo");
    EXPECT_EQ(backtrace[1].location.filename, "foo.ksj");
    EXPECT_EQ(backtrace[2].functionName, "bar");
    EXPECT_EQ(backtrace[2].location.filename, "bar.ksj");
}

// ============================================================================
// 환경 변수 테스트
// ============================================================================

TEST_F(CallStackTest, ShouldPreserveEnvironmentInFrame) {
    // Arrange
    auto env = std::make_shared<Environment>();
    env->set("x", Value::createInteger(10));
    env->set("y", Value::createString("테스트"));

    // Act
    stack->push(CallStack::StackFrame("함수1", SourceLocation("test.ksj", 10, 1), env));

    // Assert
    auto& frame = stack->current();
    EXPECT_EQ(frame.env->get("x").asInteger(), 10);
    EXPECT_EQ(frame.env->get("y").asString(), "테스트");
}

TEST_F(CallStackTest, ShouldHandleDifferentEnvironmentsPerFrame) {
    // Arrange
    auto env1 = std::make_shared<Environment>();
    env1->set("x", Value::createInteger(10));

    auto env2 = std::make_shared<Environment>();
    env2->set("x", Value::createInteger(20));

    auto env3 = std::make_shared<Environment>();
    env3->set("x", Value::createInteger(30));

    // Act
    stack->push(CallStack::StackFrame("함수1", SourceLocation("test.ksj", 10, 1), env1));
    stack->push(CallStack::StackFrame("함수2", SourceLocation("test.ksj", 20, 1), env2));
    stack->push(CallStack::StackFrame("함수3", SourceLocation("test.ksj", 30, 1), env3));

    // Assert
    const auto& frames = stack->getAll();
    EXPECT_EQ(frames[0].env->get("x").asInteger(), 10);
    EXPECT_EQ(frames[1].env->get("x").asInteger(), 20);
    EXPECT_EQ(frames[2].env->get("x").asInteger(), 30);
}

// ============================================================================
// RAII 및 메모리 안전성 테스트
// ============================================================================

TEST_F(CallStackTest, ShouldNotLeakOnRepeatedPushPop) {
    // AddressSanitizer로 메모리 누수 검증
    for (int i = 0; i < 1000; i++) {
        auto env = std::make_shared<Environment>();
        env->set("i", Value::createInteger(i));
        stack->push(CallStack::StackFrame("함수" + std::to_string(i),
                                          SourceLocation("test.ksj", i + 1, 1),
                                          env));
        stack->pop();
    }

    // 모두 제거되었는지 확인
    EXPECT_TRUE(stack->empty());
}

TEST_F(CallStackTest, ShouldHandleMoveSemantics) {
    // Arrange
    auto env = std::make_shared<Environment>();
    stack->push(CallStack::StackFrame("함수1", SourceLocation("test.ksj", 10, 1), env));

    // Act - move assignment
    auto stack2 = std::make_unique<CallStack>();
    *stack2 = std::move(*stack);

    // Assert
    EXPECT_EQ(stack2->depth(), 1);
    EXPECT_EQ(stack2->current().functionName, "함수1");
}

// ============================================================================
// 경계 조건 테스트
// ============================================================================

TEST_F(CallStackTest, ShouldHandleEmptyFunctionName) {
    // Arrange
    auto env = std::make_shared<Environment>();

    // Act
    stack->push(CallStack::StackFrame("", SourceLocation("test.ksj", 10, 1), env));

    // Assert
    EXPECT_EQ(stack->current().functionName, "");
}

TEST_F(CallStackTest, ShouldHandleDeepCallStack) {
    // Arrange - 깊은 호출 스택 (재귀 시뮬레이션)
    auto env = std::make_shared<Environment>();
    const int depth = 100;

    // Act
    for (int i = 0; i < depth; i++) {
        stack->push(CallStack::StackFrame("재귀함수",
                                          SourceLocation("test.ksj", i + 1, 1),
                                          env));
    }

    // Assert
    EXPECT_EQ(stack->depth(), depth);

    // Act - 모두 pop
    for (int i = 0; i < depth; i++) {
        stack->pop();
    }

    // Assert
    EXPECT_TRUE(stack->empty());
}

TEST_F(CallStackTest, ShouldClearStack) {
    // Arrange
    auto env = std::make_shared<Environment>();
    stack->push(CallStack::StackFrame("함수1", SourceLocation("test.ksj", 10, 1), env));
    stack->push(CallStack::StackFrame("함수2", SourceLocation("test.ksj", 20, 1), env));
    stack->push(CallStack::StackFrame("함수3", SourceLocation("test.ksj", 30, 1), env));

    // Act
    stack->clear();

    // Assert
    EXPECT_TRUE(stack->empty());
    EXPECT_EQ(stack->depth(), 0);
}

// ============================================================================
// 상수 메서드 테스트
// ============================================================================

TEST_F(CallStackTest, ShouldProvideConstAccess) {
    // Arrange
    auto env = std::make_shared<Environment>();
    stack->push(CallStack::StackFrame("함수1", SourceLocation("test.ksj", 10, 1), env));

    // Act - const 참조로 접근
    const CallStack& constStack = *stack;

    // Assert
    EXPECT_FALSE(constStack.empty());
    EXPECT_EQ(constStack.depth(), 1);
    EXPECT_EQ(constStack.getAll().size(), 1);
}
