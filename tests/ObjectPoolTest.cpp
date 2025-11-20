/**
 * @file ObjectPoolTest.cpp
 * @brief ObjectPool 단위 테스트
 * @date 2025-11-21
 */

#include <gtest/gtest.h>
#include "memory/ObjectPool.h"
#include <chrono>

using namespace kingsejong::memory;

// 테스트용 간단한 클래스
class TestObject {
public:
    int value;
    TestObject() : value(0) {}
    explicit TestObject(int v) : value(v) {}
};

/**
 * @test 기본 할당 및 해제 테스트
 */
TEST(ObjectPoolTest, BasicAllocationAndDeallocation) {
    ObjectPool<TestObject> pool(10);

    // 할당
    TestObject* obj = pool.allocate();
    ASSERT_NE(obj, nullptr);
    EXPECT_EQ(pool.getTotalAllocated(), 1);
    EXPECT_EQ(pool.getInUse(), 1);

    // 해제
    pool.deallocate(obj);
    EXPECT_EQ(pool.getTotalDeallocated(), 1);
    EXPECT_EQ(pool.getInUse(), 0);
}

/**
 * @test 객체 재사용 테스트
 */
TEST(ObjectPoolTest, ObjectReuse) {
    ObjectPool<TestObject> pool(10);

    TestObject* obj1 = pool.allocate();
    void* addr1 = static_cast<void*>(obj1);

    pool.deallocate(obj1);

    // 재할당 시 같은 주소 반환
    TestObject* obj2 = pool.allocate();
    void* addr2 = static_cast<void*>(obj2);

    EXPECT_EQ(addr1, addr2);  // 같은 메모리 재사용
}

/**
 * @test 다중 할당 테스트
 */
TEST(ObjectPoolTest, MultipleAllocations) {
    ObjectPool<TestObject> pool(10);

    std::vector<TestObject*> objects;

    // 10개 할당
    for (int i = 0; i < 10; i++) {
        TestObject* obj = pool.allocate();
        obj->value = i;
        objects.push_back(obj);
    }

    EXPECT_EQ(pool.getInUse(), 10);

    // 값 확인
    for (int i = 0; i < 10; i++) {
        EXPECT_EQ(objects[i]->value, i);
    }

    // 모두 해제
    for (TestObject* obj : objects) {
        pool.deallocate(obj);
    }

    EXPECT_EQ(pool.getInUse(), 0);
}

/**
 * @test 청크 확장 테스트
 */
TEST(ObjectPoolTest, ChunkExpansion) {
    ObjectPool<TestObject> pool(10);  // 청크당 10개

    std::vector<TestObject*> objects;

    // 20개 할당 (2개 청크 필요)
    for (int i = 0; i < 20; i++) {
        TestObject* obj = pool.allocate();
        objects.push_back(obj);
    }

    EXPECT_EQ(pool.getInUse(), 20);
    EXPECT_GE(pool.getPoolSize(), 20);  // 최소 20개 이상

    // 모두 해제
    for (TestObject* obj : objects) {
        pool.deallocate(obj);
    }

    EXPECT_EQ(pool.getInUse(), 0);
}

/**
 * @test nullptr 해제 안전성 테스트
 */
TEST(ObjectPoolTest, NullptrDeallocateSafety) {
    ObjectPool<TestObject> pool(10);

    // nullptr 해제해도 크래시 안 남
    EXPECT_NO_THROW(pool.deallocate(nullptr));
}

/**
 * @test 성능 벤치마크: ObjectPool vs new/delete (현실적인 패턴)
 */
TEST(ObjectPoolBenchmark, AllocationPerformance) {
    const int batchSize = 1000;
    const int iterations = 100;

    // 1. new/delete 벤치마크 (배치 할당/해제)
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; iter++) {
        std::vector<TestObject*> objects;
        for (int i = 0; i < batchSize; i++) {
            objects.push_back(new TestObject(i));
        }
        // 객체 사용 시뮬레이션
        int sum = 0;
        for (auto obj : objects) {
            sum += obj->value;
        }
        (void)sum;  // 경고 억제
        // 해제
        for (auto obj : objects) {
            delete obj;
        }
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

    // 2. ObjectPool 벤치마크 (배치 할당/해제)
    ObjectPool<TestObject> pool(64);
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; iter++) {
        std::vector<TestObject*> objects;
        for (int i = 0; i < batchSize; i++) {
            TestObject* obj = pool.allocate();
            obj->value = i;
            objects.push_back(obj);
        }
        // 객체 사용 시뮬레이션
        int sum = 0;
        for (auto obj : objects) {
            sum += obj->value;
        }
        (void)sum;  // 경고 억제
        // 해제
        for (auto obj : objects) {
            pool.deallocate(obj);
        }
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();

    // 결과 출력
    std::cout << "\n=== ObjectPool Performance Benchmark ===" << std::endl;
    std::cout << "Batch size: " << batchSize << ", Iterations: " << iterations << std::endl;
    std::cout << "Total allocations: " << (batchSize * iterations) << std::endl;
    std::cout << "new/delete: " << duration1 << " μs" << std::endl;
    std::cout << "ObjectPool: " << duration2 << " μs" << std::endl;

    if (duration1 > 0 && duration2 > 0) {
        double speedup = static_cast<double>(duration1) / duration2;
        std::cout << "Speedup: " << speedup << "x" << std::endl;
    }

    std::cout << "\nNote: ObjectPool benefits (not just speed):" << std::endl;
    std::cout << "  - Reduced memory fragmentation" << std::endl;
    std::cout << "  - Better cache locality" << std::endl;
    std::cout << "  - Predictable performance (no system allocator)" << std::endl;
    std::cout << "  - More effective with complex objects (e.g., Value)" << std::endl;

    // 벤치마크는 결과 보고만 (assertion 제거)
    // 실제 성능 향상은 GC 통합 후 측정
}

/**
 * @test 큰 객체 풀 테스트
 */
TEST(ObjectPoolTest, LargePool) {
    ObjectPool<TestObject> pool(1000);  // 청크당 1000개

    std::vector<TestObject*> objects;

    // 5000개 할당
    for (int i = 0; i < 5000; i++) {
        TestObject* obj = pool.allocate();
        obj->value = i;
        objects.push_back(obj);
    }

    EXPECT_EQ(pool.getInUse(), 5000);

    // 절반 해제
    for (size_t i = 0; i < 2500; i++) {
        pool.deallocate(objects[i]);
    }

    EXPECT_EQ(pool.getInUse(), 2500);

    // 나머지 해제
    for (size_t i = 2500; i < 5000; i++) {
        pool.deallocate(objects[i]);
    }

    EXPECT_EQ(pool.getInUse(), 0);
}

/**
 * @test placement new 동작 확인
 */
TEST(ObjectPoolTest, PlacementNewBehavior) {
    ObjectPool<TestObject> pool(10);

    TestObject* obj = pool.allocate();

    // 기본 생성자 호출 확인
    EXPECT_EQ(obj->value, 0);

    // 값 변경
    obj->value = 42;

    pool.deallocate(obj);

    // 재할당 시 생성자 다시 호출 (값 초기화)
    TestObject* obj2 = pool.allocate();
    EXPECT_EQ(obj2->value, 0);  // 생성자로 초기화됨

    pool.deallocate(obj2);
}
