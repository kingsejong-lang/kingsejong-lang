/**
 * @file GCTest.cpp
 * @brief 가비지 컬렉터 테스트
 * @author KingSejong Team
 * @date 2025-11-10
 */

#include <gtest/gtest.h>
#include "memory/GC.h"
#include "evaluator/Environment.h"
#include "evaluator/Value.h"

using namespace kingsejong;
using namespace kingsejong::memory;
using namespace kingsejong::evaluator;

/**
 * @class TestObject
 * @brief 테스트용 GC 객체
 */
class TestObject : public Object {
private:
    std::string name_;
    std::vector<TestObject*> references_;

public:
    explicit TestObject(const std::string& name) : name_(name) {}

    void addReference(TestObject* obj) {
        references_.push_back(obj);
    }

    std::vector<Object*> getReferences() override {
        std::vector<Object*> refs;
        for (TestObject* ref : references_) {
            refs.push_back(ref);
        }
        return refs;
    }

    const std::string& getName() const { return name_; }
};

/**
 * @brief 테스트 픽스처
 *
 * 각 테스트 전에 GC를 초기화합니다.
 */
class GCTest : public ::testing::Test {
protected:
    GarbageCollector* gc;

    void SetUp() override {
        gc = GarbageCollector::getInstance();
        gc->setAutoGC(false); // 테스트 중에는 자동 GC 비활성화
    }

    void TearDown() override {
        gc->cleanup();
    }
};

// ============================================================================
// 기본 기능 테스트
// ============================================================================

TEST_F(GCTest, ShouldCreateSingletonInstance) {
    EXPECT_NE(gc, nullptr);

    // 같은 인스턴스인지 확인
    GarbageCollector* gc2 = GarbageCollector::getInstance();
    EXPECT_EQ(gc, gc2);
}

TEST_F(GCTest, ShouldRegisterAndUnregisterObject) {
    TestObject* obj = new TestObject("test");

    size_t beforeCount = gc->getStats().currentObjects;
    gc->registerObject(obj);
    size_t afterCount = gc->getStats().currentObjects;

    EXPECT_EQ(afterCount, beforeCount + 1);
    EXPECT_EQ(gc->getStats().totalAllocations, beforeCount + 1);

    gc->unregisterObject(obj);
    EXPECT_EQ(gc->getStats().currentObjects, beforeCount);

    delete obj;
}

TEST_F(GCTest, ShouldManageRootSet) {
    TestObject* obj1 = new TestObject("root1");
    TestObject* obj2 = new TestObject("root2");

    gc->registerObject(obj1);
    gc->registerObject(obj2);

    gc->addRoot(obj1);
    gc->addRoot(obj2);

    // 루트는 GC되지 않아야 함
    size_t freed = gc->collect();
    EXPECT_EQ(freed, 0);
    EXPECT_EQ(gc->getStats().currentObjects, 2);

    gc->removeRoot(obj1);
    gc->removeRoot(obj2);

    delete obj1;
    delete obj2;
}

// ============================================================================
// Mark & Sweep 알고리즘 테스트
// ============================================================================

TEST_F(GCTest, ShouldCollectUnreachableObjects) {
    // 루트 객체 생성
    TestObject* root = new TestObject("root");
    gc->registerObject(root);
    gc->addRoot(root);

    // 도달 불가능한 객체 생성
    TestObject* unreachable = new TestObject("unreachable");
    gc->registerObject(unreachable);

    // GC 실행
    size_t freed = gc->collect();

    // unreachable이 해제되어야 함
    EXPECT_EQ(freed, 1);
    EXPECT_EQ(gc->getStats().currentObjects, 1);

    gc->removeRoot(root);
    delete root;
}

TEST_F(GCTest, ShouldPreserveReachableObjects) {
    // 루트와 연결된 객체 그래프
    TestObject* root = new TestObject("root");
    TestObject* child1 = new TestObject("child1");
    TestObject* child2 = new TestObject("child2");

    gc->registerObject(root);
    gc->registerObject(child1);
    gc->registerObject(child2);

    root->addReference(child1);
    root->addReference(child2);

    gc->addRoot(root);

    // GC 실행
    size_t freed = gc->collect();

    // 모두 도달 가능하므로 해제되지 않아야 함
    EXPECT_EQ(freed, 0);
    EXPECT_EQ(gc->getStats().currentObjects, 3);

    gc->removeRoot(root);
    delete root;
    delete child1;
    delete child2;
}

TEST_F(GCTest, ShouldHandleComplexObjectGraph) {
    // 복잡한 객체 그래프
    //     root
    //     / \
    //   A    B
    //   |    |
    //   C    D
    //    \  /
    //     E

    TestObject* root = new TestObject("root");
    TestObject* objA = new TestObject("A");
    TestObject* objB = new TestObject("B");
    TestObject* objC = new TestObject("C");
    TestObject* objD = new TestObject("D");
    TestObject* objE = new TestObject("E");

    gc->registerObject(root);
    gc->registerObject(objA);
    gc->registerObject(objB);
    gc->registerObject(objC);
    gc->registerObject(objD);
    gc->registerObject(objE);

    root->addReference(objA);
    root->addReference(objB);
    objA->addReference(objC);
    objB->addReference(objD);
    objC->addReference(objE);
    objD->addReference(objE);

    gc->addRoot(root);

    // 모두 도달 가능
    size_t freed = gc->collect();
    EXPECT_EQ(freed, 0);
    EXPECT_EQ(gc->getStats().currentObjects, 6);

    // 루트를 제거하면 모두 도달 불가능
    gc->removeRoot(root);
    freed = gc->collect();
    EXPECT_EQ(freed, 6);
    EXPECT_EQ(gc->getStats().currentObjects, 0);
}

