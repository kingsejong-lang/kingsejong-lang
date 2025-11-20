# Phase 9: 긴 함수 리팩토링 계획

> **작성일**: 2025-11-21
> **버전**: v0.5.0
> **목표**: Clean Code 원칙 적용 - 긴 함수 분해 및 가독성 향상

---

## 개요

Phase 8에서 식별된 긴 함수들을 리팩토링하여 코드 가독성, 유지보수성, 테스트 용이성을 향상시킵니다.

### 주요 타겟

- **VM::executeInstruction()**: 790줄 - 거대한 switch문
- **Evaluator::evalCallExpression()**: 345줄 - 다양한 호출 처리
- **Evaluator::evalJosaExpression()**: 282줄 - 조사 체인 메서드 처리
- **Evaluator::evalMatchExpression()**: 158줄 - 패턴 매칭

---

## 1. VM::executeInstruction() 리팩토링

### 현재 상태

**파일**: `src/bytecode/VM.cpp`
**라인**: 155~945줄 (790줄)
**문제점**:
- 56개의 OpCode를 하나의 거대한 switch문에서 처리
- 코드 탐색이 어렵고 수정 시 영향 범위 파악 곤란
- 단위 테스트 작성 어려움

### 리팩토링 전략

OpCode를 **기능별 카테고리**로 분류하고 각각 별도 메서드로 추출:

#### 1.1 상수 로드 (4개 OpCode)
```cpp
VMResult VM::executeConstantOps(OpCode instruction);
```
- `LOAD_CONST`, `LOAD_TRUE`, `LOAD_FALSE`, `LOAD_NULL`

#### 1.2 변수 조작 (4개 OpCode)
```cpp
VMResult VM::executeVariableOps(OpCode instruction);
```
- `LOAD_VAR`, `STORE_VAR`, `LOAD_GLOBAL`, `STORE_GLOBAL`

#### 1.3 산술 연산 (6개 OpCode)
```cpp
VMResult VM::executeArithmeticOps(OpCode instruction);
```
- `ADD`, `SUB`, `MUL`, `DIV`, `MOD`, `NEG`

#### 1.4 비교 연산 (6개 OpCode)
```cpp
VMResult VM::executeComparisonOps(OpCode instruction);
```
- `EQ`, `NE`, `LT`, `LE`, `GT`, `GE`

#### 1.5 논리 연산 (3개 OpCode)
```cpp
VMResult VM::executeLogicalOps(OpCode instruction);
```
- `AND`, `OR`, `NOT`

#### 1.6 제어 흐름 (7개 OpCode)
```cpp
VMResult VM::executeControlFlowOps(OpCode instruction);
```
- `JUMP`, `JUMP_IF_FALSE`, `LOOP`, `CALL`, `RETURN`, `HALT`, `POP`

#### 1.7 배열/딕셔너리 (5개 OpCode)
```cpp
VMResult VM::executeArrayOps(OpCode instruction);
```
- `BUILD_ARRAY`, `BUILD_DICT`, `INDEX_GET`, `INDEX_SET`, `ARRAY_APPEND`

#### 1.8 Promise/Async (5개 OpCode)
```cpp
VMResult VM::executePromiseOps(OpCode instruction);
```
- `AWAIT`, `BUILD_PROMISE`, `PROMISE_RESOLVE`, `PROMISE_REJECT`, `PROMISE_THEN/CATCH`

#### 1.9 기타 (나머지)
```cpp
VMResult VM::executeMiscOps(OpCode instruction);
```
- `PRINT`, `BUILD_RANGE`, `IMPORT` 등

### 리팩토링 후 구조

```cpp
VMResult VM::executeInstruction() {
    OpCode instruction = static_cast<OpCode>(readByte());

    // OpCode 카테고리별 분기
    if (instruction >= OpCode::LOAD_CONST && instruction <= OpCode::LOAD_NULL) {
        return executeConstantOps(instruction);
    }
    else if (instruction >= OpCode::LOAD_VAR && instruction <= OpCode::STORE_GLOBAL) {
        return executeVariableOps(instruction);
    }
    else if (instruction >= OpCode::ADD && instruction <= OpCode::NEG) {
        return executeArithmeticOps(instruction);
    }
    // ... 나머지 카테고리

    runtimeError("알 수 없는 OpCode");
    return VMResult::RUNTIME_ERROR;
}
```

