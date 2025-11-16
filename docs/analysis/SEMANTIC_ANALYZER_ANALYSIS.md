# SemanticAnalyzer 현황 분석

**작성일**: 2025-11-16
**작업자**: KingSejong Team
**관련 과제**: P0-1 Semantic Analyzer 완전 구현

## 📋 요약

**현재 SemanticAnalyzer는 기본 기능은 동작하지만, Scope 관리가 불완전합니다.**

**테스트 통과율**:
- ✅ NameResolutionTest: 13/13 (100%)
- ✅ TypeCheckingTest: 19/19 (100%)
- ✅ SemanticSymbolTableTest: 9/9 (100%)

**하지만 Scope 격리가 제대로 동작하지 않음!**

---

## 🔴 핵심 문제: Scope 관리 불완전

### 문제 재현

```javascript
// scope_test.ksj
만약 참 {
    정수 x = 10
}
정수 y = x  // ❌ 에러가 발생해야 하지만 통과함!
```

**결과**:
```bash
$ ./build/bin/kingsejong /tmp/scope_test.ksj
(에러 없이 실행됨)
```

**예상 동작**:
```
에러: 정의되지 않은 변수 'x'를 사용하려고 합니다.
```

### 원인 분석

#### 1. buildSymbolTable() - Scope 미사용

**위치**: `src/semantic/SemanticAnalyzer.cpp:97-103`

```cpp
void SemanticAnalyzer::buildSymbolTable(Program* program)
{
    for (auto& stmt : program->statements())
    {
        registerSymbolFromStatement(stmt.get());  // ❌ Scope 처리 없음!
    }
}
```

**문제**:
- `BlockStatement` 처리 시 `enterScope()`/`exitScope()` 호출 안 함
- 모든 변수가 전역 스코프에 등록됨
- 블록 내 변수가 블록 밖에서도 보임

#### 2. registerSymbolFromStatement() - 부분적 Scope 처리

**위치**: `src/semantic/SemanticAnalyzer.cpp:105-195`

```cpp
void SemanticAnalyzer::registerSymbolFromStatement(const Statement* stmt)
{
    // ...

    // BlockStatement: 블록 내 모든 문장 등록
    else if (auto blockStmt = dynamic_cast<const BlockStatement*>(stmt))
    {
        for (const auto& s : blockStmt->statements())
        {
            registerSymbolFromStatement(s.get());  // ❌ Scope 진입/탈출 없음!
        }
    }

    // IfStatement: then과 else 블록 등록
    else if (auto ifStmt = dynamic_cast<const IfStatement*>(stmt))
    {
        if (ifStmt->thenBranch())
        {
            registerSymbolFromStatement(ifStmt->thenBranch());  // ❌ Scope 처리 없음!
        }
        if (ifStmt->elseBranch())
        {
            registerSymbolFromStatement(ifStmt->elseBranch());  // ❌ Scope 처리 없음!
        }
    }

    // 함수 리터럴의 body도 마찬가지...
}
```

#### 3. resolveNames() - Scope 사용함 (일관성 없음!)

**위치**: `src/semantic/SemanticAnalyzer.cpp:253-274`

```cpp
// IfStatement: 조건과 블록들 검증
else if (auto ifStmt = dynamic_cast<const IfStatement*>(stmt))
{
    resolveNamesInExpression(ifStmt->condition());

    if (ifStmt->thenBranch())
    {
        symbolTable_.enterScope();  // ✅ Scope 진입 (resolveNames에서만!)
        resolveNamesInStatement(ifStmt->thenBranch());
        symbolTable_.exitScope();   // ✅ Scope 탈출
    }

    if (ifStmt->elseBranch())
    {
        symbolTable_.enterScope();  // ✅ Scope 진입
        resolveNamesInStatement(ifStmt->elseBranch());
        symbolTable_.exitScope();   // ✅ Scope 탈출
    }
}
```

**문제**:
- `resolveNames()`는 Scope를 사용
- `buildSymbolTable()`은 Scope를 사용하지 않음
- **일관성 부족!**

---

## 📊 현재 구현 상태

### ✅ 정상 동작하는 부분

