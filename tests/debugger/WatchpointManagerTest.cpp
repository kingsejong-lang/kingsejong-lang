/**
 * @file WatchpointManagerTest.cpp
 * @brief WatchpointManager 테스트
 * @author KingSejong Team
 * @date 2025-11-14
 *
 * TDD 방식: 테스트를 먼저 작성하고 구현합니다.
 */

#include <gtest/gtest.h>
#include "debugger/WatchpointManager.h"
#include "evaluator/Environment.h"
#include "evaluator/Value.h"

using namespace kingsejong::debugger;
using namespace kingsejong::evaluator;

/**
 * @class WatchpointManagerTest
 * @brief WatchpointManager 테스트 픽스처
 */
class WatchpointManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<WatchpointManager>();
        env = std::make_shared<Environment>();
    }

    std::unique_ptr<WatchpointManager> manager;
    std::shared_ptr<Environment> env;
};

// ============================================================================
// 기본 기능 테스트
// ============================================================================

TEST_F(WatchpointManagerTest, ShouldAddWatchpoint) {
    // Arrange
    env->set("x", Value::createInteger(10));

    // Act
    bool added = manager->add("x", *env);

    // Assert
    EXPECT_TRUE(added);
    EXPECT_EQ(manager->getAll().size(), 1u);
}

TEST_F(WatchpointManagerTest, ShouldNotAddDuplicateWatchpoint) {
    // Arrange
    env->set("x", Value::createInteger(10));
    manager->add("x", *env);

    // Act
    bool addedAgain = manager->add("x", *env);

    // Assert
    EXPECT_FALSE(addedAgain);
    EXPECT_EQ(manager->getAll().size(), 1u);
}

TEST_F(WatchpointManagerTest, ShouldNotAddNonExistentVariable) {
    // Act
    bool added = manager->add("nonexistent", *env);

    // Assert
    EXPECT_FALSE(added);
    EXPECT_EQ(manager->getAll().size(), 0u);
}

TEST_F(WatchpointManagerTest, ShouldThrowOnEmptyVariableName) {
    // Act & Assert
    EXPECT_THROW(manager->add("", *env), std::invalid_argument);
}

TEST_F(WatchpointManagerTest, ShouldRemoveWatchpoint) {
    // Arrange
    env->set("x", Value::createInteger(10));
    manager->add("x", *env);

    // Act
    bool removed = manager->remove("x");

    // Assert
    EXPECT_TRUE(removed);
    EXPECT_EQ(manager->getAll().size(), 0u);
}

TEST_F(WatchpointManagerTest, ShouldReturnFalseWhenRemovingNonexistentWatchpoint) {
    // Act
    bool removed = manager->remove("nonexistent");

    // Assert
    EXPECT_FALSE(removed);
}

// ============================================================================
// 변경 감지 테스트
// ============================================================================

TEST_F(WatchpointManagerTest, ShouldDetectValueChange) {
    // Arrange
    env->set("x", Value::createInteger(10));
    manager->add("x", *env);

    // Act - 값 변경
    env->set("x", Value::createInteger(20));
    auto changed = manager->checkChanges(*env);

    // Assert
    EXPECT_EQ(changed.size(), 1u);
    EXPECT_EQ(changed[0], "x");
}

TEST_F(WatchpointManagerTest, ShouldNotDetectWhenValueUnchanged) {
    // Arrange
    env->set("x", Value::createInteger(10));
    manager->add("x", *env);

    // Act - 값 변경 없음
    auto changed = manager->checkChanges(*env);

    // Assert
    EXPECT_EQ(changed.size(), 0u);
}

TEST_F(WatchpointManagerTest, ShouldDetectMultipleChanges) {
    // Arrange
    env->set("x", Value::createInteger(10));
    env->set("y", Value::createInteger(20));
    manager->add("x", *env);
    manager->add("y", *env);

    // Act - 두 변수 모두 변경
    env->set("x", Value::createInteger(15));
    env->set("y", Value::createInteger(25));
    auto changed = manager->checkChanges(*env);

    // Assert
    EXPECT_EQ(changed.size(), 2u);
}

TEST_F(WatchpointManagerTest, ShouldUpdateLastValueAfterChange) {
    // Arrange
    env->set("x", Value::createInteger(10));
    manager->add("x", *env);

    // Act - 첫 번째 변경
    env->set("x", Value::createInteger(20));
    manager->checkChanges(*env);

    // Act - 두 번째 체크 (같은 값)
    auto changed = manager->checkChanges(*env);

    // Assert - 변경 없음 (lastValue가 업데이트됨)
    EXPECT_EQ(changed.size(), 0u);
}

TEST_F(WatchpointManagerTest, ShouldTrackChangeCount) {
    // Arrange
    env->set("x", Value::createInteger(10));
    manager->add("x", *env);

    // Act - 3번 변경
    env->set("x", Value::createInteger(20));
    manager->checkChanges(*env);

    env->set("x", Value::createInteger(30));
    manager->checkChanges(*env);

    env->set("x", Value::createInteger(40));
    manager->checkChanges(*env);

    // Assert
    EXPECT_EQ(manager->getChangeCount("x"), 3);
}

// ============================================================================
// 활성화/비활성화 테스트
// ============================================================================

