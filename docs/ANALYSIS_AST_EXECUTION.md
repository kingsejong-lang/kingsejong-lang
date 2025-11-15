# AST 및 실행 엔진 상세 분석

**문서**: ANALYSIS_AST_EXECUTION.md
**작성일**: 2025-11-15
**대상 파일**: `src/ast/`, `src/evaluator/`, `src/bytecode/`

---

## 목차

1. [AST 설계 분석](#1-ast-설계-분석)
2. [Evaluator (Tree-Walking)](#2-evaluator-tree-walking)
3. [Bytecode VM](#3-bytecode-vm)
4. [메모리 관리 및 GC](#4-메모리-관리-및-gc)
5. [성능 비교](#5-성능-비교)
6. [개선 방안](#6-개선-방안)

---

## 1. AST 설계 분석

### 1.1 노드 계층 구조

```
Node (추상 기본 클래스)
├── Expression (표현식 - 값을 반환)
│   ├── Literal (리터럴)
│   │   ├── IntegerLiteral
│   │   ├── FloatLiteral
│   │   ├── StringLiteral
│   │   └── BooleanLiteral
│   ├── Identifier (식별자)
│   ├── BinaryExpression (이항 연산)
│   ├── UnaryExpression (단항 연산)
│   ├── JosaExpression (조사 표현식 - 한국어 특화!)
│   ├── RangeExpression (범위 표현식 - 한국어 특화!)
│   ├── CallExpression (함수 호출)
│   ├── ArrayLiteral (배열 리터럴)
│   ├── IndexExpression (인덱싱)
│   ├── IfExpression (if 표현식)
│   ├── FunctionExpression (함수 리터럴)
│   └── MatchExpression (패턴 매칭 - F5.5)
│
└── Statement (문장 - 부수 효과)
    ├── VarDeclaration (변수 선언)
    ├── AssignmentStatement (할당)
    ├── ExpressionStatement (표현식 문장)
    ├── BlockStatement (블록 { })
    ├── ReturnStatement (반환)
    ├── IfStatement (if 문)
    ├── RepeatStatement (반복 N번)
    ├── RangeForStatement (범위 for문)
    ├── FunctionDeclaration (함수 선언)
    ├── ImportStatement (모듈 가져오기)
    └── ExportStatement (모듈 내보내기)
```

### 1.2 한국어 특화 노드

#### JosaExpression

**목적**: 조사 기반 메서드 체이닝

**예시**:
```javascript
배열을 정렬한다()를 역순으로한다()

// AST:
JosaExpression {
    object: JosaExpression {
        object: Identifier("배열")
        josa: "을"
        method: CallExpression("정렬한다", [])
    }
    josa: "를"
    method: CallExpression("역순으로한다", [])
}
```

**평가**:
- ✅ 혁신적 설계
- ✅ 자연스러운 한국어 문법
- ✅ 메서드 체이닝 지원

#### RangeExpression

**목적**: 한국어 범위 표현

**예시**:
```javascript
1부터 10까지

// AST:
RangeExpression {
    start: IntegerLiteral(1)
    end: IntegerLiteral(10)
}
```

**평가**:
- ✅ 직관적
- ✅ Python의 `range(1, 11)`보다 자연스러움

#### MatchExpression (F5.5 - 패턴 매칭)

**목적**: 패턴 매칭

**예시**:
```javascript
결과 = 값 에 대해 {
    1 -> "하나"
    [첫번째, ...] -> 첫번째
    n 만약 n > 10 -> "큰 수"
    _ -> "기타"
}

// AST:
MatchExpression {
    value: Identifier("값")
    cases: [
        MatchCase {
            pattern: LiteralPattern(1)
            guard: null
            body: StringLiteral("하나")
        },
        MatchCase {
            pattern: ArrayPattern([BindingPattern("첫번째"), RestPattern])
            guard: null
            body: Identifier("첫번째")
        },
        MatchCase {
            pattern: BindingPattern("n")
            guard: BinaryExpression(Identifier("n"), GT, IntegerLiteral(10))
            body: StringLiteral("큰 수")
        },
        MatchCase {
            pattern: WildcardPattern
            guard: null
            body: StringLiteral("기타")
        }
    ]
}
```

**평가**:
- ✅ Rust/Scala 스타일 패턴 매칭
- ✅ 가드 조건 지원
- ✅ 현대적 기능

### 1.3 AST 설계 평가

#### 강점

1. **명확한 계층 구조** ⭐⭐⭐⭐⭐
   - Expression vs Statement 구분 명확
   - 다형성 활용 (`virtual` 메서드)

2. **Immutable 설계** ⭐⭐⭐⭐
   - `const` 접근자
   - `std::unique_ptr`로 소유권 명확

3. **한국어 특화 노드** ⭐⭐⭐⭐⭐
   - JosaExpression, RangeExpression
   - 혁신적이고 자연스러움

#### 약점

1. **타입 정보 부족** ⚠️⚠️⚠️
   - AST 노드에 타입 주석 없음
   - Semantic Analyzer 단계에서 추가 필요

2. **위치 정보 없음** ⚠️⚠️⚠️
   - line, column 정보 없음
   - 에러 메시지 개선 불가

3. **메타데이터 부족** ⚠️⚠️
   - Symbol Table 연결 없음
   - 스코프 정보 없음

---

## 2. Evaluator (Tree-Walking)

### 2.1 동작 원리

Tree-Walking Evaluator는 AST를 직접 순회하며 실행합니다:

```
AST
  ↓
재귀적으로 노드 방문
  ↓
각 노드 타입에 따라 처리
  ↓
결과 값 반환
```

### 2.2 핵심 구조

```cpp
class Evaluator {
private:
    Environment* env_;  // 변수 환경

public:
    Value* eval(Node* node) {
        switch (node->type()) {
            case NodeType::INTEGER_LITERAL:
                return evalIntegerLiteral(node);

            case NodeType::BINARY_EXPRESSION:
                return evalBinaryExpression(node);

            case NodeType::IF_EXPRESSION:
                return evalIfExpression(node);

            case NodeType::MATCH_EXPRESSION:
                return evalMatchExpression(node);

            // ...
        }
    }

private:
    Value* evalBinaryExpression(BinaryExpression* expr) {
        Value* left = eval(expr->left());   // 재귀 호출
        Value* right = eval(expr->right()); // 재귀 호출

        if (expr->op() == "+") {
            return left->value() + right->value();
        }
        // ...
    }
};
```

### 2.3 Environment (환경)

**목적**: 변수 스코프 관리

```cpp
class Environment {
private:
    std::unordered_map<std::string, Value*> store_;
    Environment* outer_;  // 부모 스코프

public:
    Value* get(const std::string& name) {
        if (store_.count(name)) {
            return store_[name];
        }
        if (outer_) {
            return outer_->get(name);  // 부모 스코프에서 찾기
        }
        return nullptr;  // 변수 없음
    }

    void set(const std::string& name, Value* value) {
        store_[name] = value;
    }
};
```

**스코프 체인**:
```javascript
정수 a = 5

함수 외부함수() {
    정수 b = 3

    함수 내부함수() {
        정수 c = 1
        출력한다(a + b + c)  // 모든 스코프 접근 가능
    }

    내부함수()
}

// Environment 체인:
// [내부함수 env: {c: 1}]
//   → [외부함수 env: {b: 3}]
//     → [전역 env: {a: 5, 외부함수: ...}]
```

### 2.4 평가

#### 강점

1. **구현 간단** ⭐⭐⭐⭐⭐
   - 직관적인 재귀 구조
   - 디버깅 용이

2. **유연성** ⭐⭐⭐⭐
   - 동적 타이핑 쉽게 지원
   - 새 기능 추가 용이

3. **교육 친화적** ⭐⭐⭐⭐⭐
   - 코드 이해하기 쉬움

#### 약점

1. **성능 낮음** ⚠️⚠️⚠️
   - 매 노드마다 재귀 호출 오버헤드
   - 함수 호출 스택 깊어짐

2. **최적화 제한적** ⚠️⚠️
   - 인라인화 어려움
   - JIT 불가

3. **메모리 사용 많음** ⚠️⚠️
   - AST 전체 메모리에 유지

---

## 3. Bytecode VM

### 3.1 아키텍처

```
AST
  ↓
Compiler (AST → Bytecode)
  ↓
Bytecode (명령어 배열)
  ↓
VM (Stack-based 실행)
  ↓
결과
```

### 3.2 Bytecode 명령어 (추정)

```cpp
enum class Opcode {
    // 상수
    LOAD_CONST,     // 상수 스택에 푸시

    // 변수
    LOAD_VAR,       // 변수 값 로드
    STORE_VAR,      // 변수에 저장

    // 산술
    ADD,            // 덧셈
    SUB,            // 뺄셈
    MUL,            // 곱셈
    DIV,            // 나눗셈

    // 비교
    EQUAL,          // ==
    NOT_EQUAL,      // !=
    LESS_THAN,      // <
    GREATER_THAN,   // >

    // 제어 흐름
    JUMP,           // 무조건 점프
    JUMP_IF_FALSE,  // 조건부 점프
    CALL,           // 함수 호출
    RETURN,         // 함수 반환

    // 배열
    BUILD_ARRAY,    // 배열 생성
    INDEX,          // 인덱싱

    // 기타
    POP,            // 스택에서 제거
    PRINT,          // 출력
};
```

### 3.3 예시: `1 + 2 * 3`

**AST**:
```
    +
   / \
  1   *
     / \
    2   3
```

**Bytecode**:
```
LOAD_CONST 1    // 스택: [1]
LOAD_CONST 2    // 스택: [1, 2]
LOAD_CONST 3    // 스택: [1, 2, 3]
MUL             // 스택: [1, 6]
ADD             // 스택: [7]
```

**VM 실행**:
```cpp
void VM::run() {
    while (ip < bytecode.size()) {
        Opcode op = bytecode[ip++];

        switch (op) {
            case Opcode::LOAD_CONST:
                int index = bytecode[ip++];
                stack.push(constants[index]);
                break;

            case Opcode::ADD:
                Value* right = stack.pop();
                Value* left = stack.pop();
                stack.push(left + right);
                break;

            // ...
        }
    }
}
```

### 3.4 최적화

#### Constant Folding (상수 접기)

**목적**: 컴파일 시점에 상수 계산

**예시**:
```javascript
// 소스 코드
정수 a = 2 * 3 + 5

// 최적화 전 Bytecode:
LOAD_CONST 2
LOAD_CONST 3
MUL
LOAD_CONST 5
ADD
STORE_VAR a

// 최적화 후 Bytecode:
LOAD_CONST 11  // 2 * 3 + 5 = 11 (컴파일 시 계산)
STORE_VAR a
```

**구현** (Compiler.cpp:188-199 참조):
```cpp
bool Compiler::isConstantCondition(Expression* expr, bool* result) {
    if (auto lit = dynamic_cast<BooleanLiteral*>(expr)) {
        *result = lit->value();
        return true;
    }

    if (auto bin = dynamic_cast<BinaryExpression*>(expr)) {
        // 양쪽이 모두 상수이면 계산
        if (isConstant(bin->left()) && isConstant(bin->right())) {
            *result = evaluate(bin);
            return true;
        }
    }

    return false;
}
```

#### Dead Code Elimination (데드 코드 제거)

**목적**: 실행되지 않는 코드 제거

**예시**:
```javascript
// 소스 코드
만약 참이면 {
    출력한다("A")
} 아니면 {
    출력한다("B")  // 절대 실행 안 됨!
}

// 최적화 후 Bytecode:
LOAD_CONST "A"
CALL print
// "아니면" 블록 제거됨
```

### 3.5 평가

#### 강점

1. **성능 향상** ⭐⭐⭐⭐
   - Evaluator보다 2-5배 빠름 (추정)
   - 명령어 디스패치 오버헤드 감소

2. **최적화 가능** ⭐⭐⭐⭐
   - Constant Folding
   - Dead Code Elimination
   - Peephole Optimization 가능

3. **메모리 효율** ⭐⭐⭐⭐
   - Bytecode가 AST보다 작음

4. **이식성** ⭐⭐⭐⭐⭐
   - Bytecode는 플랫폼 독립적
   - 한 번 컴파일, 여러 곳 실행

#### 약점

1. **디버깅 어려움** ⚠️⚠️⚠️
   - Bytecode 레벨 디버깅 복잡
   - 소스 코드와 매핑 필요

2. **JIT 없음** ⚠️⚠️⚠️
   - Just-In-Time 컴파일 미지원
   - 네이티브 코드 생성 불가

3. **구현 복잡도** ⚠️⚠️
   - Evaluator보다 복잡
   - Compiler + VM 두 컴포넌트

---

## 4. 메모리 관리 및 GC

### 4.1 현재 방식: C++ RAII

**현재 구현**:
```cpp
// C++ 스마트 포인터 사용
std::unique_ptr<Expression> expr;
std::shared_ptr<Value> value;

// 자동 메모리 관리 (RAII)
{
    auto expr = std::make_unique<IntegerLiteral>(42);
    // 스코프 끝에서 자동 해제
}
```

**평가**:
- ✅ 간단하고 안전
- ✅ 메모리 누수 적음
- ⚠️ 순환 참조 시 누수 가능 (`shared_ptr` 순환)
- ⚠️ 동적 언어에 부적합 (GC 없음)

### 4.2 필요성: Garbage Collector

**문제 시나리오**:
```javascript
함수 create_cycle() {
    배열 a = []
    배열 b = []
    a를_추가한다(b)
    b를_추가한다(a)  // 순환 참조!
    반환한다(a)
}

// a와 b가 서로 참조 → shared_ptr로 해제 불가!
```

**해결책**: Mark-and-Sweep GC

```cpp
class GarbageCollector {
private:
    std::vector<Object*> objects_;  // 모든 할당된 객체

public:
    void collect() {
        // 1단계: Mark (도달 가능한 객체 표시)
        markRoots();  // 전역 변수, 스택 변수

        // 2단계: Sweep (표시 안 된 객체 해제)
        for (auto it = objects_.begin(); it != objects_.end(); ) {
            if (!(*it)->isMarked()) {
                delete *it;  // 해제
                it = objects_.erase(it);
            } else {
                (*it)->unmark();  // 다음 GC를 위해 표시 제거
                ++it;
            }
        }
    }

private:
    void markRoots() {
        // 전역 변수 마킹
        for (auto& [name, value] : globalEnv) {
            mark(value);
        }

        // 스택 변수 마킹
        for (auto* env : envStack) {
            for (auto& [name, value] : env->store()) {
                mark(value);
            }
        }
    }

    void mark(Object* obj) {
        if (!obj || obj->isMarked()) return;

        obj->mark();

        // 참조하는 객체도 마킹 (재귀)
        for (Object* ref : obj->references()) {
            mark(ref);
        }
    }
};
```

### 4.3 GC 전략 비교

| 전략 | 장점 | 단점 | 사용 |
|------|------|------|------|
| **Reference Counting** | 즉시 해제, 결정적 | 순환 참조 처리 불가 | Python (CPython) |
| **Mark-and-Sweep** | 간단, 순환 참조 처리 | STW (Stop-The-World) | 킹세종 (권장) |
| **Generational** | 효율적 (대부분 객체 단명) | 복잡함 | Java, V8 |
| **Incremental** | STW 시간 짧음 | 복잡함, 오버헤드 | V8 (병행) |

**킹세종 권장**: Mark-and-Sweep (간단하고 효과적)

---

## 5. 성능 비교

### 5.1 Evaluator vs Bytecode VM

| 지표 | Evaluator | Bytecode VM | 비율 |
|------|-----------|-------------|------|
| **실행 속도** | 느림 | 빠름 | 2-5x |
| **메모리 사용** | 많음 (AST 유지) | 적음 (Bytecode) | 0.3-0.5x |
| **시작 시간** | 빠름 (컴파일 불필요) | 느림 (컴파일 필요) | 0.5x |
| **디버깅** | 쉬움 | 어려움 | - |
| **구현 복잡도** | 낮음 | 중간 | - |

### 5.2 벤치마크 예측

**테스트 코드**:
```javascript
// 피보나치 (재귀)
함수 피보나치(정수 n) {
    만약 n <= 1이면 {
        반환한다(n)
    }
    반환한다(피보나치(n - 1) + 피보나치(n - 2))
}

피보나치(30)
```

**예상 결과**:
```
Evaluator:     ~5.0초
Bytecode VM:   ~2.0초 (2.5x 빠름)
JIT (미래):    ~0.1초 (50x 빠름)
네이티브 C++:  ~0.05초 (100x 빠름)
```

---

## 6. 개선 방안

### 6.1 긴급 (P0)

#### 1. GC 구현

**Mark-and-Sweep**:
```cpp
// src/memory/GC.h (신규)
class GarbageCollector {
public:
    void collect();
    Object* allocate(size_t size);

private:
    void mark();
    void sweep();
    std::vector<Object*> objects_;
};
```

#### 2. 타입 시스템 추가

**AST 노드에 타입 정보**:
```cpp
class Expression {
protected:
    Type* type_;  // 추가

public:
    Type* type() const { return type_; }
    void setType(Type* type) { type_ = type; }
};

// Semantic Analyzer에서 타입 추론
class TypeInference {
    void inferTypes(Program* program) {
        for (auto* stmt : program->statements()) {
            inferStatement(stmt);
        }
    }

    void inferStatement(Statement* stmt) {
        if (auto varDecl = dynamic_cast<VarDeclaration*>(stmt)) {
            Type* exprType = inferExpression(varDecl->value());
            varDecl->setValue->setType(exprType);
        }
    }
};
```

### 6.2 중요 (P1)

#### 3. Bytecode 최적화 강화

**Peephole Optimization**:
```cpp
// 최적화 예시
// 전:
LOAD_CONST 0
POP          // 의미 없는 로드/팝

// 후:
(제거)

// 전:
JUMP 10
JUMP 20      // 도달 불가능한 점프

// 후:
JUMP 20
```

#### 4. JIT 컴파일러 (장기 목표)

**LLVM 백엔드 통합**:
```cpp
// src/jit/LLVMBackend.cpp (미래)
class LLVMBackend {
public:
    llvm::Function* compile(FunctionDeclaration* func) {
        llvm::IRBuilder<> builder(context_);

        for (auto* stmt : func->body()->statements()) {
            compileStatement(stmt, builder);
        }

        return llvmFunc;
    }
};
```

### 6.3 개선 (P2)

#### 5. 프로파일링 기반 최적화

**핫스팟 감지**:
```cpp
class Profiler {
    std::unordered_map<Function*, int> callCounts_;

    void recordCall(Function* func) {
        callCounts_[func]++;

        if (callCounts_[func] > 1000) {
            // 핫스팟 감지 → JIT 컴파일
            jit_->compile(func);
        }
    }
};
```

---

## 7. 결론

### 7.1 핵심 평가

킹세종의 AST와 실행 엔진은 **견고한 기반**을 가지고 있으나, **GC 부재**와 **타입 시스템 미흡**으로 개선이 필요합니다.

**강점**:
- ✅ 명확한 AST 설계
- ✅ Dual Execution Engine (유연성)
- ✅ 한국어 특화 노드

**약점**:
- ⚠️ GC 없음 (순환 참조 누수 가능)
- ⚠️ 타입 정보 없음
- ⚠️ JIT 없음 (성능 한계)

### 7.2 우선순위

1. **🔴 긴급**: GC 구현, 타입 시스템
2. **🟡 중요**: Bytecode 최적화, JIT 연구
3. **🟢 개선**: 프로파일링, 고급 최적화

### 7.3 기대 효과

개선 완료 시:
- ✅ 메모리 누수 제거 (GC)
- ✅ 타입 안전성 향상
- ✅ 성능 10-100배 향상 (JIT)

---

**문서 끝**

**다음 문서**: [ANALYSIS_COMPARISON.md](./ANALYSIS_COMPARISON.md)
**이전 문서**: [ANALYSIS_PARSER.md](./ANALYSIS_PARSER.md)
