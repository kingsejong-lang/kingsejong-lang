# Evaluator 리팩토링 계획 (Phase 9 Step 2)

> **날짜**: 2025-11-21
> **목표**: Evaluator::evalCallExpression() 메서드 분해 (345줄 → 5개 메서드)
> **참고**: VM 리팩토링 성공 사례 (790줄 → 50줄, 9개 메서드)

---

## 1. 현황 분석

### 1.1 Evaluator.cpp 전체 구조

- **파일 크기**: 2,016 줄
- **주요 메서드**: 39개
- **긴 메서드 Top 3**:
  1. `evalCallExpression()`: **345줄** (480-825) ⚠️ **최우선 리팩토링 대상**
  2. `evalJosaExpression()`: **284줄** (1450-1733) - 배열 함수형 메서드
  3. `evalMatchExpression()`: **164줄** (1734-1897) - 패턴 매칭

### 1.2 evalCallExpression() 구조 분석

**총 345줄**을 다음 5가지 시나리오로 처리:

#### 📌 Section 1: Promise 메서드 호출 (Lines 484-587, ~104줄)
- **조건**: `MemberAccessExpression` + `obj.isPromise()`
- **처리 내용**:
  - `.then()` / `.그러면()` 메서드
  - `.catch()` / `.오류시()` 메서드
  - 콜백 함수 유효성 검사
  - Continuation 생성 및 등록
  - Promise 상태별 처리 (FULFILLED, PENDING, REJECTED)
  - 체이닝 지원
- **추출 메서드**: `evalPromiseMethodCall()`

#### 📌 Section 2: 클래스 인스턴스 메서드 호출 (Lines 589-654, ~66줄)
- **조건**: `MemberAccessExpression` + `obj.isClassInstance()`
- **처리 내용**:
  - 클래스 정의에서 메서드 조회
  - 인자 평가 및 매개변수 개수 검증
  - 메서드 환경 생성 (클로저 기반)
  - `자신` 키워드 바인딩
  - 메서드 본문 실행
- **추출 메서드**: `evalClassMethodCall()`

#### 📌 Section 3: 내장 함수 호출 (Lines 668-673, ~6줄)
- **조건**: `funcValue.isBuiltinFunction()`
- **처리 내용**:
  - 내장 함수 직접 호출
  - 인자 전달 및 결과 반환
- **추출 메서드**: `evalBuiltinFunctionCall()` (간단하므로 통합 가능)

#### 📌 Section 4: 클래스 생성자 호출 (Lines 686-738, ~53줄)
- **조건**: `func->classDef() != nullptr`
- **처리 내용**:
  - ClassInstance 생성
  - 생성자 존재 시 실행
  - 생성자 환경 생성
  - 매개변수 바인딩
  - `자신` 키워드 바인딩
  - 인스턴스 반환
- **추출 메서드**: `evalConstructorCall()`

#### 📌 Section 5: 일반 함수 호출 (Lines 741-824, ~84줄)
- **조건**: 기본 케이스 (사용자 정의 함수)
- **처리 내용**:
  - 매개변수 개수 검증
  - Hot Path 추적 (JIT 준비)
  - 함수 환경 생성 (클로저 기반)
  - 매개변수 바인딩
  - 함수 본문 실행
  - async 함수 처리 (Promise 래핑)
- **추출 메서드**: `evalRegularFunctionCall()`

---

## 2. 리팩토링 전략

### 2.1 목표

- ✅ **코드 가독성 향상**: 345줄 → 50줄 이하로 축소 (VM 리팩토링과 유사)
- ✅ **관심사 분리**: 5가지 호출 시나리오를 독립적인 메서드로 분리
- ✅ **유지보수성 향상**: 각 메서드의 책임 명확화
- ✅ **테스트 유지**: 전체 1,490개 테스트 100% 통과 유지

### 2.2 추출할 메서드 (5개)

```cpp
// 1. Promise 메서드 호출 (.then, .catch)
Value evalPromiseMethodCall(
    ast::MemberAccessExpression* memberAccess,
    const std::vector<std::unique_ptr<ast::Expression>>& arguments,
    const Value& promiseObj
);

// 2. 클래스 메서드 호출
Value evalClassMethodCall(
    ast::MemberAccessExpression* memberAccess,
    const std::vector<std::unique_ptr<ast::Expression>>& arguments,
    const Value& instanceObj
);

// 3. 클래스 생성자 호출
Value evalConstructorCall(
    std::shared_ptr<Function> func,
    const std::vector<Value>& args
);

// 4. 일반 함수 호출
Value evalRegularFunctionCall(
    ast::CallExpression* expr,
    std::shared_ptr<Function> func,
    const std::vector<Value>& args
);

// 5. 내장 함수 호출 (간단하므로 inline 가능)
// evalBuiltinFunctionCall() - 필요시 추가
```

### 2.3 리팩토링된 evalCallExpression() 구조

