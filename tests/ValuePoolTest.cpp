/**
 * @file ValuePoolTest.cpp
 * @brief ValuePool 단위 테스트
 * @date 2025-11-21
 */

#include <gtest/gtest.h>
#include "memory/ValuePool.h"
#include "evaluator/Value.h"
#include <chrono>

using namespace kingsejong::memory;
using namespace kingsejong::evaluator;

/**
 * @test 기본 할당 및 해제 테스트
 */
TEST(ValuePoolTest, BasicAllocationAndDeallocation) {
    ValuePool& pool = ValuePool::getInstance();

    size_t initialAllocated = pool.getTotalAllocated();

    // 할당
    Value* val = pool.allocate();
    ASSERT_NE(val, nullptr);
    EXPECT_EQ(pool.getTotalAllocated(), initialAllocated + 1);
    EXPECT_EQ(pool.getInUse(), 1);

    // 해제
    pool.deallocate(val);
    EXPECT_EQ(pool.getTotalDeallocated(), initialAllocated + 1);
    EXPECT_EQ(pool.getInUse(), 0);
}

/**
 * @test 객체 재사용 테스트
 */
TEST(ValuePoolTest, ObjectReuse) {
    ValuePool& pool = ValuePool::getInstance();

    Value* val1 = pool.allocate();
    void* addr1 = static_cast<void*>(val1);

    pool.deallocate(val1);

    // 재할당 시 같은 주소 반환
    Value* val2 = pool.allocate();
    void* addr2 = static_cast<void*>(val2);

    EXPECT_EQ(addr1, addr2);  // 같은 메모리 재사용

    pool.deallocate(val2);
}

/**
 * @test Value 타입별 할당 테스트
 */
TEST(ValuePoolTest, DifferentValueTypes) {
    ValuePool& pool = ValuePool::getInstance();

    // Integer Value
    Value* intVal = pool.allocate();
    *intVal = Value::createInteger(42);
    EXPECT_EQ(intVal->getType(), kingsejong::types::TypeKind::INTEGER);
    EXPECT_EQ(intVal->asInteger(), 42);
    pool.deallocate(intVal);

    // String Value
    Value* strVal = pool.allocate();
    *strVal = Value::createString("안녕하세요");
    EXPECT_EQ(strVal->getType(), kingsejong::types::TypeKind::STRING);
    EXPECT_EQ(strVal->asString(), "안녕하세요");
    pool.deallocate(strVal);

    // Boolean Value
    Value* boolVal = pool.allocate();
    *boolVal = Value::createBoolean(true);
    EXPECT_EQ(boolVal->getType(), kingsejong::types::TypeKind::BOOLEAN);
    EXPECT_EQ(boolVal->asBoolean(), true);
    pool.deallocate(boolVal);

    // Null Value
    Value* nullVal = pool.allocate();
    *nullVal = Value::createNull();
    EXPECT_EQ(nullVal->getType(), kingsejong::types::TypeKind::NULL_TYPE);
    pool.deallocate(nullVal);
}

/**
 * @test 다중 할당 테스트
 */
TEST(ValuePoolTest, MultipleAllocations) {
    ValuePool& pool = ValuePool::getInstance();

    std::vector<Value*> values;

    // 100개 할당
    for (int i = 0; i < 100; i++) {
        Value* val = pool.allocate();
        *val = Value::createInteger(i);
        values.push_back(val);
    }

    // 값 확인
    for (int i = 0; i < 100; i++) {
        EXPECT_EQ(values[i]->asInteger(), i);
    }

    // 모두 해제
    for (Value* val : values) {
        pool.deallocate(val);
    }
}

/**
 * @test nullptr 해제 안전성 테스트
 */
TEST(ValuePoolTest, NullptrDeallocateSafety) {
    ValuePool& pool = ValuePool::getInstance();

    // nullptr 해제해도 크래시 안 남
    EXPECT_NO_THROW(pool.deallocate(nullptr));
}

