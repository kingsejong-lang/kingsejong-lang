# P0-2: SourceLocation 위치 정보 개선 완료 보고서

**작성일**: 2025-11-16
**작업자**: KingSejong Team
**관련 과제**: P0-2 SourceLocation 위치 정보 추가
**예상 공수**: 2-3일 → **실제 공수**: 2시간 ⚡

---

## 📋 요약

**SourceLocation 인프라가 이미 구현되어 있었으나, 파일명 정보가 누락되어 있었습니다.**

✅ **파일명 정보를 추가하여 에러 메시지 품질을 대폭 개선했습니다.**

**Before**:
```
Semantic Error: :2:13: 정의되지 않은 식별자: z
```

**After**:
```
Semantic Error: /tmp/error_test.ksj:2:13: 정의되지 않은 식별자: z
```

**테스트 결과**: 1217/1220 통과 (99.75%) - 회귀 없음 ✅

---

## 🔍 발견 사항

### 기존 구현 현황

놀랍게도 **SourceLocation 인프라가 이미 완전히 구현**되어 있었습니다:

1. ✅ **SourceLocation 구조체** (`src/ast/SourceLocation.h`)
   - `filename`, `line`, `column` 필드
   - `toString()`, `isValid()`, `update()` 메서드
   - SourceRange 구조체 (시작/끝 범위)

2. ✅ **Token 구조** (`src/lexer/Token.h`)
   - `ast::SourceLocation location` 필드 포함
   - 생성자에서 위치 정보 받음

3. ✅ **Lexer 위치 추적** (`src/lexer/Lexer.cpp`)
   - `currentLine`, `currentColumn` 필드
   - Token 생성 시 위치 정보 설정 (`token.location.update(tokenLine, tokenColumn)`)

4. ✅ **AST 노드 위치** (`src/ast/Node.h`)
   - 모든 Node 클래스: `protected: SourceLocation location_`
   - `location()`, `setLocation()` 메서드

5. ✅ **Parser 위치 설정** (`src/parser/Parser.cpp`)
   - AST 노드 생성 시 `setLocation()` 호출 (58회)

6. ✅ **에러 메시지 출력** (`src/main.cpp`)
   - `err.toString()` 호출하여 위치 정보 포함

### ❌ 유일한 문제: 파일명 누락

- Lexer가 파일명을 받지 않음
- Token에 파일명 정보 없음
- 에러 메시지에 `:2:13:` 만 표시 (파일명 없음)

---

## 🔧 구현 내용

### 1. Lexer에 파일명 지원 추가

**src/lexer/Lexer.h**:
```cpp
class Lexer
{
public:
    explicit Lexer(const std::string& input);
    Lexer(const std::string& input, const std::string& filename);  // ← 추가

private:
    std::string input;
    std::string filename;  // ← 추가
    // ...
};
```

**src/lexer/Lexer.cpp**:
```cpp
// 기존 생성자 (filename 빈 문자열)
Lexer::Lexer(const std::string& input)
    : input(input), filename(""), position(0), ...
{
    readChar();
}

// 새로운 생성자 (filename 포함)
Lexer::Lexer(const std::string& input, const std::string& filename)
    : input(input), filename(filename), position(0), ...
{
    readChar();
}

// Token 생성 시 filename 포함
Token Lexer::nextToken()
{
    int tokenLine = currentLine;
    int tokenColumn = currentColumn;
    Token token;
    // ... (토큰 생성)

    token.location.update(filename, tokenLine, tokenColumn);  // ← 수정
    return token;
}
```

### 2. main.cpp - Lexer 생성 시 파일명 전달

**src/main.cpp**:
```cpp
// Before
Lexer lexer(source);

// After
Lexer lexer(source, filename);
```

### 3. SemanticAnalyzer - 파일명 저장 및 에러에 포함

**src/semantic/SemanticAnalyzer.h**:
```cpp
class SemanticAnalyzer
{
private:
    SymbolTable symbolTable_;
    std::vector<SemanticError> errors_;
    std::unordered_set<std::string> builtins_;
    std::string filename_;  // ← 추가
};
```

**src/semantic/SemanticAnalyzer.cpp**:
```cpp
bool SemanticAnalyzer::analyze(Program* program)
{
    if (!program) return false;

    // Program 노드의 location에서 filename 추출
    filename_ = program->location().filename;  // ← 추가

    buildSymbolTable(program);
    resolveNames(program);
    checkTypes(program);
    resolveAmbiguities(program);

    return errors_.empty();
}

void SemanticAnalyzer::addError(const std::string& message, int line, int column)
{
    errors_.emplace_back(message, line, column, filename_);  // ← 수정
}
```

### 4. Parser - Program 노드에 파일명 설정

**src/parser/Parser.cpp**:
```cpp
std::unique_ptr<Program> Parser::parseProgram()
{
    auto program = std::make_unique<Program>();

    // Before
    // program->setLocation(1, 1);

    // After (첫 토큰의 location 사용, filename 포함)
    program->setLocation(curToken_.location);  // ← 수정

    // ...
}
```

---

## 📊 테스트 결과

### 전체 테스트 통과율

```bash
$ ./build/bin/kingsejong_tests
[==========] 1220 tests from 75 test suites ran.
[  PASSED  ] 1217 tests.
[  FAILED  ] 3 tests
```

