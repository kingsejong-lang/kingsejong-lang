# 타 인터프리터 비교 분석

**문서**: ANALYSIS_COMPARISON.md
**작성일**: 2025-11-15
**비교 대상**: CPython, V8 (JavaScript), Ruby MRI

---

## 목차

1. [비교 개요](#1-비교-개요)
2. [CPython과의 비교](#2-cpython과의-비교)
3. [V8과의 비교](#3-v8과의-비교)
4. [Ruby MRI와의 비교](#4-ruby-mri와의-비교)
5. [격차 분석](#5-격차-분석)
6. [벤치마킹](#6-벤치마킹)

---

## 1. 비교 개요

### 1.1 비교 언어 선정 기준

| 인터프리터 | 선정 이유 |
|-----------|-----------|
| **CPython** | 가장 성숙한 인터프리터, 교육 친화적 |
| **V8** | 최고 성능의 JIT 엔진, 현대적 최적화 |
| **Ruby MRI** | 킹세종과 가장 유사한 철학 (표현력 > 성능) |

### 1.2 비교 항목

1. **Lexer/Parser 전략**
2. **AST 설계**
3. **실행 엔진** (Interpreter, Bytecode VM, JIT)
4. **메모리 관리** (GC 전략)
5. **최적화 수준**
6. **에러 처리**
7. **성능**

---

## 2. CPython과의 비교

### 2.1 기본 정보

| 항목 | CPython | 킹세종 |
|------|---------|--------|
| **언어** | Python | 킹세종 (한국어) |
| **첫 릴리스** | 1991 | 2025 |
| **주 언어** | C | C++23 |
| **코드 규모** | ~700,000줄 | ~17,000줄 |
| **철학** | Simple > Complex | 한국어 자연성 > 성능 |

### 2.2 Lexer/Parser

| 측면 | CPython | 킹세종 | 평가 |
|------|---------|--------|------|
| **Lexer** | PEG (Parsing Expression Grammar) | 수작업 토큰화 | 🟡 CPython 더 견고 |
| **Parser** | PEG (LL(∞) 파싱) | Pratt (TDOP, LL(1)) | 🟢 거의 동등 |
| **문법** | 명확 (들여쓰기 기반) | 모호성 있음 (조사 "가/이") | 🔴 CPython 우수 |
| **위치 정보** | 완비 (line, column, offset) | 없음 | 🔴 CPython 우수 |
| **에러 복구** | 있음 (Python 3.10+) | 없음 | 🔴 CPython 우수 |

**핵심 차이**:

**CPython PEG**:
```python
# PEG 문법 예시 (Python 3.10+)
if_stmt:
    | 'if' named_expression ':' block elif_stmt
    | 'if' named_expression ':' block [else_block]
```
- Unlimited lookahead
- 백트래킹 지원
- 모호성 자동 해결

**킹세종 Pratt**:
- 1-token lookahead
- 백트래킹 없음 (효율적)
- 모호성 휴리스틱으로 해결

**결론**: CPython이 더 견고하지만, 킹세종의 Pratt는 효율적이고 확장 가능

### 2.3 AST 설계

| 측면 | CPython | 킹세종 | 평가 |
|------|---------|--------|------|
| **노드 계층** | 명확 (Expr/Stmt/Mod) | 명확 (Expression/Statement) | 🟢 동등 |
| **타입 주석** | 있음 (Optional[Type]) | 없음 | 🔴 CPython 우수 |
| **위치 정보** | 완비 (lineno, col_offset, end_*) | 없음 | 🔴 CPython 우수 |
| **메타데이터** | Symbol Table 연결 | 없음 | 🔴 CPython 우수 |

**CPython AST 예시**:
```python
# Python 코드
x = 1 + 2

# AST (간략화)
Module(
    body=[
        Assign(
            targets=[Name(id='x', ctx=Store(), lineno=1, col_offset=0)],
            value=BinOp(
                left=Constant(value=1, lineno=1, col_offset=4),
                op=Add(),
                right=Constant(value=2, lineno=1, col_offset=8),
                lineno=1, col_offset=4
            ),
            lineno=1, col_offset=0
        )
    ]
)
```

**결론**: CPython AST가 훨씬 풍부한 정보 제공

### 2.4 실행 엔진

| 측면 | CPython | 킹세종 | 평가 |
|------|---------|--------|------|
| **방식** | Bytecode VM (Stack-based) | Dual (Evaluator + Bytecode VM) | 🟡 유사 |
| **JIT** | 없음 (3.11+ 간단한 인라인) | 없음 | 🟢 동등 |
| **최적화** | Peephole, Constant Folding | Constant Folding (부분) | 🟡 CPython 우수 |

**CPython Bytecode 예시**:
```python
# Python 코드
def f(a, b):
    return a + b

# Bytecode
  0 LOAD_FAST                0 (a)
  2 LOAD_FAST                1 (b)
  4 BINARY_ADD
  6 RETURN_VALUE
```

**킹세종 Bytecode 예시** (추정):
```
함수 f(정수 a, 정수 b) {
    반환한다(a + b)
}

// Bytecode
LOAD_VAR a
LOAD_VAR b
ADD
RETURN
```

**결론**: 구조는 유사, CPython이 더 성숙

### 2.5 메모리 관리 (GC)

| 측면 | CPython | 킹세종 | 평가 |
|------|---------|--------|------|
| **주 전략** | Reference Counting | C++ RAII (스마트 포인터) | 🟡 다름 |
| **순환 참조** | Cycle Detector (Mark-and-Sweep) | 없음 (누수 가능) | 🔴 CPython 우수 |
| **성능** | 즉시 해제 (RC), STW (Cycle) | 자동 (RAII) | 🟡 각각 장단점 |

**CPython GC**:
```python
import gc

# Reference Counting
x = [1, 2, 3]  # refcount = 1
y = x          # refcount = 2
del x          # refcount = 1
del y          # refcount = 0 → 즉시 해제

# Cycle Detection
a = []
b = []
a.append(b)
b.append(a)  # 순환 참조
del a, del b  # refcount > 0이지만 도달 불가 → GC가 나중에 해제
gc.collect()  # 수동 트리거 가능
```

**결론**: CPython GC가 훨씬 견고

### 2.6 에러 처리

| 측면 | CPython | 킹세종 | 평가 |
|------|---------|--------|------|
| **위치 정보** | line, column, filename | 없음 | 🔴 CPython 우수 |
| **스택 트레이스** | 완전한 traceback | 기본적 에러 | 🔴 CPython 우수 |
| **제안** | "Did you mean?" (3.10+) | 없음 | 🔴 CPython 우수 |

**CPython 에러 예시**:
```python
# Python 3.10+
name = "Python"
print(nam)

# 에러 출력
Traceback (most recent call last):
  File "test.py", line 2, in <module>
    print(nam)
NameError: name 'nam' is not defined. Did you mean: 'name'?
```

**결론**: CPython 에러 처리가 월등

### 2.7 성능 비교

**벤치마크** (피보나치 30):
```
CPython:       ~2.5초
킹세종 Eval:   ~5.0초 (예상)
킹세종 VM:     ~2.0초 (예상)

비율: 킹세종 VM이 CPython의 80% 성능 (예상)
```

**결론**: 비슷한 성능대 (둘 다 JIT 없음)

### 2.8 종합 평가

| 카테고리 | CPython | 킹세종 | 격차 |
|----------|---------|--------|------|
| **Lexer/Parser** | 9/10 | 7/10 | 🔴 중간 |
| **AST** | 9/10 | 6/10 | 🔴 큼 |
| **실행 엔진** | 8/10 | 7/10 | 🟡 작음 |
| **GC** | 9/10 | 4/10 | 🔴 큼 |
| **에러 처리** | 10/10 | 4/10 | 🔴 큼 |
| **성능** | 6/10 | 5/10 | 🟡 작음 |
| **전체** | 8.5/10 | 5.5/10 | 🔴 3점 차이 |

---

## 3. V8과의 비교

### 3.1 기본 정보

| 항목 | V8 (Chrome) | 킹세종 |
|------|-------------|--------|
| **언어** | JavaScript | 킹세종 (한국어) |
| **첫 릴리스** | 2008 | 2025 |
| **주 언어** | C++ | C++23 |
| **코드 규모** | ~2,000,000줄 | ~17,000줄 |
| **철학** | 최고 성능 | 한국어 자연성 |

### 3.2 Parser

| 측면 | V8 | 킹세종 | 평가 |
|------|-----|--------|------|
| **전략** | Recursive Descent | Pratt (TDOP) | 🟡 유사 |
| **Lazy Parsing** | 있음 (함수 미리 파싱 안 함) | 없음 | 🔴 V8 우수 |
| **ASI** | 있음 (AST 생성 시) | 있음 (Lexer에서) | 🟡 V8 더 적절 |

**V8 Lazy Parsing**:
```javascript
// 함수 정의만 스캔, body는 나중에 파싱
function outer() {
    function inner() { /* 여기는 아직 파싱 안 함 */ }
    // inner 호출 시 파싱
}
```

**결론**: V8이 훨씬 최적화됨

### 3.3 실행 엔진 (3-Tier JIT)

| 계층 | V8 | 킹세종 | 평가 |
|------|-----|--------|------|
| **Tier 1** | Ignition (Bytecode Interpreter) | Bytecode VM | 🟡 유사 |
| **Tier 2** | Sparkplug (Baseline JIT) | 없음 | 🔴 V8 우수 |
| **Tier 3** | TurboFan (Optimizing JIT) | 없음 | 🔴 V8 우수 |

**V8 3-Tier 구조**:
```
소스 코드
  ↓
Parser → AST
  ↓
Ignition Compiler → Bytecode
  ↓ (인터프리터 실행, 프로파일링)
  ↓ (핫 함수 감지)
  ↓
Sparkplug → 네이티브 코드 (빠른 컴파일, 최적화 적음)
  ↓ (더 많이 호출된 함수)
  ↓
TurboFan → 네이티브 코드 (느린 컴파일, 최적화 많음)
```

**킹세종 구조**:
```
소스 코드
  ↓
Lexer → 토큰
  ↓
Parser → AST
  ↓
Compiler → Bytecode
  ↓
VM → 실행 (끝)
```

**결론**: V8이 압도적으로 빠름 (10-100배)

### 3.4 최적화

| 최적화 | V8 | 킹세종 | 평가 |
|--------|-----|--------|------|
| **Inline Caching** | 있음 | 없음 | 🔴 V8 우수 |
| **Hidden Classes** | 있음 (객체 최적화) | 없음 | 🔴 V8 우수 |
| **Escape Analysis** | 있음 (힙 할당 제거) | 없음 | 🔴 V8 우수 |
| **Loop Unrolling** | 있음 | 없음 | 🔴 V8 우수 |

**Inline Caching 예시**:
```javascript
// JavaScript
function getName(obj) {
    return obj.name;  // 매번 속성 검색?
}

// V8 최적화:
// 첫 호출: obj의 Hidden Class 확인, offset 캐싱
// 이후 호출: Hidden Class 같으면 캐시된 offset 직접 사용
getName({name: "A"})  // Hidden Class 1, offset 0
getName({name: "B"})  // Hidden Class 1 (같음!) → 캐시 사용
```

**결론**: V8 최적화 수준이 비교 불가

### 3.5 GC

| 측면 | V8 | 킹세종 | 평가 |
|------|-----|--------|------|
| **전략** | Generational (Young + Old) | RAII | 🔴 V8 우수 |
| **STW** | Incremental, Concurrent | 없음 (GC 자체가 없음) | 🟡 다름 |
| **성능** | 매우 높음 | 순환 참조 시 누수 | 🔴 V8 우수 |

**V8 Generational GC**:
```
객체 할당
  ↓
Young Generation (Scavenger)
  - 대부분 객체 여기서 죽음 (90%+)
  - 빠른 GC (몇 ms)
  ↓ (살아남은 객체)
  ↓
Old Generation (Mark-Compact)
  - 장수 객체
  - 느린 GC (수십 ms), Incremental
```

**결론**: V8 GC가 월등

### 3.6 성능 비교

**벤치마크** (피보나치 30):
```
V8 (JIT):      ~0.05초
킹세종 VM:     ~2.0초 (예상)

비율: V8이 킹세종의 40배 빠름
```

**결론**: V8은 프로덕션급 성능, 킹세종은 교육/DSL용

### 3.7 종합 평가

| 카테고리 | V8 | 킹세종 | 격차 |
|----------|-----|--------|------|
| **Parser** | 9/10 | 7/10 | 🟡 중간 |
| **실행 엔진** | 10/10 (JIT) | 7/10 (VM만) | 🔴 큼 |
| **최적화** | 10/10 | 5/10 | 🔴 매우 큼 |
| **GC** | 10/10 | 4/10 | 🔴 매우 큼 |
| **성능** | 10/10 | 5/10 | 🔴 매우 큼 |
| **전체** | 9.8/10 | 5.6/10 | 🔴 4.2점 차이 |

**결론**: V8은 세계 최고 수준, 킹세종과는 목표가 다름

---

## 4. Ruby MRI와의 비교

### 4.1 기본 정보

| 항목 | Ruby MRI | 킹세종 |
|------|----------|--------|
| **언어** | Ruby | 킹세종 (한국어) |
| **첫 릴리스** | 1995 | 2025 |
| **주 언어** | C | C++23 |
| **코드 규모** | ~500,000줄 | ~17,000줄 |
| **철학** | Programmer Happiness | 한국어 자연성 |

### 4.2 유사점 (가장 유사한 인터프리터!)

| 측면 | 유사성 |
|------|--------|
| **표현력 > 성능** | ✅ 둘 다 성능보다 개발자 경험 우선 |
| **Stack-based VM** | ✅ YARV와 킹세종 VM 모두 스택 기반 |
| **DSL 친화적** | ✅ Ruby (Rails DSL), 킹세종 (조사 기반 DSL) |
| **JIT 부재 (초기)** | ✅ Ruby 2.5까지 JIT 없음, 킹세종도 없음 |

### 4.3 Parser

| 측면 | Ruby MRI | 킹세종 | 평가 |
|------|----------|--------|------|
| **전략** | Bison (LALR) | Pratt (TDOP) | 🟡 다름 |
| **복잡도** | 매우 복잡 | 중간 | 🟢 킹세종 단순 |
| **확장성** | 낮음 | 높음 | 🟢 킹세종 우수 |

**결론**: 킹세종의 Pratt가 더 유지보수 쉬움

### 4.4 실행 엔진

| 측면 | Ruby MRI | 킹세종 | 평가 |
|------|----------|--------|------|
| **방식** | YARV (Bytecode VM) | Bytecode VM | 🟢 거의 동일! |
| **JIT** | MJIT (Ruby 2.6+) | 없음 | 🟡 Ruby 우수 |
| **최적화** | 기본적 | 기본적 | 🟢 동등 |

**YARV (Yet Another Ruby VM)**:
```ruby
# Ruby 코드
def f(a, b)
  a + b
end

# YARV Bytecode
== disasm: <RubyVM::InstructionSequence:f@test.rb>
0000 getlocal_OP__WC__0                          a
0002 getlocal_OP__WC__0                          b
0004 opt_plus
0005 leave
```

**킹세종 Bytecode** (유사):
```
함수 f(정수 a, 정수 b) {
    반환한다(a + b)
}

// Bytecode
LOAD_VAR a
LOAD_VAR b
ADD
RETURN
```

**결론**: 거의 동일한 구조!

### 4.5 GC

| 측면 | Ruby MRI | 킹세종 | 평가 |
|------|----------|--------|------|
| **전략** | Generational (Ruby 2.1+) | RAII | 🔴 Ruby 우수 |
| **STW** | Incremental (Ruby 2.2+) | 없음 | 🔴 Ruby 우수 |

### 4.6 철학 비교

| 가치 | Ruby | 킹세종 | 평가 |
|------|------|--------|------|
| **Matz's Principle** | "Ruby is designed to make programmers happy" | "한국어로 자연스럽게" | 🟢 동일한 철학 |
| **DSL** | Rails, RSpec, Rake | 조사 기반 DSL | 🟢 동일한 방향 |
| **성능 철학** | "충분히 빠르면 됨" | (추정) 동일 | 🟢 동일 |

**Ruby DSL 예시**:
```ruby
# RSpec (테스트 DSL)
describe "Array" do
  it "should be empty" do
    expect([]).to be_empty
  end
end
```

**킹세종 DSL 예시**:
```javascript
배열을 필터링한다(조건)를 매핑한다(변환)를 출력한다()
```

**결론**: 철학적으로 가장 유사!

### 4.7 성능 비교

**벤치마크** (피보나치 30):
```
Ruby MRI 3.0:  ~3.0초 (JIT 없이)
Ruby MJIT 3.0: ~1.5초 (JIT 사용)
킹세종 VM:     ~2.0초 (예상)

비율: 킹세종이 Ruby MRI와 유사 (~67% 성능)
```

**결론**: 비슷한 성능대

### 4.8 종합 평가

| 카테고리 | Ruby MRI | 킹세종 | 격차 |
|----------|----------|--------|------|
| **Parser** | 7/10 | 7/10 | 🟢 동등 |
| **실행 엔진** | 8/10 | 7/10 | 🟡 작음 |
| **GC** | 9/10 | 4/10 | 🔴 큼 |
| **철학** | 10/10 (표현력) | 10/10 (한국어) | 🟢 동등 |
| **성능** | 6/10 | 5/10 | 🟡 작음 |
| **전체** | 8.0/10 | 6.6/10 | 🟡 1.4점 차이 |

**결론**: Ruby MRI와 가장 유사, 격차도 가장 작음

---

## 5. 격차 분석

### 5.1 전체 비교 요약

| 측면 | CPython | V8 | Ruby MRI | 킹세종 |
|------|---------|-----|----------|--------|
| **코드 규모** | 700K | 2M | 500K | 17K |
| **개발 연수** | 34년 | 17년 | 30년 | 0.3년 |
| **Lexer** | 9 | 9 | 7 | 7 |
| **Parser** | 9 | 9 | 7 | 7 |
| **AST** | 9 | 9 | 8 | 6 |
| **Semantic Analysis** | 9 | 10 | 8 | 2 |
| **실행 엔진** | 8 | 10 | 8 | 7 |
| **GC** | 9 | 10 | 9 | 4 |
| **최적화** | 7 | 10 | 6 | 5 |
| **에러 처리** | 10 | 9 | 8 | 4 |
| **성능** | 6 | 10 | 6 | 5 |
| **전체** | 8.5 | 9.5 | 7.6 | 5.4 |

### 5.2 격차 분석

#### 가장 큰 격차

1. **Semantic Analysis**: CPython/V8 10점, 킹세종 2점 (**8점 차이**)
2. **GC**: V8 10점, 킹세종 4점 (**6점 차이**)
3. **에러 처리**: CPython 10점, 킹세종 4점 (**6점 차이**)

#### 작은 격차

1. **Parser**: CPython/V8 9점, 킹세종 7점 (**2점 차이**)
2. **실행 엔진**: V8 10점, 킹세종 7점 (**3점 차이**)
3. **성능**: V8 10점, 킹세종 5점 (**5점 차이**, 하지만 목표 다름)

### 5.3 우선순위 개선 영역

**기준**: 격차가 크고 + 중요도가 높은 영역

| 순위 | 영역 | 격차 | 중요도 | 우선순위 |
|------|------|------|--------|----------|
| 1 | **Semantic Analysis** | 8점 | 높음 | 🔴 최우선 |
| 2 | **에러 처리** | 6점 | 높음 | 🔴 긴급 |
| 3 | **GC** | 6점 | 중간 | 🟡 중요 |
| 4 | **AST 메타데이터** | 3점 | 높음 | 🟡 중요 |
| 5 | **최적화** | 5점 | 낮음 | 🟢 개선 |

---

## 6. 벤치마킹

### 6.1 피보나치 벤치마크

**코드**:
```
function fib(n) {
    if (n <= 1) return n
    return fib(n - 1) + fib(n - 2)
}
fib(30)
```

| 인터프리터 | 시간 (초) | 상대 속도 |
|-----------|-----------|-----------|
| **V8** | 0.05 | 100x |
| **Ruby MJIT** | 1.5 | 3.3x |
| **킹세종 VM** | 2.0 (예상) | 2.5x |
| **CPython** | 2.5 | 2.0x |
| **킹세종 Eval** | 5.0 (예상) | 1.0x (기준) |

### 6.2 배열 처리 벤치마크

**코드**:
```
arr = [1, 2, ..., 1000000]
sum = 0
for i in arr:
    sum += i
```

| 인터프리터 | 시간 (초) | 상대 속도 |
|-----------|-----------|-----------|
| **V8** | 0.01 | 100x |
| **CPython** | 0.05 | 20x |
| **Ruby MRI** | 0.08 | 12.5x |
| **킹세종 VM** | 0.1 (예상) | 10x |

### 6.3 결론

- **V8**: 압도적 성능 (JIT)
- **CPython/Ruby/킹세종**: 비슷한 성능대 (Bytecode VM)
- **킹세종 목표**: Ruby MRI 수준 달성 가능

---

## 7. 최종 결론

### 7.1 핵심 발견

1. **킹세종 ≈ Ruby MRI** (철학, 구조, 성능 모두 유사)
2. **격차 최대**: Semantic Analysis, GC, 에러 처리
3. **격차 최소**: Parser, 실행 엔진
4. **성능**: 교육/DSL용으로는 충분, 프로덕션용으로는 JIT 필요

### 7.2 개선 방향

**단기 (Ruby MRI 수준 목표)**:
1. Semantic Analyzer 추가
2. GC 구현
3. 에러 처리 개선

**장기 (CPython 수준 목표)**:
4. JIT 컴파일러
5. 고급 최적화

**초장기 (V8은 목표 아님)**:
- V8 수준은 수십 명 팀, 수년 필요
- 킹세종 철학과 맞지 않음 (표현력 > 성능)

---

**문서 끝**

**다음 문서**: [ANALYSIS_BUGS.md](./ANALYSIS_BUGS.md)
**이전 문서**: [ANALYSIS_AST_EXECUTION.md](./ANALYSIS_AST_EXECUTION.md)
