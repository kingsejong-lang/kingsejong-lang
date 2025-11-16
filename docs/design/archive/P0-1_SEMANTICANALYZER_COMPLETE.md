# P0-1: SemanticAnalyzer 완성 보고서

**작성일**: 2025-11-16
**작업자**: KingSejong Team
**관련 과제**: P0-1 SemanticAnalyzer 완전 구현
**예상 공수**: 2-3일 → **실제 공수**: 3시간 ⚡

---

## 📋 요약

**buildSymbolTable과 resolveNames를 통합하여 Scope를 일관되게 관리합니다.**

✅ **변수 스코프 격리를 올바르게 구현하여 블록 내 변수가 블록 밖에서 접근 불가능하도록 개선했습니다.**

**Before**: 블록 내 변수가 블록 밖에서도 접근 가능 (Scope 미격리)
**After**: **블록 내 변수는 블록 스코프에만 존재** (Scope 격리 완성) 🎉

**테스트 통과율**: **1220/1220 (100%)** - 회귀 없음!

---

## 🔍 문제 분석

### 기존 문제

**Scope 격리 미작동**:
```javascript
만약 참 {
    정수 x = 10
}
정수 y = x  // ❌ 에러가 발생해야 하지만 통과함!
```

**원인**:
1. `buildSymbolTable()`: Scope를 사용하지 않고 모든 변수를 전역 스코프에 등록
2. `resolveNames()`: Scope를 사용하지만, 변수를 다시 등록하지 않음
3. **불일치**: 두 단계가 각각 별도의 Scope 인스턴스를 생성하여 일관성 부족

**근본 원인**:
```cpp
// Phase 1: buildSymbolTable - Scope 미사용
void buildSymbolTable(Program* program)
{
    for (auto& stmt : program->statements())
    {
        registerSymbolFromStatement(stmt.get());  // 전역 스코프에만 등록
    }
}

// Phase 2: resolveNames - Scope 사용
void resolveNames(Program* program)
{
    // BlockStatement 처리 시 enterScope/exitScope 호출
    // 하지만 심볼은 Phase 1에서 이미 전역에 등록됨
    // 결과: 블록 내 변수도 전역에서 찾아짐
}
```

---

## 🔧 해결 방안

### 통합 접근법: analyzeAndResolve

**Phase 1+2 통합**: buildSymbolTable과 resolveNames를 하나의 pass로 통합

**새로운 메서드**:
- `analyzeAndResolve(Program*)`: 프로그램 전체 분석
- `analyzeAndResolveStatement(const Statement*)`: 문장별 심볼 등록 + 이름 해석
- `analyzeAndResolveExpression(const Expression*)`: 표현식 내 식별자 검증

**핵심 아이디어**:
- 변수 선언: **현재 스코프에 즉시 등록**
- 식별자 참조: **현재 스코프 체인에서 검색**
- 블록/함수: **새 스코프 생성 → 내부 처리 → 스코프 탈출**

---

## 📝 구현 내용

### 1. SemanticAnalyzer.h - 새로운 메서드 선언

```cpp
// Phase 1+2: Symbol Table 구축 및 이름 해석 (통합)
void analyzeAndResolve(ast::Program* program);
void analyzeAndResolveStatement(const ast::Statement* stmt);
void analyzeAndResolveExpression(const ast::Expression* expr);
```

### 2. SemanticAnalyzer.cpp - analyze() 메서드 수정

```cpp
bool SemanticAnalyzer::analyze(Program* program)
{
    // ...

    // Phase 1+2: Symbol Table 구축 및 이름 해석 (통합)
    analyzeAndResolve(program);  // ← 기존 buildSymbolTable + resolveNames 대체

    // Phase 3: 타입 검사
    checkTypes(program);

    // Phase 4: 모호성 해결
    resolveAmbiguities(program);

    return errors_.empty();
}
```

### 3. analyzeAndResolveStatement() - 통합 로직

**변수 선언**:
```cpp
if (auto varDecl = dynamic_cast<const VarDeclaration*>(stmt))
{
    // 변수를 현재 스코프에 등록
    symbolTable_.define(varDecl->varName(), SymbolKind::VARIABLE, varDecl->varType());

    // 초기화 표현식 분석
    if (varDecl->initializer())
    {
        analyzeAndResolveExpression(varDecl->initializer());
    }
}
```

**블록문 - 새 스코프 생성**:
```cpp
else if (auto blockStmt = dynamic_cast<const BlockStatement*>(stmt))
{
    symbolTable_.enterScope();  // 스코프 진입

    for (const auto& s : blockStmt->statements())
    {
        analyzeAndResolveStatement(s.get());  // 내부 문장 처리
    }

    symbolTable_.exitScope();  // 스코프 탈출 (블록 내 변수 파괴)
}
```

