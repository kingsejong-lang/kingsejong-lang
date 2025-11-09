# 가비지 컬렉터 설계 문서

> KingSejong 언어의 Mark & Sweep 가비지 컬렉터

**작성일**: 2025-11-10
**상태**: ✅ 구현 완료

---

## 개요

KingSejong 언어는 자동 메모리 관리를 위한 **Mark & Sweep 가비지 컬렉터**를 제공합니다. C++의 `shared_ptr` 기반 메모리 관리와 함께 동작하며, 순환 참조를 감지하고 메모리 누수를 방지합니다.

---

## 설계 원칙

### 1. 하이브리드 접근 (Hybrid Approach)

기존 `shared_ptr` 기반 시스템을 유지하면서 GC 레이어를 추가:
- **shared_ptr**: 기본 메모리 관리 (참조 카운팅)
- **GC**: 순환 참조 감지 및 해제, 메모리 통계

### 2. Mark & Sweep 알고리즘

전통적인 Tracing GC 알고리즘:
1. **Mark Phase**: 루트부터 도달 가능한 모든 객체를 마킹
2. **Sweep Phase**: 마킹되지 않은 객체를 해제

### 3. 최소 침투적 설계 (Minimally Invasive)

기존 코드 수정을 최소화:
- `Value`, `Environment`, `Function`은 그대로 유지
- GC는 선택적으로 사용 가능
- 레거시 코드와 호환성 유지

---

## 아키텍처

### 클래스 다이어그램

```
┌─────────────────────────────────────┐
│      GarbageCollector (Singleton)   │
├─────────────────────────────────────┤
│ - allObjects_: set<Object*>         │
│ - roots_: set<Object*>              │
│ - environments_: map<void*, weak>   │
│ - stats_: GCStats                   │
├─────────────────────────────────────┤
│ + registerObject(obj)               │
│ + addRoot(obj)                      │
│ + collect() -> size_t               │
│ + markPhase()                       │
│ + sweepPhase() -> size_t            │
└─────────────────────────────────────┘
           │
           │ manages
           ▼
┌─────────────────────────────────────┐
│           Object (abstract)         │
├─────────────────────────────────────┤
│ - marked_: bool                     │
│ - id_: uint64_t                     │
├─────────────────────────────────────┤
│ + mark()                            │
│ + unmark()                          │
│ + isMarked() -> bool                │
│ + getReferences() -> vector<Object*>│
└─────────────────────────────────────┘
           △
           │ inherits
           │
   ┌───────┴────────┐
   │                │
┌──┴─────┐    ┌────┴──────┐
│Function│    │CustomObject│
└────────┘    └───────────┘
```

---

## 핵심 컴포넌트

### 1. Object 베이스 클래스

모든 GC 관리 객체의 부모 클래스:

```cpp
class Object {
private:
    bool marked_ = false;      // Mark bit
    uint64_t id_;              // 고유 ID

public:
    void mark();
    void unmark();
    bool isMarked() const;

    // 참조하는 다른 객체들 반환
    virtual std::vector<Object*> getReferences() { return {}; }
};
```

**특징**:
- Mark bit로 도달 가능성 추적
- `getReferences()`로 객체 그래프 탐색
- 고유 ID로 디버깅 지원

### 2. GarbageCollector (Singleton)

전역 GC 매니저:

```cpp
class GarbageCollector {
private:
    std::unordered_set<Object*> allObjects_;  // 모든 객체
    std::unordered_set<Object*> roots_;       // 루트 집합
    GCStats stats_;                           // 통계

public:
    static GarbageCollector* getInstance();

    void registerObject(Object* obj);
    void unregisterObject(Object* obj);

    void addRoot(Object* obj);
    void removeRoot(Object* obj);

    size_t collect();  // Mark & Sweep 실행
};
```

**책임**:
- 모든 GC 객체 추적
- 루트 집합 관리
- GC 알고리즘 실행
- 메모리 통계 수집

### 3. Environment 추적

`shared_ptr<Environment>`를 `weak_ptr`로 추적:

```cpp
std::unordered_map<void*, std::weak_ptr<Environment>> environments_;
```

**이유**:
- Environment는 Object를 상속하지 않음 (기존 코드 유지)
- weak_ptr로 순환 참조 방지
- 자동 만료 감지