### 예상 효과

- ✅ 각 메서드 100줄 이하로 축소
- ✅ 기능별 단위 테스트 작성 가능
- ✅ OpCode 추가 시 수정 범위 명확화
- ✅ 코드 탐색 및 이해 용이

---

## 2. Evaluator::evalCallExpression() 리팩토링

### 현재 상태

**파일**: `src/evaluator/Evaluator.cpp`
**라인**: 480~825줄 (345줄)
**문제점**:
- Promise 메서드, 내장 함수, 사용자 함수, 생성자 등 다양한 호출 처리
- 긴 중첩 if문으로 가독성 저하
- 각 호출 타입별 테스트 어려움

### 리팩토링 전략

호출 타입별로 별도 메서드 추출:

#### 2.1 Promise 메서드 호출
```cpp
Value Evaluator::evalPromiseMethodCall(
    const std::shared_ptr<Promise>& promise,
    const std::string& methodName,
    const std::vector<std::unique_ptr<ast::Expression>>& args
);
```
- `.then`, `.catch` (그러면, 오류시)

#### 2.2 내장 메서드 호출
```cpp
Value Evaluator::evalBuiltinMethodCall(
    const Value& object,
    const std::string& methodName,
    const std::vector<Value>& args
);
```
- 배열, 딕셔너리, 문자열 등의 내장 메서드

#### 2.3 사용자 정의 함수 호출
```cpp
Value Evaluator::evalUserFunctionCall(
    const std::shared_ptr<Function>& func,
    const std::vector<Value>& args
);
```
- 일반 함수, async 함수

#### 2.4 생성자 호출
```cpp
Value Evaluator::evalConstructorCall(
    const std::shared_ptr<Function>& constructor,
    const std::vector<Value>& args
);
```
- 클래스 인스턴스 생성

### 리팩토링 후 구조

```cpp
Value Evaluator::evalCallExpression(ast::CallExpression* expr) {
    // 1. 메서드 호출인 경우
    if (auto* memberAccess = dynamic_cast<ast::MemberAccessExpression*>(expr->function())) {
        Value obj = eval(memberAccess->object());
        std::string methodName = memberAccess->memberName();

        // Promise 메서드
        if (obj.isPromise()) {
            return evalPromiseMethodCall(obj.asPromise(), methodName, expr->arguments());
        }

        // 내장 메서드
        return evalBuiltinMethodCall(obj, methodName, evaluateArguments(expr->arguments()));
    }

    // 2. 일반 함수 호출
    Value func = eval(expr->function());
    std::vector<Value> args = evaluateArguments(expr->arguments());

    if (func.isFunction()) {
        auto funcObj = func.asFunction();

        // 생성자 호출
        if (funcObj->isConstructor()) {
            return evalConstructorCall(funcObj, args);
        }

        // 일반 함수 호출
        return evalUserFunctionCall(funcObj, args);
    }

    throw error::TypeError("호출 가능한 객체가 아닙니다");
}
```

### 예상 효과

- ✅ 각 메서드 50~100줄로 축소
- ✅ 호출 타입별 독립적 테스트 가능
- ✅ 새로운 호출 타입 추가 용이
- ✅ 코드 흐름 파악 간편화

---

## 3. Evaluator::evalJosaExpression() 리팩토링

### 현재 상태

**파일**: `src/evaluator/Evaluator.cpp`
**라인**: 1450~1732줄 (282줄)
**문제점**:
- 걸러낸다, 변환한다, 합친다, 정렬한다 등 다수 메서드를 하나의 함수에서 처리
- 각 메서드마다 긴 중첩 로직
- 메서드별 테스트 및 디버깅 어려움

### 리팩토링 전략

조사 메서드별로 별도 함수 추출:

#### 3.1 필터 메서드
```cpp
Value Evaluator::evalJosaFilter(
    const std::vector<Value>& array,
    const Value& filterFunc
);
```
- `걸러낸다`, `걸러내고`

#### 3.2 맵 메서드
```cpp
Value Evaluator::evalJosaMap(
    const std::vector<Value>& array,
    const Value& mapFunc
);
```
- `변환한다`, `변환하고`

#### 3.3 리듀스 메서드
```cpp
Value Evaluator::evalJosaReduce(
    const std::vector<Value>& array,
    const Value& reduceFunc,
    const Value& initialValue
);
```
- `합친다`, `합치고`