**결과**: ✅ **1217/1220 (99.75%)** - 회귀 없음!

**실패한 테스트**:
- `ErrorRecoveryTest.ShouldCollectMultipleErrors`
- `ErrorRecoveryTest.ShouldContinueParsingAfterError`
- `ErrorRecoveryTest.ShouldCollectAllErrorsInOnePass`

→ 기존부터 실패했던 테스트 (P0-3 ErrorRecovery 개선 대상)

### 수동 테스트

**테스트 파일** (`/tmp/error_test.ksj`):
```javascript
정수 x = 10
정수 y = z
```

**결과**:
```bash
$ ./build/bin/kingsejong /tmp/error_test.ksj
Semantic Error: /tmp/error_test.ksj:2:13: 정의되지 않은 식별자: z
```

✅ **파일명, 줄, 열이 모두 정확히 표시됨!**

---

## 📝 수정된 파일

| 파일 | 변경 내용 | 줄 수 |
|------|-----------|-------|
| `src/lexer/Lexer.h` | filename 필드 추가, 생성자 오버로드 | +3 |
| `src/lexer/Lexer.cpp` | 새 생성자 구현, Token 생성 시 filename 전달 | +8 |
| `src/main.cpp` | Lexer 생성 시 filename 전달 | +1 |
| `src/semantic/SemanticAnalyzer.h` | filename_ 필드 추가 | +1 |
| `src/semantic/SemanticAnalyzer.cpp` | filename 저장 및 에러에 포함 | +3 |
| `src/parser/Parser.cpp` | Program location 설정 개선 | +1 |
| **Total** | **6 files** | **+17 lines** |

**매우 간단한 수정**으로 큰 효과!

---

## 🎯 달성 목표

### ✅ 완료된 목표

1. ✅ **에러 메시지에 파일명 포함**
   - Before: `:2:13: 에러 메시지`
   - After: `/path/to/file.ksj:2:13: 에러 메시지`

2. ✅ **회귀 없음**
   - 1217/1220 테스트 통과 유지

3. ✅ **기존 인프라 활용**
   - 이미 구현된 SourceLocation 사용
   - 최소한의 코드 변경

4. ✅ **호환성 유지**
   - 기존 Lexer(input) 생성자 유지
   - 테스트 코드 수정 불필요

---

## 💡 추가 개선 가능 사항 (선택)

### 1. 파싱 에러에도 파일명 표시

현재 Parser 에러는 파일명이 없습니다:
```cpp
// Parser.cpp
void Parser::peekError(TokenType expected)
{
    std::string msg = "expected next token to be " + tokenTypeToString(expected) +
                     ", got " + tokenTypeToString(peekToken_.type) + " instead";
    errors_.push_back(msg);  // ← 위치 정보 없음!
}
```

**개선 방법**:
```cpp
void Parser::peekError(TokenType expected)
{
    std::ostringstream oss;
    oss << peekToken_.location.toString() << ": "
        << "expected " << tokenTypeToString(expected)
        << ", got " << tokenTypeToString(peekToken_.type);
    errors_.push_back(oss.str());
}
```

### 2. REPL에서 위치 정보 표시

REPL 모드에서도 에러 위치를 표시하면 유용합니다:
```
> 정수 x = y
<stdin>:1:11: 정의되지 않은 식별자: y
```

### 3. 컬러 출력

에러 메시지에 색상 추가:
```bash
error_test.ksj:2:13: error: 정의되지 않은 식별자: z
   ^^^^^^^^^^^^^  ^^^^^  ← 빨간색
```

---

## 📚 관련 작업

### 완료된 P0 과제

- ✅ P1-1: 형태소 분석기 분리 (2주)
- ✅ P1-2: LL(4) Lookahead (3일)
- ✅ P1-3: 문법 개선 (1일)
- ✅ **P0-2: SourceLocation 개선 (2시간)** ← 현재

### 다음 P0 과제

- ⏳ **P0-3: ErrorRecovery 개선** (3-4일)
  - Panic Mode Recovery
  - 여러 에러 한 번에 보고
  - 동기화 토큰 정의
  - 현재 실패 중인 3개 테스트 해결

- ⏳ **P0-1: SemanticAnalyzer 완성** (2-3일)
  - buildSymbolTable + resolveNames 통합
  - Scope 격리 올바르게 구현

---

## 🎉 결론

**P0-2 과제를 매우 빠르게 완료했습니다!**

### 성과

1. ✅ 파일명 정보 추가로 에러 메시지 품질 향상
2. ✅ 17줄 변경으로 큰 효과
3. ✅ 회귀 없음 (1217/1220 유지)
4. ✅ 기존 인프라 활용 (재발명 없음)

### 소요 시간

- **예상**: 2-3일
- **실제**: 2시간
- **이유**: 인프라가 이미 완성되어 있었음!

### 다음 단계

**추천**: P0-3 ErrorRecovery 개선으로 진행

- 100% 테스트 통과율 달성 (1220/1220)
- 사용자 경험 대폭 개선
- SourceLocation 활용

---

**상태**: ✅ **완료**
**테스트**: ✅ **1217/1220 (99.75%)**
**회귀**: ✅ **없음**