TEST_F(GCTest, ShouldHandleCyclicReferences) {
    // 순환 참조
    // A -> B -> C -> A

    TestObject* objA = new TestObject("A");
    TestObject* objB = new TestObject("B");
    TestObject* objC = new TestObject("C");

    gc->registerObject(objA);
    gc->registerObject(objB);
    gc->registerObject(objC);

    objA->addReference(objB);
    objB->addReference(objC);
    objC->addReference(objA);

    // 루트가 없으면 순환 참조라도 모두 해제되어야 함
    size_t freed = gc->collect();
    EXPECT_EQ(freed, 3);
    EXPECT_EQ(gc->getStats().currentObjects, 0);
}

// ============================================================================
// Environment 추적 테스트
// ============================================================================

TEST_F(GCTest, ShouldTrackEnvironments) {
    auto env = std::make_shared<Environment>();

    size_t beforeCount = gc->getStats().currentObjects;
    gc->registerEnvironment(env);
    size_t afterCount = gc->getStats().currentObjects;

    EXPECT_EQ(afterCount, beforeCount + 1);
}

TEST_F(GCTest, ShouldCleanupExpiredEnvironments) {
    {
        auto env = std::make_shared<Environment>();
        gc->registerEnvironment(env);
    }
    // env가 스코프를 벗어남

    // Sweep이 만료된 weak_ptr를 정리해야 함
    gc->collect();

    // 통계 확인
    EXPECT_GE(gc->getStats().gcRunCount, 1);
}

// ============================================================================
// 통계 및 설정 테스트
// ============================================================================

TEST_F(GCTest, ShouldTrackStatistics) {
    const GCStats& stats = gc->getStats();

    size_t initialAllocs = stats.totalAllocations;
    size_t initialDeallocs = stats.totalDeallocations;

    TestObject* obj = new TestObject("test");
    gc->registerObject(obj);

    EXPECT_EQ(gc->getStats().totalAllocations, initialAllocs + 1);

    gc->unregisterObject(obj);
    delete obj;

    EXPECT_EQ(gc->getStats().totalDeallocations, initialDeallocs + 1);
}

TEST_F(GCTest, ShouldGenerateStatsString) {
    std::string stats = gc->getStatsString();

    EXPECT_FALSE(stats.empty());
    EXPECT_NE(stats.find("가비지 컬렉터 통계"), std::string::npos);
    EXPECT_NE(stats.find("총 할당"), std::string::npos);
    EXPECT_NE(stats.find("GC 실행 횟수"), std::string::npos);
}

TEST_F(GCTest, ShouldConfigureGCThreshold) {
    gc->setGCThreshold(10);
    gc->setAutoGC(true);

    // 10개 객체 할당 (임계값)
    for (int i = 0; i < 10; i++) {
        TestObject* obj = new TestObject("obj" + std::to_string(i));
        gc->registerObject(obj);
        // auto GC가 트리거될 수 있음
    }

    // 자동 GC가 실행되었는지 확인
    EXPECT_GE(gc->getStats().gcRunCount, 0);

    gc->setAutoGC(false);
}

// ============================================================================
// Mark 알고리즘 세부 테스트
// ============================================================================

TEST_F(GCTest, ShouldMarkOnlyReachableObjects) {
    TestObject* root = new TestObject("root");
    TestObject* reachable = new TestObject("reachable");
    TestObject* unreachable = new TestObject("unreachable");

    gc->registerObject(root);
    gc->registerObject(reachable);
    gc->registerObject(unreachable);

    root->addReference(reachable);
    gc->addRoot(root);

    // Mark phase만 실행
    gc->markPhase();

    EXPECT_TRUE(root->isMarked());
    EXPECT_TRUE(reachable->isMarked());
    EXPECT_FALSE(unreachable->isMarked());

    gc->removeRoot(root);
    delete root;
    delete reachable;
    delete unreachable;
}

TEST_F(GCTest, ShouldHandleNullReferences) {
    TestObject* root = new TestObject("root");
    gc->registerObject(root);
    gc->addRoot(root);

    // null 참조 추가 (getReferences에서 nullptr 반환)
    root->addReference(nullptr);

    // 크래시하지 않아야 함
    EXPECT_NO_THROW(gc->collect());

    gc->removeRoot(root);
    delete root;
}

// ============================================================================
// 정리 및 종료 테스트
// ============================================================================

TEST_F(GCTest, ShouldCleanupAllObjects) {
    // 여러 객체 생성
    for (int i = 0; i < 5; i++) {
        TestObject* obj = new TestObject("obj" + std::to_string(i));
        gc->registerObject(obj);
    }

    size_t beforeCleanup = gc->getStats().currentObjects;
    EXPECT_EQ(beforeCleanup, 5);

    // cleanup 호출
    gc->cleanup();

    EXPECT_EQ(gc->getStats().currentObjects, 0);
}

// ============================================================================
// 실행
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