---

## Mark & Sweep 알고리즘

### Mark Phase

루트부터 DFS로 도달 가능한 객체 마킹:

```cpp
void GarbageCollector::markPhase() {
    // 1. 모든 마크 해제
    for (Object* obj : allObjects_) {
        obj->unmark();
    }

    // 2. 루트부터 재귀 마킹
    for (Object* root : roots_) {
        markObject(root);
    }
}

void GarbageCollector::markObject(Object* obj) {
    if (obj == nullptr || obj->isMarked()) {
        return;  // 이미 마킹됨
    }

    obj->mark();

    // 참조 객체들도 재귀 마킹
    for (Object* ref : obj->getReferences()) {
        markObject(ref);
    }
}
```

### Sweep Phase

마킹되지 않은 객체 해제:

```cpp
size_t GarbageCollector::sweepPhase() {
    std::vector<Object*> toDelete;

    // 마킹되지 않은 객체 수집
    for (Object* obj : allObjects_) {
        if (!obj->isMarked()) {
            toDelete.push_back(obj);
        }
    }

    // 해제
    for (Object* obj : toDelete) {
        allObjects_.erase(obj);
        delete obj;
    }

    return toDelete.size();
}
```

---

## 루트 집합 (Root Set)

GC의 시작점:

1. **전역 Environment**: 최상위 스코프
2. **평가 스택**: 현재 실행 중인 함수의 로컬 변수
3. **REPL 환경**: 대화형 세션의 변수
4. **명시적 루트**: 사용자가 addRoot()로 추가

```cpp
// 예제: 전역 환경을 루트로 추가
auto globalEnv = std::make_shared<Environment>();
gc->registerEnvironment(globalEnv);

// 명시적 루트
TestObject* root = new TestObject("important");
gc->registerObject(root);
gc->addRoot(root);
```

---

## 자동 GC 트리거

객체 수가 임계값을 넘으면 자동 실행:

```cpp
void GarbageCollector::checkAndCollect() {
    if (!autoGC_) return;

    if (stats_.currentObjects >= gcThreshold_) {
        collect();
    }
}
```

**설정**:
```cpp
gc->setAutoGC(true);
gc->setGCThreshold(100);  // 100개 객체마다 GC
```

---

## 통계 및 모니터링

### GCStats 구조체

```cpp
struct GCStats {
    size_t totalAllocations;     // 총 할당
    size_t totalDeallocations;   // 총 해제
    size_t currentObjects;       // 현재 객체 수
    size_t gcRunCount;           // GC 실행 횟수
    size_t objectsFreed;         // 마지막 GC에서 해제
    size_t totalObjectsFreed;    // 총 GC로 해제
    size_t bytesAllocated;       // 추정 메모리
};
```

### 통계 출력

```cpp
std::string stats = gc->getStatsString();
std::cout << stats << std::endl;
```

출력 예시:
```
=== 가비지 컬렉터 통계 ===
총 할당: 150
총 해제: 120
현재 객체 수: 30
GC 실행 횟수: 5
마지막 GC에서 해제: 8
총 GC로 해제: 35
할당된 메모리 (추정): 2400 bytes
```

---

## 순환 참조 처리

### 문제

```
Function A → Environment 1 → Value → Function B
                ↑                        ↓
                └────── Environment 2 ←──┘
```

shared_ptr만으로는 순환 참조 해제 불가.

### 해결

1. **weak_ptr 사용**: Environment를 weak_ptr로 추적
2. **Mark & Sweep**: 도달 불가능한 순환 참조 감지
3. **자동 정리**: Sweep phase에서 만료된 weak_ptr 제거

```cpp
// Sweep에서 만료된 Environment 정리
std::vector<void*> expiredEnvs;
for (auto& pair : environments_) {
    if (pair.second.expired()) {
        expiredEnvs.push_back(pair.first);
    }
}

for (void* ptr : expiredEnvs) {
    environments_.erase(ptr);
}
```

---

## 사용 예제

### 기본 사용법