#### 3.4 정렬 메서드
```cpp
Value Evaluator::evalJosaSort(
    const std::vector<Value>& array,
    const Value& compareFunc = Value::createNull()
);
```
- `정렬한다`, `정렬하고`

#### 3.5 역순 메서드
```cpp
Value Evaluator::evalJosaReverse(
    const std::vector<Value>& array
);
```
- `역순으로_나열한다`, `역순으로_나열하고`

### 리팩토링 후 구조

```cpp
Value Evaluator::evalJosaExpression(ast::JosaExpression* expr) {
    Value object = eval(expr->object());

    if (!object.isArray()) {
        throw error::TypeError("조사 표현식은 배열에만 사용 가능");
    }

    auto arr = object.asArray();
    auto* callExpr = dynamic_cast<ast::CallExpression*>(expr->method());

    if (!callExpr) {
        throw error::TypeError("조사 표현식의 메서드가 잘못되었습니다");
    }

    std::string methodName = getMethodName(callExpr);

    // 메서드별 분기
    if (methodName == "걸러낸다" || methodName == "걸러내고") {
        return evalJosaFilter(arr, evaluateArg(callExpr, 0));
    }
    else if (methodName == "변환한다" || methodName == "변환하고") {
        return evalJosaMap(arr, evaluateArg(callExpr, 0));
    }
    else if (methodName == "합친다" || methodName == "합치고") {
        return evalJosaReduce(arr, evaluateArg(callExpr, 0), evaluateArg(callExpr, 1));
    }
    else if (methodName == "정렬한다" || methodName == "정렬하고") {
        return evalJosaSort(arr, callExpr->arguments().empty() ? Value::createNull() : evaluateArg(callExpr, 0));
    }
    else if (methodName == "역순으로_나열한다" || methodName == "역순으로_나열하고") {
        return evalJosaReverse(arr);
    }

    throw error::NameError("지원하지 않는 조사 메서드: " + methodName);
}
```

### 예상 효과

- ✅ 각 메서드 30~50줄로 축소
- ✅ 메서드별 독립 테스트 가능
- ✅ 새로운 조사 메서드 추가 간편
- ✅ 디버깅 및 오류 추적 용이

---

## 4. Evaluator::evalMatchExpression() 리팩토링

### 현재 상태

**파일**: `src/evaluator/Evaluator.cpp`
**라인**: 1734~1892줄 (158줄)
**문제점**:
- 패턴 타입별 매칭 로직이 하나의 switch문에 집중
- 패턴 타입 추가 시 함수 비대화

### 리팩토링 전략

패턴 타입별 매칭 함수 추출:

#### 4.1 리터럴 패턴
```cpp
bool Evaluator::matchLiteralPattern(
    const Value& matchValue,
    ast::Node* pattern,
    std::shared_ptr<Environment> env
);
```

#### 4.2 식별자 패턴 (바인딩)
```cpp
bool Evaluator::matchIdentifierPattern(
    const Value& matchValue,
    ast::Identifier* pattern,
    std::shared_ptr<Environment> env
);
```

#### 4.3 배열 패턴
```cpp
bool Evaluator::matchArrayPattern(
    const Value& matchValue,
    ast::ArrayLiteral* pattern,
    std::shared_ptr<Environment> env
);
```

#### 4.4 와일드카드 패턴 (_)
```cpp
bool Evaluator::matchWildcardPattern(
    const Value& matchValue,
    ast::Node* pattern
);
```

### 리팩토링 후 구조

```cpp
Value Evaluator::evalMatchExpression(ast::MatchExpression* node) {
    Value matchValue = eval(node->value());

    for (const auto& matchCase : node->cases()) {
        auto caseEnv = std::make_shared<Environment>(env_);
        auto pattern = matchCase.pattern();

        bool matched = false;

        switch (pattern->type()) {
            case ast::NodeType::IntegerLiteral:
            case ast::NodeType::StringLiteral:
            case ast::NodeType::BooleanLiteral:
                matched = matchLiteralPattern(matchValue, pattern, caseEnv);
                break;

            case ast::NodeType::Identifier:
                matched = matchIdentifierPattern(matchValue, dynamic_cast<ast::Identifier*>(pattern), caseEnv);
                break;

            case ast::NodeType::ArrayLiteral:
                matched = matchArrayPattern(matchValue, dynamic_cast<ast::ArrayLiteral*>(pattern), caseEnv);
                break;

            default:
                matched = matchWildcardPattern(matchValue, pattern);
                break;
        }

        if (matched) {
            auto previousEnv = env_;
            env_ = caseEnv;
            Value result = eval(matchCase.body());
            env_ = previousEnv;
            return result;
        }
    }

    throw error::RuntimeError("매칭되는 패턴이 없습니다");
}
```