1. **SymbolTable & Scope** (완전 구현됨)
   - Scope 클래스: 부모 스코프 추적, 심볼 정의/조회
   - SymbolTable 클래스: enterScope/exitScope, define, lookup
   - 중첩 스코프 지원
   - 테스트: 9/9 통과

2. **Name Resolution** (부분적으로 동작)
   - 정의되지 않은 변수 감지
   - Builtin 함수 인식
   - 표현식 내 식별자 검증
   - 테스트: 13/13 통과
   - **하지만**: Scope 격리는 제대로 검증되지 않음

3. **Type Checking** (기본 동작)
   - 타입 추론 (inferType)
   - 타입 호환성 검사
   - 변수 선언 시 타입 검사
   - 할당문 타입 검사
   - 테스트: 19/19 통과

### ❌ 불완전한 부분

1. **buildSymbolTable - Scope 미사용**
   - BlockStatement: Scope 처리 없음
   - IfStatement: Scope 처리 없음
   - RepeatStatement: Scope 처리 없음
   - RangeForStatement: Scope 처리 없음
   - FunctionLiteral body: Scope 처리 없음

2. **테스트 커버리지 부족**
   - Scope 격리 테스트 없음
   - 블록 밖에서 블록 내 변수 접근 시도 테스트 없음
   - 함수 매개변수 스코프 테스트 없음

3. **TODO 주석들**
   ```cpp
   // TODO: 모든 식별자가 정의된 심볼인지 확인 (line 202)
   // TODO: 타입 검사 구현 (line 447)
   // TODO: 현재 함수의 반환 타입과 비교 (line 509)
   // TODO: 모호성 해결 구현 (line 775, 785)
   ```

---

## 🎯 해결 방안

### 필수 개선 사항 (P0)

#### 1. buildSymbolTable에 Scope 처리 추가

**목표**: 모든 블록/함수에서 올바른 Scope 진입/탈출

**구현**:
```cpp
void SemanticAnalyzer::registerSymbolFromStatement(const Statement* stmt)
{
    // BlockStatement: 새로운 스코프 생성
    if (auto blockStmt = dynamic_cast<const BlockStatement*>(stmt))
    {
        symbolTable_.enterScope();  // ✅ 스코프 진입
        for (const auto& s : blockStmt->statements())
        {
            registerSymbolFromStatement(s.get());
        }
        symbolTable_.exitScope();   // ✅ 스코프 탈출
    }

    // IfStatement: then/else 각각 스코프 생성
    else if (auto ifStmt = dynamic_cast<const IfStatement*>(stmt))
    {
        if (ifStmt->thenBranch())
        {
            symbolTable_.enterScope();
            registerSymbolFromStatement(ifStmt->thenBranch());
            symbolTable_.exitScope();
        }
        if (ifStmt->elseBranch())
        {
            symbolTable_.enterScope();
            registerSymbolFromStatement(ifStmt->elseBranch());
            symbolTable_.exitScope();
        }
    }

    // RangeForStatement: 루프 변수는 블록 스코프에 속함
    else if (auto rangeFor = dynamic_cast<const RangeForStatement*>(stmt))
    {
        symbolTable_.enterScope();
        // 루프 변수 등록
        symbolTable_.define(rangeFor->varName(), SymbolKind::VARIABLE,
                          types::TypeRegistry::getInteger());
        // 블록 내 문장 등록
        registerSymbolFromStatement(rangeFor->body());
        symbolTable_.exitScope();
    }

    // FunctionLiteral: 매개변수와 body는 함수 스코프
    // (함수 등록 시 별도 처리 필요)
}
```

#### 2. 함수 매개변수 Scope 처리

**문제**: 함수 매개변수가 함수 body 스코프에 등록되지 않음

**구현**:
```cpp
void SemanticAnalyzer::registerFunction(const std::string& name, FunctionLiteral* funcLit)
{
    // 1. 함수를 현재 스코프에 등록
    symbolTable_.define(name, SymbolKind::FUNCTION,
                       types::TypeRegistry::getFunction());

    // 2. 함수 body용 새로운 스코프 생성
    symbolTable_.enterScope();

    // 3. 매개변수를 함수 스코프에 등록
    for (const auto& param : funcLit->parameters())
    {
        symbolTable_.define(param, SymbolKind::VARIABLE,
                          types::TypeRegistry::getAny());  // 타입 추론 가능하면 개선
    }

    // 4. 함수 body 등록
    if (funcLit->body())
    {
        registerSymbolFromStatement(funcLit->body());
    }

    // 5. 함수 스코프 탈출
    symbolTable_.exitScope();
}
```