```cpp
#include "memory/GC.h"

using namespace kingsejong::memory;

// 1. GC 인스턴스 얻기
GarbageCollector* gc = GarbageCollector::getInstance();

// 2. 객체 등록
TestObject* obj = new TestObject("my_object");
gc->registerObject(obj);

// 3. 루트 설정
gc->addRoot(obj);

// 4. GC 실행
size_t freed = gc->collect();
std::cout << "해제된 객체: " << freed << std::endl;

// 5. 정리
gc->removeRoot(obj);
delete obj;
```

### Environment 통합

```cpp
auto env = std::make_shared<Environment>();

// Environment 등록
gc->registerEnvironment(env);

// 사용...

// 자동으로 정리됨 (weak_ptr)
```

### 객체 그래프

```cpp
// 복잡한 객체 그래프
TestObject* root = new TestObject("root");
TestObject* child1 = new TestObject("child1");
TestObject* child2 = new TestObject("child2");

gc->registerObject(root);
gc->registerObject(child1);
gc->registerObject(child2);

root->addReference(child1);
root->addReference(child2);

gc->addRoot(root);

// 모든 객체 도달 가능 → 해제되지 않음
gc->collect();  // freed = 0

// 루트 제거 → 모두 도달 불가능
gc->removeRoot(root);
gc->collect();  // freed = 3
```

---

## 성능 고려사항

### 시간 복잡도

- **Mark Phase**: O(N + E)
  N = 객체 수, E = 참조 엣지 수

- **Sweep Phase**: O(N)
  모든 객체 순회

- **Total**: O(N + E)
  객체 그래프 크기에 비례

### 메모리 오버헤드

- **Object 베이스 클래스**: ~16 bytes/객체
  - marked_: 1 byte (+ 패딩)
  - id_: 8 bytes
  - vtable pointer: 8 bytes

- **GC 추적 구조체**: ~24 bytes/객체
  - set 노드 오버헤드

- **Total**: ~40 bytes/객체 오버헤드

### 최적화 전략

1. **증분 GC (Incremental GC)**: 대기 시간 감소
2. **세대별 GC (Generational GC)**: 젊은 객체 우선 수집
3. **동시 GC (Concurrent GC)**: 백그라운드 실행
4. **압축 (Compaction)**: 메모리 단편화 해결

---

## 제한사항 및 향후 개선

### 현재 제한사항

1. **Stop-the-World**: GC 중 프로그램 일시 정지
2. **단순 Mark & Sweep**: 세대별/증분 GC 없음
3. **수동 통합**: Object 상속 필요
4. **메모리 단편화**: 압축 기능 없음

### 향후 개선 계획

#### Phase 3.1: 증분 GC
- Mark/Sweep를 여러 단계로 분할
- 각 단계 사이에 프로그램 실행
- 대기 시간 감소

#### Phase 3.2: 세대별 GC
- Young/Old 세대 분리
- 젊은 객체 우선 수집
- 처리량 향상

#### Phase 3.3: 동시 GC
- 별도 스레드에서 GC 실행
- 프로그램 중단 최소화
- Read/Write 배리어 필요

#### Phase 3.4: Value 통합
- Value를 Object 기반으로 재구현
- shared_ptr 제거
- 완전한 GC 관리

---

## 테스트

### 테스트 커버리지

- ✅ 객체 등록/해제
- ✅ 루트 관리
- ✅ Mark & Sweep 알고리즘
- ✅ 순환 참조 처리
- ✅ Environment 추적
- ✅ 통계 수집
- ✅ 자동 GC 트리거
- ✅ 복잡한 객체 그래프

### 실행

```bash
cd build
ctest -R GCTest -V
```

---

## 참고 자료

### 논문 및 책

1. **"Garbage Collection: Algorithms for Automatic Dynamic Memory Management"**
   - Richard Jones, Rafael Lins (1996)

2. **"The Garbage Collection Handbook"**
   - Richard Jones, Antony Hosking, Eliot Moss (2011)

### 구현 참조

1. **Java HotSpot GC**: Generational + CMS/G1
2. **V8 (JavaScript)**: Generational + Incremental
3. **Go GC**: Concurrent Mark & Sweep
4. **CPython**: Reference Counting + Cycle Detection

---

## 라이선스

MIT License - 자유롭게 사용, 수정, 배포 가능

---

**작성자**: KingSejong Team
**버전**: 0.1.0
**최종 수정**: 2025-11-10