/**
 * @test 통계 정보 테스트
 */
TEST(ValuePoolTest, Statistics) {
    ValuePool& pool = ValuePool::getInstance();

    size_t initialAllocated = pool.getTotalAllocated();
    size_t initialDeallocated = pool.getTotalDeallocated();

    // 10개 할당
    std::vector<Value*> values;
    for (int i = 0; i < 10; i++) {
        values.push_back(pool.allocate());
    }

    EXPECT_EQ(pool.getTotalAllocated(), initialAllocated + 10);
    EXPECT_EQ(pool.getInUse(), 10);

    // 5개 해제
    for (int i = 0; i < 5; i++) {
        pool.deallocate(values[i]);
    }

    EXPECT_EQ(pool.getTotalDeallocated(), initialDeallocated + 5);
    EXPECT_EQ(pool.getInUse(), 5);

    // 나머지 해제
    for (int i = 5; i < 10; i++) {
        pool.deallocate(values[i]);
    }

    EXPECT_EQ(pool.getInUse(), 0);
}

/**
 * @test 싱글톤 패턴 테스트
 */
TEST(ValuePoolTest, SingletonPattern) {
    ValuePool& pool1 = ValuePool::getInstance();
    ValuePool& pool2 = ValuePool::getInstance();

    // 같은 인스턴스인지 확인
    EXPECT_EQ(&pool1, &pool2);
}

/**
 * @test 성능 벤치마크: ValuePool vs new/delete
 */
TEST(ValuePoolBenchmark, AllocationPerformance) {
    const int batchSize = 1000;
    const int iterations = 100;

    // 1. new/delete 벤치마크
    auto start1 = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; iter++) {
        std::vector<Value*> values;
        for (int i = 0; i < batchSize; i++) {
            values.push_back(new Value(Value::createInteger(i)));
        }
        // 객체 사용 시뮬레이션
        int sum = 0;
        for (auto val : values) {
            sum += val->asInteger();
        }
        (void)sum;  // 경고 억제
        // 해제
        for (auto val : values) {
            delete val;
        }
    }
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count();

    // 2. ValuePool 벤치마크
    ValuePool& pool = ValuePool::getInstance();
    auto start2 = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < iterations; iter++) {
        std::vector<Value*> values;
        for (int i = 0; i < batchSize; i++) {
            Value* val = pool.allocate();
            *val = Value::createInteger(i);
            values.push_back(val);
        }
        // 객체 사용 시뮬레이션
        int sum = 0;
        for (auto val : values) {
            sum += val->asInteger();
        }
        (void)sum;  // 경고 억제
        // 해제
        for (auto val : values) {
            pool.deallocate(val);
        }
    }
    auto end2 = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count();

    // 결과 출력
    std::cout << "\n=== ValuePool Performance Benchmark ===" << std::endl;
    std::cout << "Batch size: " << batchSize << ", Iterations: " << iterations << std::endl;
    std::cout << "Total allocations: " << (batchSize * iterations) << std::endl;
    std::cout << "new/delete: " << duration1 << " μs" << std::endl;
    std::cout << "ValuePool:  " << duration2 << " μs" << std::endl;

    if (duration1 > 0 && duration2 > 0) {
        double speedup = static_cast<double>(duration1) / duration2;
        std::cout << "Speedup: " << speedup << "x" << std::endl;

        if (speedup > 1.0) {
            std::cout << "✓ ValuePool is " << speedup << "x faster!" << std::endl;
        } else {
            std::cout << "✗ ValuePool is slower (complex Value objects)" << std::endl;
        }
    }

    std::cout << "\nNote: ValuePool benefits:" << std::endl;
    std::cout << "  - Reduced memory fragmentation" << std::endl;
    std::cout << "  - Better cache locality" << std::endl;
    std::cout << "  - Predictable performance" << std::endl;
    std::cout << "  - Effective for long-running programs" << std::endl;
}
