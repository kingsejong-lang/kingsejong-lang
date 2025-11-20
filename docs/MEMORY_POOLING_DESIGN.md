# 메모리 풀링 설계 (Memory Pooling Strategy)

> **날짜**: 2025-11-21
> **목표**: Phase 6.3 - 메모리 할당/해제 성능 2~5배 향상
> **상태**: 설계 단계

---

## 1. 목표

### 성능 목표
- ✅ **할당 속도**: `new/delete` 대비 2~5배 향상
- ✅ **메모리 단편화 감소**: 연속적인 메모리 블록 사용
- ✅ **캐시 친화성**: 같은 타입 객체를 인접 메모리에 배치

### 적용 대상
- **Value 객체**: 가장 빈번하게 할당/해제되는 객체
- **Function 객체**: 함수 리터럴 생성 시
- **ClassInstance 객체**: 클래스 인스턴스 생성 시
- **작은 문자열**: 10~64바이트 크기의 문자열

---

## 2. 현재 문제점

### 2.1 `new/delete`의 성능 문제
```cpp
// 현재 구현 (느림)
Value* val = new Value(42);  // 시스템 allocator 호출
delete val;                   // 시스템 deallocator 호출
```

**문제점**:
- 시스템 allocator는 범용적이라 느림
- lock contention (멀티스레드)
- 메모리 단편화
- 객체마다 할당/해제 시스템 호출

### 2.2 빈번한 할당/해제
```ksj
# 예제: 루프에서 빈번한 Value 생성
i가 0부터 10000 미만 {
    x = i * 2          # Value 할당
    y = x + 1          # Value 할당
    출력(y)             # 사용 후 즉시 GC 대상
}
```

**10,000번 반복 시**:
- `new` 호출: 20,000회
- `delete` 호출: 20,000회
- **총 40,000회의 시스템 호출!**

---

## 3. 메모리 풀링 전략

### 3.1 객체 풀 (Object Pool)

#### 구조
```cpp
template<typename T>
class ObjectPool {
private:
    std::vector<T*> freeList_;      // 재사용 가능한 객체 리스트
    std::vector<Chunk*> chunks_;    // 메모리 청크
    size_t chunkSize_ = 64;         // 청크당 객체 수

public:
    T* allocate();                  // 객체 할당
    void deallocate(T* obj);        // 객체 반환 (delete 대신)
};
```

#### 작동 방식
1. **할당 요청**:
   - freeList가 비어있지 않으면 → 재사용
   - 비어있으면 → 새로운 Chunk 할당

2. **해제 요청**:
   - 객체를 소멸시키지 않고 freeList에 추가
   - 나중에 재사용

#### 장점
- ✅ 할당 속도: O(1)
- ✅ 해제 속도: O(1)
- ✅ 캐시 친화성: 같은 청크 내 객체들은 메모리상 인접

### 3.2 크기별 풀 (Size-based Pool)

#### Small Object Pool
```cpp
class SmallObjectPool {
private:
    std::array<ObjectPool<void>, 16> pools_;  // 16, 32, 48, ... 256바이트

public:
    void* allocate(size_t size);
    void deallocate(void* ptr, size_t size);
};
```

#### 크기 클래스
| 크기 범위 | 풀 인덱스 | 용도 |
|-----------|----------|------|
| 1-16 바이트 | 0 | 작은 Value |
| 17-32 바이트 | 1 | 중간 Value |
| 33-48 바이트 | 2 | 큰 Value |
| 49-64 바이트 | 3 | Function 포인터 |
| ... | ... | ... |
| 241-256 바이트 | 15 | 큰 객체 |

---

## 4. 구현 계획

### Phase 1: ObjectPool 템플릿 구현
```cpp
// src/memory/ObjectPool.h
template<typename T>
class ObjectPool {
private:
    struct Chunk {
        T* memory;              // 객체 배열
        size_t capacity;        // 용량
        std::vector<bool> used; // 사용 여부 비트맵
    };

    std::vector<Chunk*> chunks_;
    std::stack<T*> freeList_;
    size_t chunkSize_;

public:
    ObjectPool(size_t chunkSize = 64);
    ~ObjectPool();

    T* allocate();
    void deallocate(T* obj);

    size_t getStats() const;
};
```

### Phase 2: Value 전용 풀
```cpp
// src/evaluator/ValuePool.h
class ValuePool {
private:
    ObjectPool<Value> pool_;

public:
    static ValuePool& getInstance();

    Value* allocate();
    void deallocate(Value* val);
};
```

