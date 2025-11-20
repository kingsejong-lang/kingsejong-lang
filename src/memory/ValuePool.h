#pragma once

/**
 * @file ValuePool.h
 * @brief Value 객체 전용 메모리 풀
 * @author KingSejong Team
 * @date 2025-11-21
 *
 * Phase 6.3: 메모리 풀링 전략
 * Value 객체의 할당/해제 성능을 향상시키기 위한 싱글톤 풀
 */

#include "memory/ObjectPool.h"
#include "evaluator/Value.h"

namespace kingsejong {
namespace memory {

/**
 * @class ValuePool
 * @brief Value 객체 전용 싱글톤 메모리 풀
 *
 * ObjectPool<Value>를 감싸는 래퍼 클래스입니다.
 * 싱글톤 패턴으로 구현되어 전역적으로 하나의 풀만 존재합니다.
 *
 * 사용법:
 * @code
 * ValuePool& pool = ValuePool::getInstance();
 * Value* val = pool.allocate();
 * // ... 사용 ...
 * pool.deallocate(val);
 * @endcode
 */
class ValuePool {
private:
    ObjectPool<evaluator::Value> pool_;  ///< 내부 ObjectPool

    /**
     * @brief 생성자 (private - 싱글톤)
     * @param chunkSize 청크당 Value 객체 수 (기본: 128)
     */
    explicit ValuePool(size_t chunkSize = 128)
        : pool_(chunkSize)
    {}

    // 복사/이동 방지
    ValuePool(const ValuePool&) = delete;
    ValuePool& operator=(const ValuePool&) = delete;
    ValuePool(ValuePool&&) = delete;
    ValuePool& operator=(ValuePool&&) = delete;

public:
    /**
     * @brief 싱글톤 인스턴스 반환
     * @return ValuePool 인스턴스 참조
     */
    static ValuePool& getInstance() {
        static ValuePool instance;
        return instance;
    }

    /**
     * @brief Value 객체 할당
     * @return 할당된 Value 포인터 (기본 생성자로 초기화됨)
     *
     * 풀에서 재사용 가능한 객체를 가져오거나,
     * 없으면 새로운 청크를 할당합니다.
     */
    evaluator::Value* allocate() {
        return pool_.allocate();
    }

    /**
     * @brief Value 객체 해제 (풀에 반환)
     * @param val 해제할 Value 포인터
     *
     * 객체를 소멸시키고 풀에 반환합니다.
     * 메모리는 실제로 해제되지 않고 재사용을 위해 보관됩니다.
     */
    void deallocate(evaluator::Value* val) {
        pool_.deallocate(val);
    }

    /**
     * @brief 통계 정보: 총 할당된 객체 수
     * @return 할당 횟수
     */
    size_t getTotalAllocated() const {
        return pool_.getTotalAllocated();
    }

    /**
     * @brief 통계 정보: 총 해제된 객체 수
     * @return 해제 횟수
     */
    size_t getTotalDeallocated() const {
        return pool_.getTotalDeallocated();
    }

    /**
     * @brief 통계 정보: 현재 사용 중인 객체 수
     * @return 사용 중인 객체 수
     */
    size_t getInUse() const {
        return pool_.getInUse();
    }

    /**
     * @brief 풀 크기 (총 용량)
     * @return 모든 청크의 용량 합계
     */
    size_t getPoolSize() const {
        return pool_.getPoolSize();
    }
};

} // namespace memory
} // namespace kingsejong