```cpp
Value Evaluator::evalCallExpression(ast::CallExpression* expr)
{
    // 1. 메서드 호출인지 확인 (MemberAccessExpression)
    if (auto* memberAccess = dynamic_cast<ast::MemberAccessExpression*>(
        const_cast<ast::Expression*>(expr->function())))
    {
        Value obj = eval(const_cast<ast::Expression*>(memberAccess->object()));

        // Promise 메서드 호출
        if (obj.isPromise()) {
            return evalPromiseMethodCall(memberAccess, expr->arguments(), obj);
        }

        // 클래스 메서드 호출
        if (obj.isClassInstance()) {
            return evalClassMethodCall(memberAccess, expr->arguments(), obj);
        }

        throw error::RuntimeError("메서드 호출은 클래스 인스턴스 또는 Promise에만 가능합니다");
    }

    // 2. 함수 및 인자 평가
    Value funcValue = eval(const_cast<ast::Expression*>(expr->function()));
    std::vector<Value> args;
    for (const auto& argExpr : expr->arguments()) {
        args.push_back(eval(const_cast<ast::Expression*>(argExpr.get())));
    }

    // 3. 내장 함수 호출
    if (funcValue.isBuiltinFunction()) {
        return funcValue.asBuiltinFunction()(args);
    }

    // 4. 사용자 정의 함수인지 확인
    if (!funcValue.isFunction()) {
        throw error::TypeError(/* ... */);
    }

    auto func = funcValue.asFunction();

    // 5. 생성자 호출
    if (func->classDef()) {
        return evalConstructorCall(func, args);
    }

    // 6. 일반 함수 호출
    return evalRegularFunctionCall(expr, func, args);
}
```

**예상 크기**: ~50줄 (345줄 → 50줄, **85% 감소**)

---

## 3. 구현 단계

### Phase 1: 헤더 파일 수정 (Evaluator.h)
- [ ] 5개 private 메서드 선언 추가
- [ ] Doxygen 주석 작성

### Phase 2: Helper 메서드 구현 (Evaluator.cpp)
- [ ] `evalPromiseMethodCall()` 구현
- [ ] `evalClassMethodCall()` 구현
- [ ] `evalConstructorCall()` 구현
- [ ] `evalRegularFunctionCall()` 구현
- [ ] 각 단계마다 테스트 실행하여 회귀 방지

### Phase 3: evalCallExpression() 리팩토링
- [ ] 기존 코드를 helper 메서드 호출로 교체
- [ ] 코드 크기 확인 (목표: 50줄 이하)

### Phase 4: 테스트 및 검증
- [ ] 전체 테스트 실행: `ctest --output-on-failure`
- [ ] 1,490개 테스트 100% 통과 확인
- [ ] 메모리 누수 검사: `valgrind` 또는 ASan

### Phase 5: 문서 업데이트
- [ ] ROADMAP.md 업데이트 (Phase 9 Step 2 완료 표시)
- [ ] TODOLIST.md 업데이트
- [ ] 커밋 메시지: `refactor: Evaluator::evalCallExpression() 분해 (345줄 → 50줄)`

---

## 4. 예상 결과

### Before (현재)
```
evalCallExpression()      345줄  ⚠️ 너무 길고 복잡
```

### After (리팩토링 후)
```
evalCallExpression()              50줄  ✅ 명확하고 간결
├── evalPromiseMethodCall()      ~100줄
├── evalClassMethodCall()         ~60줄
├── evalConstructorCall()         ~50줄
└── evalRegularFunctionCall()     ~80줄
```

**총 코드**: 345줄 → 340줄 (약간 증가, but 가독성 대폭 향상)
**핵심 메서드**: 345줄 → 50줄 (**85% 감소**)

---

## 5. 리스크 및 대응

### 리스크 1: 테스트 실패
- **대응**: 각 helper 메서드 구현 후 즉시 테스트 실행
- **롤백**: Git에서 이전 커밋으로 복원

### 리스크 2: 성능 저하
- **대응**: 함수 호출 오버헤드는 미미함 (inline 최적화 가능)
- **검증**: 벤치마크 실행하여 성능 회귀 없음 확인

### 리스크 3: 코드 중복
- **대응**: 공통 로직(환경 생성, 매개변수 바인딩)은 추가 helper 함수로 추출

---

## 6. 추가 리팩토링 후보

Phase 9 Step 2 완료 후 검토할 메서드:

### evalJosaExpression() (284줄)
- **현재**: 배열 함수형 메서드 (filter, map, reduce, find)를 하나의 메서드에서 처리
- **제안**: 각 메서드를 독립적인 helper로 분리
  - `evalJosaFilter()` (~80줄)
  - `evalJosaMap()` (~80줄)
  - `evalJosaReduce()` (~80줄)
  - `evalJosaFind()` (~50줄)
- **우선순위**: Medium (현재 구조도 비교적 명확함)

### evalBinaryExpression() (76줄)
- **현재**: 이미 적당한 크기
- **제안**: 리팩토링 불필요

---

## 7. 성공 기준

- ✅ evalCallExpression() 크기: 50줄 이하
- ✅ 전체 테스트: 1,490개 100% 통과
- ✅ 코드 가독성: 각 메서드의 책임이 명확함
- ✅ 유지보수성: 새로운 호출 타입 추가 시 독립적인 메서드만 수정
- ✅ 문서화: Doxygen 주석 및 ROADMAP 업데이트 완료

---

## 8. 타임라인

- **분석 완료**: 2025-11-21 ✅
- **구현 시작**: 2025-11-21 (예정)
- **완료 목표**: 2025-11-21 (당일 완료)
- **Phase 9 완료**: 2025-11 말

---

**참고 문서**:
- [VM 리팩토링 성공 사례](../src/bytecode/VM.cpp) - 790줄 → 50줄
- [ROADMAP.md](./ROADMAP.md) - Phase 9 진행 상황
- [TODOLIST.md](./TODOLIST.md) - 작업 목록