### Phase 3: GC 통합
```cpp
// GC와 ObjectPool 통합
class GarbageCollector {
private:
    ValuePool& valuePool_;

public:
    size_t sweepPhase() {
        for (Object* obj : toDelete) {
            if (Value* val = dynamic_cast<Value*>(obj)) {
                valuePool_.deallocate(val);  // 풀에 반환
            } else {
                delete obj;  // 일반 삭제
            }
        }
    }
};
```

---

## 5. 성능 예측

### 5.1 벤치마크 시나리오

#### Before (현재)
```cpp
// 10,000번 할당/해제
for (int i = 0; i < 10000; i++) {
    Value* val = new Value(i);
    delete val;
}
// 예상 시간: ~5ms (시스템 allocator)
```

#### After (메모리 풀링)
```cpp
// 10,000번 할당/해제
ValuePool& pool = ValuePool::getInstance();
for (int i = 0; i < 10000; i++) {
    Value* val = pool.allocate();
    pool.deallocate(val);
}
// 예상 시간: ~1ms (5배 빠름)
```

### 5.2 예상 성능 향상
| 작업 | Before | After | 향상율 |
|------|--------|-------|--------|
| Value 할당 | 50 ns | 10 ns | **5배** |
| Value 해제 | 50 ns | 5 ns | **10배** |
| 메모리 단편화 | 높음 | 낮음 | **개선** |
| 캐시 미스 | 높음 | 낮음 | **개선** |

---

## 6. 구현 단계

### ✅ Step 1: ObjectPool 템플릿 (완료)
- [x] ObjectPool.h 작성 (169줄)
- [x] ObjectPool 구현 (allocate, deallocate)
- [x] 단위 테스트 작성 (8개 테스트)
- [x] 커밋: 24969f3

### ✅ Step 2: ValuePool 래퍼 (완료)
- [x] ValuePool.h 싱글톤 구현 (125줄)
- [x] ObjectPool<Value> 래퍼
- [x] 테스트 작성 (8개 테스트)
- [x] 커밋: 09a5805

### ⏸️ Step 3: GC 통합 (보류)
- [ ] GC sweepPhase에서 풀 사용 고려
- [ ] 벤치마크 결과가 negative이므로 재검토 필요

### ✅ Step 4: 벤치마크 및 분석 (완료)
- [x] ObjectPool 벤치마크: 단순 객체에서 2.2배 느림
- [x] ValuePool 벤치마크: 복잡한 객체에서 2.4배 느림
- [x] 원인 분석: placement new 오버헤드 > 시스템 allocator

### ✅ Step 5: 문서화 및 커밋 (완료)
- [x] ObjectPool & ValuePool 구현 완료
- [x] 전체 1,506개 테스트 100% 통과
- [x] 문서 업데이트

**실제 소요 시간**: 약 3시간

---

## 7. 고려 사항

### 7.1 멀티스레딩
- 현재는 싱글스레드만 지원
- 향후 멀티스레드 지원 시:
  - Thread-local pool 사용
  - Lock-free 자료구조 적용

### 7.2 메모리 제한
- 풀 크기 제한 필요
- 최대 메모리 사용량: 10MB
- 초과 시 일반 `new/delete` 폴백

### 7.3 디버깅
- 메모리 누수 탐지 어려움
- 디버그 모드에서는 풀 비활성화 옵션 제공

---

## 8. 대안: 증분 GC vs 메모리 풀링

| 항목 | 증분 GC | 메모리 풀링 |
|------|---------|------------|
| **구현 복잡도** | 높음 (Write barrier 필요) | 중간 |
| **성능 향상** | Pause time 감소 | 할당/해제 속도 향상 |
| **적용 범위** | GC 전체 | 특정 객체 타입 |
| **구현 시간** | 2주 | 5시간 |
| **우선순위** | 낮음 | **높음** ✅ |

**결론**: **메모리 풀링을 먼저 구현**하는 것이 실용적

---

## 9. 테스트 계획

### 9.1 단위 테스트
```cpp
TEST(ObjectPoolTest, AllocateAndDeallocate) {
    ObjectPool<Value> pool(10);

    Value* val = pool.allocate();
    EXPECT_NE(val, nullptr);

    pool.deallocate(val);

    // 재사용 확인
    Value* val2 = pool.allocate();
    EXPECT_EQ(val, val2);  // 같은 포인터 재사용
}
```

