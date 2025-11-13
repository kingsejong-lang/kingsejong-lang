/**
 * @file BreakpointManagerTest.cpp
 * @brief BreakpointManager 테스트
 * @author KingSejong Team
 * @date 2025-11-11
 *
 * TDD 방식: 테스트를 먼저 작성하고 구현합니다.
 */

#include <gtest/gtest.h>
#include "debugger/BreakpointManager.h"
#include "error/Error.h"
#include "evaluator/Environment.h"
#include "evaluator/Value.h"

using namespace kingsejong::debugger;
using namespace kingsejong::error;
using namespace kingsejong::evaluator;

/**
 * @class BreakpointManagerTest
 * @brief BreakpointManager 테스트 픽스처
 */
class BreakpointManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<BreakpointManager>();
    }

    std::unique_ptr<BreakpointManager> manager;
};

// ============================================================================
// 기본 기능 테스트
// ============================================================================

TEST_F(BreakpointManagerTest, ShouldAddBreakpoint) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);

    // Act
    bool result = manager->add(loc);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(manager->getAll().size(), 1);
}

TEST_F(BreakpointManagerTest, ShouldNotAddDuplicateBreakpoint) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    manager->add(loc);

    // Act
    bool result = manager->add(loc);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(manager->getAll().size(), 1);
}

TEST_F(BreakpointManagerTest, ShouldRemoveBreakpoint) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    manager->add(loc);

    // Act
    bool result = manager->remove(loc);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(manager->getAll().size(), 0);
}

TEST_F(BreakpointManagerTest, ShouldNotRemoveNonExistentBreakpoint) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);

    // Act
    bool result = manager->remove(loc);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(BreakpointManagerTest, ShouldHandleMultipleBreakpoints) {
    // Arrange
    SourceLocation loc1("test.ksj", 10, 1);
    SourceLocation loc2("test.ksj", 20, 1);
    SourceLocation loc3("other.ksj", 15, 1);

    // Act
    manager->add(loc1);
    manager->add(loc2);
    manager->add(loc3);

    // Assert
    EXPECT_EQ(manager->getAll().size(), 3);
}

// ============================================================================
// 조건부 브레이크포인트 테스트
// ============================================================================

TEST_F(BreakpointManagerTest, ShouldAddConditionalBreakpoint) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    std::string condition = "x > 10";

    // Act
    bool result = manager->add(loc, condition);

    // Assert
    EXPECT_TRUE(result);
    const auto& breakpoints = manager->getAll();
    EXPECT_EQ(breakpoints.size(), 1);
    EXPECT_TRUE(breakpoints.at(loc).condition.has_value());
    EXPECT_EQ(breakpoints.at(loc).condition.value(), "x > 10");
}

TEST_F(BreakpointManagerTest, ShouldBreakOnUnconditionalBreakpoint) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    manager->add(loc);
    auto env = std::make_shared<Environment>();

    // Act
    bool shouldBreak = manager->shouldBreak(loc, *env);

    // Assert
    EXPECT_TRUE(shouldBreak);
}

TEST_F(BreakpointManagerTest, ShouldBreakWhenConditionIsTrue) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    manager->add(loc, "x > 10");

    auto env = std::make_shared<Environment>();
    env->set("x", Value::createInteger(15));

    // Act
    bool shouldBreak = manager->shouldBreak(loc, *env);

    // Assert
    EXPECT_TRUE(shouldBreak);
}

TEST_F(BreakpointManagerTest, ShouldNotBreakWhenConditionIsFalse) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    manager->add(loc, "x > 10");

    auto env = std::make_shared<Environment>();
    env->set("x", Value::createInteger(5));

    // Act
    bool shouldBreak = manager->shouldBreak(loc, *env);

    // Assert
    EXPECT_FALSE(shouldBreak);
}

TEST_F(BreakpointManagerTest, ShouldNotBreakAtNonBreakpointLocation) {
    // Arrange
    SourceLocation loc1("test.ksj", 10, 1);
    SourceLocation loc2("test.ksj", 20, 1);
    manager->add(loc1);

    auto env = std::make_shared<Environment>();

    // Act
    bool shouldBreak = manager->shouldBreak(loc2, *env);

    // Assert
    EXPECT_FALSE(shouldBreak);
}