**if문 - then/else 각각 새 스코프**:
```cpp
else if (auto ifStmt = dynamic_cast<const IfStatement*>(stmt))
{
    // 조건 표현식 분석
    analyzeAndResolveExpression(ifStmt->condition());

    // then 블록
    if (ifStmt->thenBranch())
    {
        symbolTable_.enterScope();
        analyzeAndResolveStatement(ifStmt->thenBranch());
        symbolTable_.exitScope();
    }

    // else 블록
    if (ifStmt->elseBranch())
    {
        symbolTable_.enterScope();
        analyzeAndResolveStatement(ifStmt->elseBranch());
        symbolTable_.exitScope();
    }
}
```

**범위 for문 - 루프 변수를 블록 스코프에 등록**:
```cpp
else if (auto forStmt = dynamic_cast<const RangeForStatement*>(stmt))
{
    symbolTable_.enterScope();

    // 루프 변수 등록
    symbolTable_.define(forStmt->varName(), SymbolKind::VARIABLE,
                      types::Type::integerType());

    // start, end 표현식 분석
    analyzeAndResolveExpression(forStmt->start());
    analyzeAndResolveExpression(forStmt->end());

    // body 분석
    analyzeAndResolveStatement(forStmt->body());

    symbolTable_.exitScope();
}
```

**함수 선언 - 매개변수를 함수 스코프에 등록**:
```cpp
if (auto funcLit = dynamic_cast<const FunctionLiteral*>(assignStmt->value()))
{
    // 함수 이름을 현재 스코프에 등록
    symbolTable_.define(assignStmt->varName(), SymbolKind::FUNCTION, nullptr);

    // 함수 body를 새로운 스코프에서 분석
    symbolTable_.enterScope();

    // 매개변수를 함수 스코프에 등록
    for (const auto& param : funcLit->parameters())
    {
        symbolTable_.define(param, SymbolKind::VARIABLE, nullptr);
    }

    // 함수 body 분석
    analyzeAndResolveStatement(funcLit->body());

    symbolTable_.exitScope();
}
```

### 4. analyzeAndResolveExpression() - 식별자 검증

```cpp
void SemanticAnalyzer::analyzeAndResolveExpression(const Expression* expr)
{
    // 식별자: 정의되어 있는지 확인
    if (auto ident = dynamic_cast<const Identifier*>(expr))
    {
        if (!symbolTable_.lookup(ident->name()) &&
            !isBuiltinFunction(ident->name()))
        {
            addError("정의되지 않은 식별자: " + ident->name(),
                    expr->location().line, expr->location().column);
        }
    }

    // 이항 표현식: 양쪽 피연산자 분석
    else if (auto binary = dynamic_cast<const BinaryExpression*>(expr))
    {
        analyzeAndResolveExpression(binary->left());
        analyzeAndResolveExpression(binary->right());
    }

    // ... (모든 표현식 타입 처리)
}
```

---

## 📊 테스트 결과

### 1. Scope 격리 검증

**테스트 코드** (`/tmp/scope_test.ksj`):
```javascript
만약 참 {
    정수 x = 10
}
정수 y = x
```

**Before**:
```bash
$ ./build/bin/kingsejong /tmp/scope_test.ksj
(에러 없이 실행됨)
```

**After**:
```bash
$ ./build/bin/kingsejong /tmp/scope_test.ksj
Semantic Error: /tmp/scope_test.ksj:4:13: 정의되지 않은 식별자: x
```

✅ **Scope 격리 정상 작동!**

### 2. 전체 테스트 통과율

```bash
$ ./build/bin/kingsejong_tests
[==========] 1220 tests from 75 test suites ran.
[  PASSED  ] 1220 tests.
```

**결과**: ✅ **1220/1220 (100%)** - 회귀 없음!

---

## 📝 수정된 파일

| 파일 | 변경 내용 | 줄 수 |
|------|-----------|-------|
| `src/semantic/SemanticAnalyzer.h` | 새 메서드 선언 추가 | +15 |
| `src/semantic/SemanticAnalyzer.cpp` | analyzeAndResolve 구현 (통합) | +270 |
| **Total** | **2 files** | **+285 lines** |

**주요 변경**:
- ✅ `analyzeAndResolve()` - buildSymbolTable + resolveNames 통합
- ✅ `analyzeAndResolveStatement()` - 심볼 등록 + 이름 해석
- ✅ `analyzeAndResolveExpression()` - 식별자 검증
- ✅ 모든 블록/함수에서 Scope 일관 관리

---

## 🎯 달성 목표

### ✅ 완료된 목표

1. ✅ **Scope 격리 구현**
   - 블록 내 변수는 블록 밖에서 접근 불가
   - 함수 매개변수는 함수 밖에서 접근 불가
   - 루프 변수는 루프 밖에서 접근 불가

2. ✅ **buildSymbolTable + resolveNames 통합**
   - 단일 pass로 심볼 등록 + 이름 해석
   - Scope를 일관되게 관리
   - 코드 중복 제거

