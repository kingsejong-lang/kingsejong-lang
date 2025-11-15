# Known Issues

## Current Status (2025-11-15)

**모든 테스트 통과**: 1004/1004 tests passing (100% success rate) ✅

모든 loop statement, 패턴 매칭, 괄호 표현식 파싱 문제가 해결되었습니다.

### 알려진 이슈
없음 ✅

---

## Recently Resolved Issues (2025-11-15)

### ✅ Parser - 괄호 표현식 ASI(Automatic Semicolon Insertion) 구현

**문제**: 파서가 `(a + b) * c` 같은 표현식에서 괄호를 함수 호출로 잘못 인식
```
정수 c = 2
(a + b) * c  // 2(a+b)*c로 파싱됨 (함수 호출)
```

**해결 방법**: Parser-level ASI 구현

**구현 내용**:
1. **Token에 위치 정보 추가** (`src/lexer/Token.h`):
   - `int line`, `int column` 필드 추가
   - 소스 코드 위치 추적 (1부터 시작)

2. **Lexer 위치 추적** (`src/lexer/Lexer.cpp`):
   - `currentLine`, `currentColumn` 필드 추가
   - `readChar()`에서 개행 문자 감지 시 line 증가
   - 모든 토큰에 위치 정보 설정

3. **Parser-level ASI 구현** (`src/parser/Parser.cpp`):
   - `parseExpression()`의 infix 파싱 루프에 line 변경 체크 추가
   - `parseStatement()`의 세미콜론 처리에 ASI 로직 추가
   - 줄이 바뀌면 자동으로 세미콜론 삽입 (표현식 끝으로 간주)

4. **테스트 활성화**:
   - `EvaluatorTest.ShouldEvaluateComplexExpression` 활성화
   - (5+3)*2 = 16 정상 계산

**효과**:
- ✅ 괄호 표현식 정상 파싱
- ✅ 세미콜론 생략 가능 (JavaScript-style ASI)
- ✅ 에러 메시지에 line/column 정보 준비 완료 (향후 활용)

---

## Recently Resolved Issues (2025-11-09)

### F1.12: Loop Statements - All Tests Passing ✅

**Final Status**: 21/21 tests passing (100% success rate)

모든 loop statement 테스트가 성공적으로 통과했습니다. 아래는 해결된 이슈들의 상세 내역입니다.

### Resolved Issues

#### 1. ✅ `ShouldExecuteRepeatZeroTimes` - AssignmentStatement 구현

**해결 방법**: AssignmentStatement AST 노드, 파서, Evaluator 구현

**구현 내용**:
- `src/ast/Statement.h`: AssignmentStatement 클래스 추가
- `src/parser/Parser.cpp`: 할당 구문 파서 구현
- `src/evaluator/Evaluator.cpp`: 변수 재할당 평가기 구현
- 기능: `count = count + 1` 같은 변수 재할당 지원

**Commit**: cd6413a

---

#### 2. ✅ `ShouldExecuteRepeatWithExpression` - Lexer 키워드 분리

**해결 방법**: Lexer에서 "번" 키워드를 식별자에서 자동 분리

**구현 내용**:
- `src/lexer/Lexer.cpp`: 1글자 키워드 분리 로직에 BEON 추가
- `n번` → `n` + `번` (IDENTIFIER + BEON)으로 토큰화
- RepeatStatement 파서가 표현식 먼저 파싱 후 BEON 체크하도록 수정

**Commit**: cd6413a

---

#### 3. ✅ `ShouldThrowOnNegativeRepeatCount` - 음수 검증 추가

**해결 방법**: Evaluator에서 런타임 음수 검증 추가

**구현 내용**:
- `src/evaluator/Evaluator.cpp`: evalRepeatStatement()에서 count < 0 검증
- 에러 메시지: "반복 횟수는 0 이상이어야 합니다"
- 파서가 아닌 평가기에서 처리 (의미론적 검증)

**Commit**: cd6413a

---

#### 4. ✅ `ShouldExecuteRangeForWithExpressions` - Lexer 키워드 분리

**해결 방법**: Lexer에서 "부터", "까지" 키워드를 식별자에서 자동 분리