TEST_F(WatchpointManagerTest, ShouldEnableAndDisableWatchpoint) {
    // Arrange
    env->set("x", Value::createInteger(10));
    manager->add("x", *env);

    // Act & Assert - 기본 활성화 상태
    EXPECT_TRUE(manager->isEnabled("x"));

    // Act - 비활성화
    manager->setEnabled("x", false);
    EXPECT_FALSE(manager->isEnabled("x"));

    // Act - 다시 활성화
    manager->setEnabled("x", true);
    EXPECT_TRUE(manager->isEnabled("x"));
}

TEST_F(WatchpointManagerTest, ShouldNotDetectChangesWhenDisabled) {
    // Arrange
    env->set("x", Value::createInteger(10));
    manager->add("x", *env);
    manager->setEnabled("x", false);

    // Act - 값 변경
    env->set("x", Value::createInteger(20));
    auto changed = manager->checkChanges(*env);

    // Assert - 비활성화되어 있으므로 감지 안 됨
    EXPECT_EQ(changed.size(), 0u);
}

TEST_F(WatchpointManagerTest, ShouldThrowWhenCheckingNonexistentWatchpoint) {
    // Act & Assert
    EXPECT_THROW(manager->isEnabled("nonexistent"), std::out_of_range);
    EXPECT_THROW(manager->getChangeCount("nonexistent"), std::out_of_range);
}

// ============================================================================
// 타입 변경 감지 테스트
// ============================================================================

TEST_F(WatchpointManagerTest, ShouldDetectTypeChange) {
    // Arrange
    env->set("x", Value::createInteger(10));
    manager->add("x", *env);

    // Act - 타입 변경 (정수 → 문자열)
    env->set("x", Value::createString("hello"));
    auto changed = manager->checkChanges(*env);

    // Assert
    EXPECT_EQ(changed.size(), 1u);
    EXPECT_EQ(changed[0], "x");
}

TEST_F(WatchpointManagerTest, ShouldDetectStringChange) {
    // Arrange
    env->set("name", Value::createString("Alice"));
    manager->add("name", *env);

    // Act - 문자열 변경
    env->set("name", Value::createString("Bob"));
    auto changed = manager->checkChanges(*env);

    // Assert
    EXPECT_EQ(changed.size(), 1u);
}

TEST_F(WatchpointManagerTest, ShouldDetectBooleanChange) {
    // Arrange
    env->set("flag", Value::createBoolean(true));
    manager->add("flag", *env);

    // Act - 불리언 변경
    env->set("flag", Value::createBoolean(false));
    auto changed = manager->checkChanges(*env);

    // Assert
    EXPECT_EQ(changed.size(), 1u);
}

// ============================================================================
// RAII 및 메모리 안전성 테스트
// ============================================================================

TEST_F(WatchpointManagerTest, ShouldNotLeakOnRepeatedAddRemove) {
    // AddressSanitizer로 메모리 누수 검증
    for (int i = 0; i < 1000; i++) {
        std::string varName = "var" + std::to_string(i);
        env->set(varName, Value::createInteger(i));
        manager->add(varName, *env);
        manager->remove(varName);
    }

    // 모두 제거되었는지 확인
    EXPECT_EQ(manager->getAll().size(), 0u);
}

TEST_F(WatchpointManagerTest, ShouldHandleMoveSemantics) {
    // Arrange
    env->set("x", Value::createInteger(10));
    manager->add("x", *env);

    // Act - move assignment
    auto manager2 = std::make_unique<WatchpointManager>();
    *manager2 = std::move(*manager);

    // Assert
    EXPECT_EQ(manager2->getAll().size(), 1u);
}

TEST_F(WatchpointManagerTest, ShouldClearAllWatchpoints) {
    // Arrange
    env->set("x", Value::createInteger(10));
    env->set("y", Value::createInteger(20));
    env->set("z", Value::createInteger(30));
    manager->add("x", *env);
    manager->add("y", *env);
    manager->add("z", *env);

    // Act
    manager->clear();

    // Assert
    EXPECT_EQ(manager->getAll().size(), 0u);
}

// ============================================================================
// 엣지 케이스 테스트
// ============================================================================

TEST_F(WatchpointManagerTest, ShouldHandleVariableDeleted) {
    // Arrange
    env->set("x", Value::createInteger(10));
    manager->add("x", *env);

    // Act - 변수 삭제 (Environment에서 제거)
    // Note: Environment가 변수 삭제를 지원하지 않으면 이 테스트는 스킵
    // 여기서는 변수가 없는 경우 무시하는 것으로 테스트
    auto env2 = std::make_shared<Environment>();
    auto changed = manager->checkChanges(*env2);

    // Assert - 변수가 없으므로 변경 감지 안 됨
    EXPECT_EQ(changed.size(), 0u);
}

TEST_F(WatchpointManagerTest, ShouldHandleMultipleConsecutiveChanges) {
    // Arrange
    env->set("x", Value::createInteger(10));
    manager->add("x", *env);

    // Act - 연속 변경 (10 → 20 → 30 → 40 → 50 → 60)
    for (int i = 1; i <= 5; i++) {
        env->set("x", Value::createInteger(10 + i * 10));
        auto changed = manager->checkChanges(*env);
        EXPECT_EQ(changed.size(), 1u);
    }

    // Assert
    EXPECT_EQ(manager->getChangeCount("x"), 5);
}
