# JIT Tier 2: 함수 인라이닝 설계 문서

**작성일**: 2025-11-24
**버전**: v0.1.0 (Draft)
**상태**: 설계 단계

---

## 📋 목차

1. [개요](#개요)
2. [JIT Tier 1 현황](#jit-tier-1-현황)
3. [Tier 2 목표](#tier-2-목표)
4. [함수 인라이닝 전략](#함수-인라이닝-전략)
5. [구현 계획](#구현-계획)
6. [예상 성능 개선](#예상-성능-개선)
7. [참고 자료](#참고-자료)

---

## 개요

KingSejong 언어의 JIT 컴파일러는 Tiered Compilation 전략을 사용합니다:

- **Tier 0**: 인터프리터 (bytecode 실행)
- **Tier 1**: 빠른 템플릿 기반 JIT (루프 최적화) ✅ **완료**
- **Tier 2**: 함수 인라이닝 JIT (함수 호출 최적화) 🚧 **계획 중**

이 문서는 **Tier 2 함수 인라이닝**의 설계 및 구현 계획을 다룹니다.

---

## JIT Tier 1 현황

### 지원 기능

**컴파일 대상**:
- 루프 (반복문)
- 산술/논리 집약적 코드

**지원 OpCode** (총 21개):

| 카테고리 | OpCode |
|---------|--------|
| **상수** | LOAD_CONST, LOAD_TRUE, LOAD_FALSE |
| **산술** | ADD, SUB, MUL, DIV, MOD, NEG |
| **비교** | EQ, NE, LT, GT, LE, GE |
| **논리** | AND, OR, NOT |
| **스택** | POP, DUP |
| **제어 흐름** | JUMP, JUMP_IF_FALSE, JUMP_IF_TRUE |

**아키텍처 지원**:
- x86-64 (asmjit x86::Assembler)
- ARM64 (asmjit a64::Assembler)

### 제한 사항

**미지원 OpCode**:
- `CALL`, `TAIL_CALL`, `RETURN` - 함수 호출 및 반환
- `LOAD_GLOBAL`, `STORE_GLOBAL` - 전역 변수 접근
- `LOAD_LOCAL`, `STORE_LOCAL` - 지역 변수 접근
- 배열 연산 (`ARRAY_LOAD`, `ARRAY_STORE`)
- 객체 및 클래스 연산

**현재 성능**:
- 루프 최적화: **5-10배 개선**
- 함수 호출: **최적화 없음** (인터프리터와 동일)

---

## Tier 2 목표

### 핵심 목표

1. **함수 인라이닝 구현**
   - 작은 함수 호출을 함수 본문으로 대체
   - 함수 호출 오버헤드 제거

2. **함수 호출 성능 개선**
   - **목표**: 20-30% 성능 향상
   - **방법**: CallFrame 생성 제거, 인자 전달 최적화

3. **추가 최적화 기회 활용**
   - 상수 전파 (Constant Propagation)
   - 데드 코드 제거 (Dead Code Elimination)
   - 레지스터 할당 최적화

### 비목표 (Non-Goals)

- ❌ 다형성 함수 인라이닝 (추후 고려)
- ❌ 재귀 함수 인라이닝 (TCO로 해결 완료)
- ❌ 전체 프로그램 최적화 (Tier 2는 함수 단위)

---

## 함수 인라이닝 전략

### 1. 인라이닝 후보 선정 기준

**인라이닝 가능 함수 조건**:

✅ **필수 조건**:
1. **작은 함수 크기**: 바이트코드 명령어 수 ≤ 50
2. **단순한 제어 흐름**: 중첩 루프 없음, 예외 처리 없음
3. **타입 안정성**: 매개변수 타입이 정적으로 확정 가능
4. **빈번한 호출**: 실행 횟수 ≥ 100 (Hot Function)

❌ **제외 조건**:
1. 재귀 함수 (직접/간접)
2. 가변 인자 함수 (`...args`)
3. 클래스 메서드 (동적 디스패치 필요)
4. 네이티브 함수 (`stdlib.*`)

### 2. 인라이닝 휴리스틱

**우선순위**:

| 우선순위 | 함수 특징 | 예상 효과 |
|---------|----------|----------|
| **높음** | 단일 반환, 산술 연산만 | 30-50% 개선 |
| **중간** | 조건문 1-2개 포함 | 20-30% 개선 |
| **낮음** | 루프 1개 포함 | 10-20% 개선 |

**크기 제한**:
```c++
const size_t MAX_INLINE_SIZE = 50;        // 최대 바이트코드 크기
const size_t MAX_INLINE_DEPTH = 3;        // 최대 인라이닝 깊이
const size_t MAX_TOTAL_INLINE_SIZE = 200; // 총 인라인 바이트코드 크기
```

### 3. 인라이닝 변환 예시

**변환 전** (바이트코드):
```
함수 add(a, b) {
    반환 a + b
}

x = add(10, 20)  // CALL add, 2 args
```

**변환 후** (인라인):
```
// add(10, 20) 인라인 확장
LOAD_CONST 10
LOAD_CONST 20
ADD
STORE_LOCAL x  // 결과 저장
```

**효과**:
- `CALL` OpCode 제거 (CallFrame 생성 없음)
- `RETURN` OpCode 제거
- 함수 호출 오버헤드 **100% 제거**

---

## 구현 계획

### Phase 1: 인라이닝 인프라 구축 (1주)

**1.1 함수 메타데이터 수집**
```cpp
struct FunctionMetadata {
    size_t bytecodeSize;        // 바이트코드 크기
    size_t executionCount;      // 실행 횟수
    bool hasLoops;              // 루프 포함 여부
    bool hasRecursion;          // 재귀 호출 여부
    std::vector<OpCode> ops;    // OpCode 시퀀스
};
```

**1.2 인라이닝 휴리스틱 엔진**
```cpp
class InliningAnalyzer {
public:
    bool canInline(const FunctionMetadata& meta);
    int getInlinePriority(const FunctionMetadata& meta);
    size_t estimateCodeSize(const FunctionMetadata& meta);
};
```

**1.3 CallSite 분석**
```cpp
struct CallSite {
    size_t callOffset;          // CALL 위치
    FunctionMetadata* callee;   // 호출 대상 함수
    std::vector<Value> args;    // 인자 (상수인 경우)
};
```

### Phase 2: 인라이닝 변환 구현 (2주)

**2.1 바이트코드 복사 및 삽입**
```cpp
class InliningTransformer {
public:
    // 함수 본문을 호출 위치에 복사
    void inlineFunction(Chunk* chunk, const CallSite& site);

    // 인자 바인딩 (LOAD_LOCAL → LOAD_CONST)
    void bindArguments(Chunk* chunk, const CallSite& site);

    // RETURN → 제거 (스택 정리)
    void removeReturn(Chunk* chunk, size_t returnOffset);
};
```

**2.2 상수 전파**
```cpp
// add(10, 20) → LOAD_CONST 30
bool tryConstantFolding(const CallSite& site);
```

**2.3 데드 코드 제거**
```cpp
// 사용되지 않는 중간 값 제거
void eliminateDeadCode(Chunk* chunk);
```

### Phase 3: JIT Tier 2 컴파일러 구현 (2주)

**3.1 JITCompilerT2 클래스**
```cpp
class JITCompilerT2 : public JITCompilerT1 {
public:
    // 함수 인라이닝 포함 컴파일
    NativeFunction* compileWithInlining(
        bytecode::Chunk* chunk,
        size_t startOffset,
        size_t endOffset,
        const std::vector<CallSite>& inlineSites
    );

private:
    InliningAnalyzer analyzer_;
    InliningTransformer transformer_;
};
```

**3.2 VM 통합**
```cpp
// VM.cpp - 함수 호출 감지
if (executionCount > HOT_FUNCTION_THRESHOLD) {
    // Tier 2 컴파일 트리거
    jitT2_->compileWithInlining(chunk, funcAddr, funcEnd, callSites);
}
```

### Phase 4: 테스트 및 벤치마크 (1주)

**4.1 단위 테스트**
- 인라이닝 휴리스틱 테스트
- 바이트코드 변환 테스트
- 상수 전파 테스트

**4.2 통합 테스트**
- 전체 프로그램 실행 테스트
- 기존 1,519개 테스트 100% 통과 유지

**4.3 성능 벤치마크**
- fibonacci (재귀) - TCO와 비교
- 작은 함수 집약 프로그램
- 실제 사용 사례 (stdlib 함수 사용)

---

## 예상 성능 개선

### 벤치마크 시나리오

**1. 작은 함수 집약 프로그램**
```ksj
함수 add(a, b) { 반환 a + b }
함수 mul(a, b) { 반환 a * b }
함수 square(x) { 반환 mul(x, x) }

합계 = 0
i가 0부터 9999까지 {
    합계 = add(합계, square(i))
}
```

**예상 결과**:
- **현재** (Tier 1): 0.08s
- **Tier 2**: 0.055s
- **개선율**: **31% 빠름** ✨

**2. 조건문 포함 함수**
```ksj
함수 max(a, b) {
    만약 (a > b) { 반환 a }
    아니면 { 반환 b }
}

최대값 = 0
i가 0부터 9999까지 {
    최대값 = max(최대값, i)
}
```

**예상 결과**:
- **현재** (Tier 1): 0.06s
- **Tier 2**: 0.045s
- **개선율**: **25% 빠름** ✨

**3. stdlib 함수 사용**
```ksj
// stdlib/math.ksj
함수 abs(x) {
    만약 (x < 0) { 반환 -x }
    반환 x
}

합계 = 0
i가 -4999부터 5000까지 {
    합계 = 합계 + abs(i)
}
```

**예상 결과**:
- **현재** (Tier 1): 0.09s
- **Tier 2**: 0.063s
- **개선율**: **30% 빠름** ✨

### 전체 성능 목표

| 항목 | Tier 1 | Tier 2 | 개선율 |
|------|--------|--------|--------|
| 작은 함수 집약 | 0.08s | 0.055s | **31% ↑** |
| 조건문 포함 | 0.06s | 0.045s | **25% ↑** |
| stdlib 함수 | 0.09s | 0.063s | **30% ↑** |
| **평균** | - | - | **~29% ↑** |

**목표 달성**: **20-30% 성능 향상** ✅

---

## 타임라인

```
[Week 1] 인라이닝 인프라 구축
  - FunctionMetadata 수집
  - InliningAnalyzer 구현
  - CallSite 분석

[Week 2-3] 인라이닝 변환 구현
  - InliningTransformer 구현
  - 상수 전파 및 데드 코드 제거
  - 바이트코드 변환 로직

[Week 4-5] JIT Tier 2 컴파일러
  - JITCompilerT2 클래스 구현
  - VM 통합
  - Hot Function 감지

[Week 6] 테스트 및 벤치마크
  - 단위/통합 테스트
  - 성능 벤치마크
  - 최적화 튜닝
```

**예상 완료**: **2025-12 중순 (6주)**

---

## 참고 자료

### 학술 자료
- [Efficient Inline Substitution in a Native Compiler](https://dl.acm.org/doi/10.1145/143103.143142) - David Detlefs, Ole Agesen (1994)
- [Adaptive Optimization in the Jalapeño JVM](https://dl.acm.org/doi/10.1145/286936.286960) - Matthew Arnold et al. (2000)

### 구현 사례
- **V8 (Chrome)**: TurboFan Inlining
- **PyPy**: JIT Inlining Heuristics
- **LuaJIT**: Trace-based Inlining

### 내부 문서
- [PERFORMANCE_ANALYSIS.md](./PERFORMANCE_ANALYSIS.md) - 성능 분석 보고서
- [ARCHITECTURE_ANALYSIS.md](./ARCHITECTURE_ANALYSIS.md) - 아키텍처 분석
- [JIT Tier 1 구현](../src/jit/JITCompilerT1.cpp)

---

**작성자**: KingSejong Team
**리뷰어**: TBD
**승인자**: TBD