### 예상 효과

- ✅ 각 매칭 함수 20~40줄
- ✅ 패턴 타입별 독립 테스트 가능
- ✅ 새로운 패턴 타입 추가 용이
- ✅ 패턴 매칭 로직 명확화

---

## 5. 리팩토링 단계별 계획

### Step 1: VM::executeInstruction() (우선순위 1)

1. OpCode 카테고리 정의
2. 각 카테고리별 메서드 추출
3. 기존 테스트 통과 확인
4. 새로운 단위 테스트 추가

**예상 기간**: 3~4일
**테스트**: 기존 1,503개 테스트 100% 통과 유지

### Step 2: Evaluator::evalCallExpression() (우선순위 2)

1. 호출 타입별 메서드 추출
2. evaluateArguments() 헬퍼 추가
3. 기존 테스트 통과 확인
4. 호출 타입별 단위 테스트 추가

**예상 기간**: 2~3일
**테스트**: 함수 호출 관련 모든 테스트 통과

### Step 3: Evaluator::evalJosaExpression() (우선순위 3)

1. 조사 메서드별 함수 추출
2. getMethodName(), evaluateArg() 헬퍼 추가
3. 기존 테스트 통과 확인
4. 조사 메서드별 단위 테스트 추가

**예상 기간**: 2~3일
**테스트**: 조사 체인 관련 모든 테스트 통과

### Step 4: Evaluator::evalMatchExpression() (우선순위 4)

1. 패턴 타입별 매칭 함수 추출
2. 기존 테스트 통과 확인
3. 패턴 타입별 단위 테스트 추가

**예상 기간**: 1~2일
**테스트**: 패턴 매칭 관련 모든 테스트 통과

---

## 6. 테스트 전략

### 6.1 회귀 테스트

모든 리팩토링 단계마다:

```bash
# 전체 테스트 실행
ctest --output-on-failure

# 특정 모듈 테스트
./build/bin/kingsejong_tests --gtest_filter="VM*"
./build/bin/kingsejong_tests --gtest_filter="Evaluator*"
```

### 6.2 새로운 단위 테스트

리팩토링 후 각 분리된 메서드에 대한 단위 테스트 추가:

- `VMConstantOpsTest.cpp` - 상수 로드 OpCode 테스트
- `VMVariableOpsTest.cpp` - 변수 OpCode 테스트
- `EvaluatorPromiseCallTest.cpp` - Promise 메서드 호출 테스트
- `EvaluatorJosaFilterTest.cpp` - 걸러낸다 메서드 테스트
- 등등...

**목표**: 테스트 커버리지 95% 유지

---

## 7. 성공 기준

### Phase 9 완료 조건

- ✅ VM::executeInstruction() → 100줄 이하 메서드 9개로 분해
- ✅ Evaluator::evalCallExpression() → 100줄 이하 메서드 4개로 분해
- ✅ Evaluator::evalJosaExpression() → 50줄 이하 메서드 5개로 분해
- ✅ Evaluator::evalMatchExpression() → 50줄 이하 메서드 4개로 분해
- ✅ 기존 1,503개 테스트 100% 통과
- ✅ 새로운 단위 테스트 50개 이상 추가
- ✅ 코드 리뷰 및 문서 업데이트

### 코드 품질 지표

- 함수 평균 길이: 50줄 이하
- 최대 함수 길이: 200줄 이하
- Cyclomatic Complexity: 10 이하
- 테스트 커버리지: 95% 이상

---

## 8. 참고 자료

- **Clean Code (Robert C. Martin)**: 함수는 한 가지 일만 해야 한다
- **Refactoring (Martin Fowler)**: Extract Method 패턴
- **C++ Core Guidelines**: F.1 - Package meaningful operations as carefully named functions

---

**마지막 업데이트**: 2025-11-21
**다음 단계**: Step 1 (VM::executeInstruction() 리팩토링) 시작