### 9.2 성능 테스트
```cpp
TEST(ValuePoolBenchmark, AllocationSpeed) {
    ValuePool& pool = ValuePool::getInstance();

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 100000; i++) {
        Value* val = pool.allocate();
        pool.deallocate(val);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 목표: 100,000번 할당/해제가 100ms 이내
    EXPECT_LT(duration.count(), 100);
}
```

### 9.3 회귀 테스트
- 전체 1,490개 테스트 100% 통과 확인
- 기존 코드와 동일한 동작 보장

---

## 10. 벤치마크 결과 및 분석

### 10.1 ObjectPool 벤치마크

**테스트 환경**:
- 배치 크기: 1000개
- 반복 횟수: 100회
- 총 할당/해제: 100,000회

**결과**:
```
new/delete:  4,390 μs
ObjectPool:  9,519 μs
Speedup:     0.46x (2.2배 느림)
```

### 10.2 ValuePool 벤치마크

**테스트 환경**:
- 배치 크기: 1000개
- 반복 횟수: 100회
- 총 할당/해제: 100,000회

**결과**:
```
new/delete:  20,434 μs
ValuePool:   48,876 μs
Speedup:     0.42x (2.4배 느림)
```

### 10.3 원인 분석

**왜 ObjectPool이 느린가?**

1. **Placement new 오버헤드**
   - 명시적 생성자 호출 비용
   - 명시적 소멸자 호출 비용

2. **시스템 allocator 최적화**
   - macOS의 `malloc` zone allocator 매우 빠름
   - 단순 객체에 최적화됨
   - 캐시 친화적인 fast path

3. **Value 객체의 복잡성**
   - `std::variant<...>` 생성자/소멸자
   - `std::shared_ptr` 참조 카운팅
   - 복잡한 초기화 로직

### 10.4 ObjectPool의 실제 장점

속도는 느리지만, 다음과 같은 장점이 있습니다:

1. **메모리 단편화 감소**
   - 장기 실행 프로그램에서 효과적
   - 연속적인 메모리 블록 사용

2. **캐시 지역성 향상**
   - 같은 청크의 객체들은 메모리상 인접
   - L1/L2 캐시 히트율 향상

3. **예측 가능한 성능**
   - 시스템 allocator 호출 없음
   - GC pause time 감소 가능

4. **특수 용도에 효과적**
   - 매우 큰 객체
   - 생성자/소멸자가 비싼 객체
   - Real-time 시스템

### 10.5 결론

**메모리 풀링은 언제 사용해야 하는가?**

✅ **사용하는 경우**:
- 매우 큰 객체 (>1KB)
- 생성자/소멸자가 비싼 객체
- Real-time 시스템 (예측 가능한 latency 필요)
- 장기 실행 프로그램 (메모리 단편화 방지)

❌ **사용하지 않는 경우**:
- 단순하고 작은 객체 (<100 bytes)
- 빠른 생성자/소멸자
- 단기 실행 프로그램
- 시스템 allocator가 충분히 빠른 경우

**KingSejong 언어의 경우**:
- Value 객체는 복잡하지만, 시스템 allocator가 더 빠름
- GC와 통합 시 메모리 단편화 감소 효과는 있을 수 있음
- **현재는 GC 통합 보류, 향후 필요시 재검토**

---

## 11. 성공 기준

- ✅ ObjectPool 구현 완료
- ❌ Value 할당 속도 향상 (오히려 2.4배 느림)
- ✅ 전체 테스트 1,506개 100% 통과
- ✅ 메모리 사용량 정상
- ✅ 벤치마크 결과 문서화 및 분석 완료

---

## 12. 다음 단계

### 대안 1: 증분 GC (Incremental GC)
- 메모리 풀링 대신 GC pause time 개선
- Write barrier 구현
- 더 복잡하지만 근본적인 해결책

### 대안 2: 특수 목적 풀
- 큰 배열 전용 풀
- Function 객체 전용 풀
- Value 일반 풀 대신 특수 케이스 풀

### 대안 3: 현재 구조 유지
- 시스템 allocator가 충분히 빠름
- Generational GC가 이미 구현됨
- 추가 최적화 불필요

**권장 사항**: **대안 3** - 현재 구조 유지하고 다른 기능 개발에 집중