// ============================================================================
// 브레이크포인트 상태 테스트
// ============================================================================

TEST_F(BreakpointManagerTest, ShouldEnableAndDisableBreakpoint) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    manager->add(loc);

    // Act & Assert - 활성화 상태
    EXPECT_TRUE(manager->isEnabled(loc));

    // Act & Assert - 비활성화
    manager->setEnabled(loc, false);
    EXPECT_FALSE(manager->isEnabled(loc));

    // Act & Assert - 다시 활성화
    manager->setEnabled(loc, true);
    EXPECT_TRUE(manager->isEnabled(loc));
}

TEST_F(BreakpointManagerTest, ShouldNotBreakAtDisabledBreakpoint) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    manager->add(loc);
    manager->setEnabled(loc, false);

    auto env = std::make_shared<Environment>();

    // Act
    bool shouldBreak = manager->shouldBreak(loc, *env);

    // Assert
    EXPECT_FALSE(shouldBreak);
}

TEST_F(BreakpointManagerTest, ShouldTrackHitCount) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    manager->add(loc);
    auto env = std::make_shared<Environment>();

    // Act - 3번 중단 체크
    manager->shouldBreak(loc, *env);
    manager->shouldBreak(loc, *env);
    manager->shouldBreak(loc, *env);

    // Assert
    EXPECT_EQ(manager->getHitCount(loc), 3);
}

// ============================================================================
// 에러 처리 테스트
// ============================================================================

TEST_F(BreakpointManagerTest, ShouldThrowOnInvalidLocation) {
    // Arrange
    SourceLocation invalidLoc("", -1, -1);

    // Act & Assert
    EXPECT_THROW(manager->add(invalidLoc), std::invalid_argument);
}

TEST_F(BreakpointManagerTest, ShouldHandleInvalidConditionGracefully) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    manager->add(loc, "invalid syntax +++");

    auto env = std::make_shared<Environment>();

    // Act & Assert - 조건식 파싱 실패 시 중단하지 않음
    EXPECT_FALSE(manager->shouldBreak(loc, *env));
}

// ============================================================================
// RAII 및 메모리 안전성 테스트
// ============================================================================

TEST_F(BreakpointManagerTest, ShouldNotLeakOnRepeatedAddRemove) {
    // AddressSanitizer로 메모리 누수 검증
    for (int i = 1; i <= 1000; i++) {
        SourceLocation loc("test.ksj", i, 1);
        manager->add(loc, "x > " + std::to_string(i));
        manager->remove(loc);
    }

    // 모두 제거되었는지 확인
    EXPECT_EQ(manager->getAll().size(), 0);
}

TEST_F(BreakpointManagerTest, ShouldHandleMoveSemantics) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    manager->add(loc);

    // Act - move assignment
    auto manager2 = std::make_unique<BreakpointManager>();
    *manager2 = std::move(*manager);

    // Assert
    EXPECT_EQ(manager2->getAll().size(), 1);
}

// ============================================================================
// 복잡한 조건식 테스트
// ============================================================================

TEST_F(BreakpointManagerTest, ShouldEvaluateComplexCondition) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    manager->add(loc, "x > 10 && y < 20");

    auto env = std::make_shared<Environment>();
    env->set("x", Value::createInteger(15));
    env->set("y", Value::createInteger(10));

    // Act
    bool shouldBreak = manager->shouldBreak(loc, *env);

    // Assert
    EXPECT_TRUE(shouldBreak);
}

TEST_F(BreakpointManagerTest, ShouldHandleStringComparisonInCondition) {
    // Arrange
    SourceLocation loc("test.ksj", 10, 1);
    manager->add(loc, "name == \"test\"");

    auto env = std::make_shared<Environment>();
    env->set("name", Value::createString("test"));

    // Act
    bool shouldBreak = manager->shouldBreak(loc, *env);

    // Assert
    EXPECT_TRUE(shouldBreak);
}