3. ✅ **100% 테스트 통과율 유지** (1220/1220)
   - 회귀 없음
   - 기존 기능 모두 정상 작동

4. ✅ **에러 메시지 품질 향상**
   - Before: 에러 미감지
   - After: "정의되지 않은 식별자" 에러 정확히 보고

---

## 💡 작동 원리

### Scope 생명주기

```
프로그램 시작
  └─ 전역 스코프 (Global Scope)
      ├─ 정수 a = 10  ← 전역 변수
      │
      └─ 만약 참 {
          └─ 새 스코프 (Block Scope)
              ├─ 정수 x = 20  ← 블록 변수 (여기서만 보임)
              │
              └─ 만약 거짓 {
                  └─ 새 스코프 (Nested Block Scope)
                      └─ 정수 y = 30  ← 중첩 블록 변수
              }  ← exitScope() - y 파괴
      }  ← exitScope() - x 파괴
      │
      └─ 정수 b = x  ← ❌ 에러: x는 이미 파괴됨
```

### 식별자 검색 (Lookup)

```
현재 스코프에서 검색
  ↓ 없으면
부모 스코프에서 검색
  ↓ 없으면
부모의 부모 스코프에서 검색
  ↓ ...
  ↓ 최상위까지 없으면
❌ 에러: 정의되지 않은 식별자
```

---

## 🔍 기술적 세부사항

### 함수 스코프

**함수 매개변수**:
```javascript
함수 더하기(a, b) {
    a + b  // ✅ a, b는 함수 스코프에 존재
}
정수 x = a  // ❌ 에러: a는 함수 밖에서 접근 불가
```

**구현**:
```cpp
symbolTable_.enterScope();  // 함수 스코프 진입

// 매개변수 등록
for (const auto& param : funcLit->parameters())
{
    symbolTable_.define(param, SymbolKind::VARIABLE, nullptr);
}

// 함수 body 분석
analyzeAndResolveStatement(funcLit->body());

symbolTable_.exitScope();  // 매개변수 파괴
```

### 루프 변수 스코프

**범위 for문**:
```javascript
i가 1부터 10까지 반복한다 {
    출력(i)  // ✅ i는 루프 스코프에 존재
}
정수 x = i  // ❌ 에러: i는 루프 밖에서 접근 불가
```

**구현**:
```cpp
symbolTable_.enterScope();  // 루프 스코프 진입

// 루프 변수 등록
symbolTable_.define(forStmt->varName(), SymbolKind::VARIABLE,
                  types::Type::integerType());

// 루프 body 분석
analyzeAndResolveStatement(forStmt->body());

symbolTable_.exitScope();  // 루프 변수 파괴
```

---

## 📚 관련 작업

### 완료된 P0 과제

- ✅ P1-1: 형태소 분석기 분리 (2주)
- ✅ P1-2: LL(4) Lookahead (3일)
- ✅ P1-3: 문법 개선 (1일)
- ✅ P0-2: SourceLocation 개선 (2시간)
- ✅ P0-3: ErrorRecovery 개선 (30분)
- ✅ **P0-1: SemanticAnalyzer 완성 (3시간)** ← 현재

### 🎉 P0 과제 완료!

**모든 P0 (최우선) 과제를 완료했습니다!**

---

## 🎊 결론

**P0-1 과제를 성공적으로 완료했습니다!**

### 성과

1. ✅ Scope 격리 완전 구현 - 변수/함수/루프 스코프 올바르게 관리
2. ✅ buildSymbolTable + resolveNames 통합 - 일관된 Scope 관리
3. ✅ 100% 테스트 통과율 유지 (1220/1220)
4. ✅ 에러 메시지 품질 향상 - 스코프 외부 접근 정확히 감지

### 소요 시간

- **예상**: 2-3일
- **실제**: 3시간
- **이유**: Scope 인프라가 이미 완성되어 있었고, 통합만 하면 됨!

### 기술적 성과

- **단일 pass 분석**: buildSymbolTable + resolveNames를 하나로 통합
- **Scope 일관성**: 모든 블록/함수에서 enterScope/exitScope 호출
- **코드 품질**: 중복 코드 제거, 일관된 로직

### 다음 단계

**P0 과제가 모두 완료되었으므로, P1 또는 Phase 6 작업으로 진행할 수 있습니다!**

추천:
- **stdlib 완성** (Phase 6: 현재 55% 완료)
- **LSP 기능 강화** (Phase 3)
- **패턴 매칭 완성** (Phase 5)

---

**상태**: ✅ **완료**
**테스트**: ✅ **1220/1220 (100%)**
**회귀**: ✅ **없음**
**Scope 격리**: ✅ **완전 구현**

**🎊 축하합니다! SemanticAnalyzer 완성! 🎊**