#### 3. 테스트 추가

**테스트 케이스**:
```cpp
// 1. 블록 밖에서 블록 내 변수 접근 (실패해야 함)
TEST(NameResolutionTest, ShouldDetectVariableOutsideBlockScope)
{
    std::string code = R"(
만약 참 {
    정수 x = 10
}
정수 y = x  // ❌ 에러: x는 블록 밖에서 보이지 않음
)";
    // ...
    EXPECT_FALSE(result);
    EXPECT_GT(analyzer.errors().size(), 0);
}

// 2. 함수 매개변수는 함수 밖에서 보이지 않음
TEST(NameResolutionTest, ShouldNotAccessFunctionParametersOutside)
{
    std::string code = R"(
함수 더하기(a, b) {
    a + b
}
정수 x = a  // ❌ 에러: a는 함수 밖에서 보이지 않음
)";
    // ...
}

// 3. 중첩 블록 스코프
TEST(NameResolutionTest, ShouldHandleNestedBlockScopes)
{
    std::string code = R"(
정수 x = 10
만약 참 {
    정수 x = 20  // ✅ 새로운 변수 (shadowing)
    만약 참 {
        정수 x = 30  // ✅ 또 다른 새로운 변수
    }
}
)";
    // ...
}

// 4. 루프 변수 스코프
TEST(NameResolutionTest, ShouldDetectLoopVariableOutsideLoop)
{
    std::string code = R"(
i가 1부터 10까지 반복한다 {
    출력(i)
}
정수 x = i  // ❌ 에러: i는 루프 밖에서 보이지 않음
)";
    // ...
}
```

### 선택적 개선 사항 (P1)

1. **함수 반환 타입 검사**
   - TODO line 509 구현
   - 함수가 선언한 반환 타입과 실제 반환 값 비교

2. **상수 (const) 지원**
   - `const` 키워드 도입
   - Symbol의 `isMutable` 플래그 활용
   - 상수에 재할당 시도 시 에러

3. **타입 추론 강화**
   - 함수 매개변수 타입 추론
   - 변수 선언 시 타입 생략 가능
   - 컨텍스트 기반 타입 추론

---

## 📝 작업 계획

### Phase 1: Scope 처리 완성 (필수)

**예상 공수**: 1-2일

1. ✅ 현황 분석 완료
2. ⏳ buildSymbolTable에 Scope 처리 추가
   - BlockStatement
   - IfStatement
   - RepeatStatement
   - RangeForStatement
3. ⏳ 함수 매개변수 Scope 처리
4. ⏳ 테스트 추가 (4개 이상)
5. ⏳ 전체 테스트 실행 및 회귀 검증

### Phase 2: TODO 해결 (선택)

**예상 공수**: 2-3일

1. 함수 반환 타입 검사
2. 상수 지원
3. 타입 추론 강화
4. 모호성 해결 로직 정리

---

## 🔍 관련 파일

- `src/semantic/SemanticAnalyzer.h` - 인터페이스
- `src/semantic/SemanticAnalyzer.cpp` - 구현 (800줄)
- `src/semantic/SymbolTable.h` - Scope & SymbolTable (완전 구현됨)
- `tests/NameResolutionTest.cpp` - Name Resolution 테스트
- `tests/TypeCheckingTest.cpp` - Type Checking 테스트
- `tests/SemanticSymbolTableTest.cpp` - SymbolTable 테스트

---

## 📚 참고

- **Scope 기반 Symbol Table**: 이미 완전히 구현됨
- **4단계 분석**: analyze() → buildSymbolTable → resolveNames → checkTypes → resolveAmbiguities
- **현재 테스트 통과율**: 41/41 (100%) - 하지만 Scope 격리는 미검증

---

**결론**: SymbolTable 인프라는 완성되어 있지만, `buildSymbolTable()`에서 Scope를 제대로 활용하지 않고 있습니다. 이 문제를 해결하면 P0-1 과제가 완료됩니다.