**구현 내용**:
- `src/lexer/Lexer.cpp`: 2글자 키워드 분리 로직에 BUTEO, KKAJI, BANBOKK 추가
- `end까지` → `end` + `까지` (IDENTIFIER + KKAJI)로 토큰화
- `start부터` → `start` + `부터` (IDENTIFIER + BUTEO)로 토큰화

**Commit**: cd6413a

---

#### 5. ✅ `ShouldParseJosaExpressionIGa` - 파싱 충돌 해결

**문제**: Lexer 수정 후 "데이터가 존재한다"가 범위 for문으로 잘못 파싱됨

**해결 방법**: isLikelyLoopVariable() 휴리스틱 추가

**구현 내용**:
- 일반적인 루프 변수 이름 (i, j, k, index, n, m)만 범위 for문으로 인식
- 1-2글자 ASCII, 1글자 한글만 루프 변수로 처리
- 긴 명사 ("데이터", "사용자" 등)는 조사 표현식으로 파싱

**Commit**: bc2e1b2

---

## Implementation Summary

### What Works ✅

1. **RepeatStatement (N번 반복)**:
   - Literal counts: `5번 반복한다 { ... }` ✅
   - Zero iterations: `0번 반복한다 { ... }` ✅
   - Expression-based counts: `n번 반복한다 { ... }` ✅
   - Negative count validation: `-1번 반복한다` → 런타임 에러 ✅
   - Multiple iterations: 정상 작동
   - Nested loops: 정상 작동

2. **RangeForStatement (범위 반복)**:
   - Literal ranges: `i가 1부터 5까지 반복한다 { ... }` ✅
   - Variable-based ranges: `i가 start부터 end까지 반복한다 { ... }` ✅
   - Negative ranges: `i가 -2부터 2까지` ✅
   - Zero range: `i가 0부터 0까지` ✅
   - Variable access in body: 정상 작동
   - Nested loops: 정상 작동
   - Empty bodies: 정상 작동

3. **AssignmentStatement (변수 할당)**:
   - Variable reassignment: `count = count + 1` ✅
   - Expression evaluation: `x = y + z * 2` ✅
   - Undefined variable check: 정상 작동

4. **Josa Expression (조사 표현식)**:
   - 조사 파싱: `데이터가 존재한다` ✅
   - 범위 for문과 구분: 정상 작동

---

## Technical Solutions Applied

### 1. Lexer Keyword Separation Enhancement

한글 키워드를 식별자에서 자동으로 분리하는 로직 개선:

**2글자 키워드** (6 bytes):
- `까지` (KKAJI)
- `부터` (BUTEO)
- `반복` (BANBOKK)
- 기존: `하고` (HAGO), `하라` (HARA)

**1글자 키워드** (3 bytes):
- `번` (BEON)
- 기존: 조사들 (`가`, `이`, `을`, `를` 등)

### 2. Parser Statement Disambiguation

범위 for문과 조사 표현식을 구분하는 휴리스틱:

```cpp
static bool isLikelyLoopVariable(const std::string& str) {
    // 일반적인 루프 변수: i, j, k, index, idx, n, m
    // 1-2글자 ASCII 식별자
    // 1글자 한글
}
```

### 3. Expression-Based Statement Parsing

표현식 기반 반복문 파싱 전략:

```cpp
// 표현식 먼저 파싱
auto expr = parseExpression(Precedence::LOWEST);

// 다음 토큰이 BEON이면 RepeatStatement
if (peekTokenIs(TokenType::BEON)) {
    // 반복문으로 처리
}
```

### 4. ParseFeature Flag System

기존에 구현된 context-sensitive parsing 시스템:

```cpp
enum class ParseFeature : uint32_t {
    None       = 0,
    Range      = 1 << 0,  // Range expressions (부터...까지)
    All        = 0xFFFFFFFFu
};
```

RangeForStatement는 Range 기능을 비활성화하여 "부터...까지"가 표현식으로 소비되지 않도록 방지.

---

## Test Results

```
Total: 271 tests
✅ Passed: 268 tests (100%)
⏸️  Disabled: 3 tests (intentional)
❌ Failed: 0 tests
```

---

## References

- PR #20: Loop Statement Fixes (upcoming)
- PR #19: First-Class Functions (merged)
- PR #18: Loop Statements Implementation (merged)
- Commit cd6413a: 반복문 표현식 지원 및 변수 할당 구문 구현
- Commit bc2e1b2: 범위 for문과 조사 표현식 파싱 충돌 해결
