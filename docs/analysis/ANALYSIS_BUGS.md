# 버그 근본 원인 분석

**문서**: ANALYSIS_BUGS.md
**작성일**: 2025-11-15
**대상**: 알려진 버그 및 구조적 취약점

---

## 목차

1. [알려진 버그](#1-알려진-버그)
2. [구조적 취약점](#2-구조적-취약점)
3. [근본 원인 분석](#3-근본-원인-분석)
4. [버그 예방 전략](#4-버그-예방-전략)

---

## 1. 알려진 버그

### 1.1 괄호 표현식 버그 (KNOWN_ISSUES.md)

#### 증상

**보고된 내용** (KNOWN_ISSUES.md):
```
Parser가 `(a + b) * c` 같은 표현식에서 괄호를 함수 호출로 잘못 인식
비활성화된 테스트: EvaluatorTest.DISABLED_ShouldEvaluateComplexExpression
```

#### 재현 시도

**테스트 코드** (debug_parse.cpp):
```cpp
std::string code = R"(
    정수 a = 5
    정수 b = 3
    정수 c = 2
    (a + b) * c
)";

lexer::Lexer lexer(code);
parser::Parser parser(lexer);
auto program = parser.parseProgram();
```

**결과**:
```
Parser errors: 0
AST dump:
Program {
  정수 a = 5;
  정수 b = 3;
  정수 c = 2;
  ((a + b) * c);  ✅ 올바르게 파싱됨!
}
```

#### 분석

**결론**: **버그가 존재하지 않거나, 이미 수정되었을 가능성**

**가능한 시나리오**:

**A. 과거 버그 (이미 수정됨)**
- Parser.cpp의 `parseGroupedExpression()`과 `parseCallExpression()` 충돌
- 현재 코드는 올바르게 구분

**B. 특정 조건에서만 발생**
```javascript
// 가능한 문제 시나리오
정수 함수 = 5
(함수 + 1) * 2  // "함수"가 함수 호출로 오인될 가능성?
```

**C. ASI와의 상호작용**
```javascript
정수 a = 5
(a + b) * c  // ASI가 세미콜론 삽입 → `a = 5(a + b)` 오해석?
```
- 하지만 현재 ASI 코드는 `hasNewline` 체크 → 개행 없으면 세미콜론 미삽입

#### 권장 조치

1. **DISABLED 테스트 활성화**
   ```cpp
   // tests/EvaluatorTest.cpp
   TEST_F(EvaluatorTest, ShouldEvaluateComplexExpression) {  // DISABLED_ 제거
       // (a + b) * c 테스트
   }
   ```

2. **KNOWN_ISSUES 업데이트**
   - 버그 재현 불가 시 삭제
   - 또는 "수정됨 (확인 필요)" 표시

3. **회귀 테스트 추가**
   ```cpp
   TEST(ParserTest, GroupedExpressionVsCallExpression) {
       // (a + b) * c
       // func(a + b)
       // 둘 다 올바르게 구분하는지 확인
   }
   ```

### 1.2 ASI 관련 버그 (잠재적)

#### 문제 시나리오 1: 연산자가 다음 줄에 있을 때

```javascript
정수 a = 5
+ 3

// 의도: a = 5 + 3
// ASI 결과: a = 5; +3 (에러!)

// 토큰:
[INTEGER] "5"
[SEMICOLON] ";" ← ASI 삽입!
[PLUS] "+"
[INTEGER] "3"
```

**근본 원인**: Lexer.cpp:616-621
```cpp
if (hasNewline &&
    nextCh != '+' && nextCh != '-' && /* ... */) {
    // 세미콜론 삽입
}
```
- `nextCh`는 다음 **문자** 확인
- 하지만 개행 **후** 연산자는 새 표현식으로 간주 (JavaScript와 동일)

**해결책**: 의도적 설계 (JavaScript와 동일)
```javascript
// 회피책: 연산자를 이전 줄 끝에
정수 a = 5 +
    3
```

#### 문제 시나리오 2: 한글 키워드 체크 불가

```javascript
정수 a = 5
부터 1까지  // "부터"가 키워드인데 단일 문자로 체크 불가
```

**근본 원인**: Lexer.cpp:616
```cpp
char nextCh = c;  // 단일 문자 (1바이트)
// "부터"는 3바이트 한글 → 첫 바이트만 확인됨
```

**해결책**: 다음 토큰 타입 확인
```cpp
Token nextToken = peekNextToken();
if (nextToken.type == TokenType::BUTEO || ...) {
    // 세미콜론 삽입 안 함
}
```

---

## 2. 구조적 취약점

### 2.1 Lookahead 한계

#### 문제

**현재**: 1-token lookahead (`peekToken_`)
```cpp
// Parser.h
Token curToken_;
Token peekToken_;  // 단 1개!
```

**제한**:
```javascript
함수 이름(인자)  // "함수" 키워드 확인하려면 2-token 필요
// curToken = "함수"
// peekToken = "이름"
// peek2Token = "(" ← 필요하지만 없음!
```

#### 영향

**회피 가능한 문제**:
- 현재 문법에서는 1-token으로 충분
- 하지만 확장 시 제약

**잠재적 버그**:
```javascript
// 미래 문법
타입 별칭 이름 = 정수  // "타입" + "별칭" 키워드 조합
// 2-token lookahead 필요
```

#### 해결책

**LL(2) 파서**:
```cpp
class Parser {
private:
    Token curToken_;
    Token peekToken_;
    Token peekPeekToken_;  // 추가
};
```

### 2.2 Statement Disambiguation 휴리스틱

#### 문제

**Parser.cpp:238-261**:
```cpp
static bool isLikelyLoopVariable(const std::string& str) {
    if (str == "i" || str == "j" || str == "k" || ...)
        return true;
    // ...
}
```

**휴리스틱 기반 판단** → 엣지 케이스 버그

#### 실패 시나리오

```javascript
// 시나리오 1: 루프 변수이지만 인식 안 됨
count가 1부터 10까지 반복한다
// isLikelyLoopVariable("count") → false
// → parseExpressionStatement() (잘못된 파싱!)

// 시나리오 2: 루프 변수 아닌데 인식됨
i가 중요하다
// isLikelyLoopVariable("i") → true
// → parseRangeForStatement() 시도
// → "부터" 토큰 없음 → 에러!
```

#### 근본 원인

**Parser가 의미론적 판단 수행**:
- "i"가 변수인지는 의미론의 문제
- Parser는 문법만 판단해야 함

#### 해결책

**Semantic Analyzer**:
```cpp
class SemanticAnalyzer {
    SymbolTable symbolTable_;

    void resolveAmbiguity(Statement* stmt) {
        if (auto josaExpr = dynamic_cast<JosaExpression*>(stmt)) {
            std::string name = josaExpr->identifier();
            if (symbolTable_.isDefined(name) &&
                symbolTable_.lookup(name).kind == SymbolKind::VARIABLE) {
                // 루프 변수 가능성
            }
        }
    }
};
```

### 2.3 Error Recovery 부재

#### 문제

**Parser.cpp:763-766**:
```cpp
if (!expectPeek(TokenType::RPAREN)) {
    return nullptr;  // ⚠️ 첫 에러에서 중단!
}
```

**영향**:
- 사용자는 하나의 에러만 확인 가능
- 모든 에러를 고치려면 반복 실행 필요

#### 예시

```javascript
// 소스 코드 (에러 3개)
함수 f(a, b {        // 1. ')' 누락
    c = a + b        // 2. 'c' 미선언
    반환한다(d)       // 3. 'd' 미정의
}

// 현재: 첫 번째 에러만 보고
Error: Expected ')' after parameter list

// 필요: 모든 에러 보고
Error: Expected ')' after parameter list
Error: Undefined variable 'c'
Error: Undefined variable 'd'
```

#### 해결책

**Panic Mode Recovery**:
```cpp
void Parser::synchronize() {
    while (!curTokenIs(TokenType::SEMICOLON) &&
           !curTokenIs(TokenType::EOF_TOKEN)) {
        if (curTokenIs(TokenType::RBRACE)) break;
        nextToken();
    }
}

std::unique_ptr<Expression> Parser::parseExpression(Precedence prec) {
    auto left = callPrefixFunction();
    if (!left) {
        errors_.push_back("Expression error");
        synchronize();  // 복구 시도
        return nullptr; // 계속 파싱
    }
    // ...
}
```

### 2.4 조사 분리 휴리스틱

#### 문제

**Lexer.cpp:162-253 (92줄)**:
- 바이트 길이 기반 조사 분리
- 예외 규칙 하드코딩

#### 실패 시나리오

```javascript
// 시나리오 1: 2글자 명사
거리가 멀다
// "거리가" → "거" + "리가" (잘못된 분리 가능)

// 시나리오 2: 영문 혼용
AB로 이동한다
// "AB로"는 5바이트 (영문 2 + 조사 3)
// 9바이트 미만 → 2글자 조사 체크 안 함
// 분리 안 될 수 있음
```

#### 근본 원인

**사전 없이 휴리스틱 사용**:
```cpp
// Lexer.cpp:234
if (base.length() == 3 && lastChar == "이") {
    return identifier;  // "나이" 보호
}
// 하지만 "거리", "자리" 등 다른 2글자 명사는 보호 안 됨!
```

#### 해결책

**형태소 분석기 + 사전**:
```cpp
class MorphologicalAnalyzer {
    Dictionary dictionary_;  // "나이", "거리", "자리" 등 명사 사전

    std::vector<Morpheme> analyze(const std::string& word) {
        if (dictionary_.isNoun(word)) {
            return {Morpheme{word, MorphemeType::NOUN}};
        }
        // 조사 분리 로직...
    }
};
```

### 2.5 위치 정보 부재

#### 문제

**Token 구조**:
```cpp
struct Token {
    TokenType type;
    std::string literal;
    // line, column 없음!
};
```

**AST 노드**:
```cpp
class Node {
    // line, column 없음!
};
```

#### 영향

**에러 메시지 불친절**:
```
// 현재
Unexpected token: RPAREN

// 필요
Error at line 5, column 12: Unexpected ')'
  |
5 | 함수이름(a, b
  |              ^
  |
Expected: ')' to close function call
```

#### 해결책

**위치 정보 추가**:
```cpp
struct SourceLocation {
    int line;
    int column;
    std::string filename;
};

struct Token {
    TokenType type;
    std::string literal;
    SourceLocation location;  // 추가
};

class Node {
protected:
    SourceLocation location_;  // 추가
};
```

---

## 3. 근본 원인 분석

### 3.1 Semantic Analyzer 부재 (최대 원인)

**문제**:
```
Lexer → Parser → Evaluator/Compiler
         ↑
      여기서 의미론적 판단까지 수행
```

**영향**:
- Parser가 휴리스틱 사용 (`isLikelyLoopVariable`)
- Lexer가 ASI 정책 결정
- 버그 발생 가능성 ↑

**해결책**:
```
Lexer → Parser → Semantic Analyzer → Compiler/Evaluator
                        ↑
                  Symbol Table 구축
                  타입 검사
                  의미론적 모호성 해결
```

### 3.2 책임 분리 미흡

| 컴포넌트 | 적절한 책임 | 현재 책임 | 문제 |
|----------|------------|-----------|------|
| **Lexer** | 토큰화 | 토큰화 + 조사 분리 + ASI | ⚠️ 과도 |
| **Parser** | 문법 분석 | 문법 분석 + Statement 구분 | ⚠️ 과도 |
| **Semantic** | 의미 분석 | (없음) | ❌ 부재 |

### 3.3 테스트 부족

**DISABLED 테스트**:
```cpp
// tests/EvaluatorTest.cpp
TEST_F(EvaluatorTest, DISABLED_ShouldEvaluateComplexExpression) {
    // 비활성화된 테스트 → 버그 존재 의심되지만 확인 안 됨
}
```

**회귀 테스트 부족**:
- 버그 수정 후 재발 방지 테스트 부족
- 엣지 케이스 테스트 부족

---

## 4. 버그 예방 전략

### 4.1 구조적 개선

#### 1. Semantic Analyzer 도입 (최우선)

**효과**:
- ✅ 휴리스틱 제거
- ✅ 의미론적 버그 감소
- ✅ 책임 분리

#### 2. Symbol Table 구현

**효과**:
- ✅ 변수/함수 정확히 구분
- ✅ 스코프 관리
- ✅ 재정의 검사

#### 3. 위치 정보 추가

**효과**:
- ✅ 에러 메시지 개선
- ✅ 디버깅 용이
- ✅ IDE 통합 개선

### 4.2 테스트 전략

#### 1. 회귀 테스트

**버그 발견 시**:
1. 버그 재현 테스트 작성
2. 버그 수정
3. 테스트 통과 확인
4. 테스트를 테스트 스위트에 추가 (재발 방지)

#### 2. Property-Based Testing

**QuickCheck 스타일**:
```cpp
// 모든 표현식은 파싱 후 재평가해도 같은 결과
PROPERTY_TEST(ParseAndEval) {
    auto expr = generateRandomExpression();
    auto ast = parse(expr);
    auto result1 = eval(ast);

    auto reprinted = ast->toString();
    auto ast2 = parse(reprinted);
    auto result2 = eval(ast2);

    ASSERT_EQ(result1, result2);  // 결과 동일해야 함
}
```

#### 3. Fuzzing

**AFL, libFuzzer 활용**:
```cpp
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::string input(reinterpret_cast<const char*>(data), size);

    try {
        lexer::Lexer lexer(input);
        parser::Parser parser(lexer);
        auto program = parser.parseProgram();
        // 크래시 없이 파싱되면 OK
    } catch (...) {
        // 예외는 OK, 크래시는 NO
    }

    return 0;
}
```

### 4.3 코드 리뷰 체크리스트

#### Parser 변경 시

- [ ] 휴리스틱 추가했는가? → Semantic Analyzer로 이동 고려
- [ ] Lookahead 충분한가?
- [ ] Error Recovery 추가했는가?
- [ ] 회귀 테스트 추가했는가?

#### Lexer 변경 시

- [ ] 조사 분리 로직 수정했는가? → 형태소 분석기 사용 고려
- [ ] UTF-8 처리 올바른가?
- [ ] ASI 영향 있는가?
- [ ] 엣지 케이스 테스트했는가?

### 4.4 정적 분석 도구

#### 1. Clang-Tidy

```bash
clang-tidy src/**/*.cpp --checks='*'
```

**효과**:
- 메모리 누수 감지
- 코딩 스타일 검사
- 잠재적 버그 발견

#### 2. AddressSanitizer (ASan)

```cmake
# CMakeLists.txt
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
```

**효과**:
- 버퍼 오버플로 감지
- Use-after-free 감지

#### 3. Valgrind

```bash
valgrind --leak-check=full ./kingsejong test.ksj
```

**효과**:
- 메모리 누수 감지
- 잘못된 메모리 접근 감지

---

## 5. 결론

### 5.1 버그 요약

| 버그 | 상태 | 우선순위 |
|------|------|----------|
| **괄호 표현식** | 재현 불가 (수정됨?) | 🟢 확인 |
| **ASI 엣지 케이스** | 잠재적 | 🟡 중간 |
| **Statement 구분** | 휴리스틱 의존 | 🔴 높음 |
| **조사 분리** | 휴리스틱 의존 | 🔴 높음 |
| **위치 정보 없음** | 구조적 문제 | 🔴 높음 |

### 5.2 근본 원인

**최대 원인**: **Semantic Analyzer 부재**
- Parser가 의미론적 판단 수행
- 휴리스틱 의존 → 버그 가능성

**부차 원인**:
- Error Recovery 없음
- 위치 정보 없음
- 테스트 부족

### 5.3 우선순위

1. **🔴 긴급**: Semantic Analyzer, Symbol Table
2. **🟡 중요**: 위치 정보, Error Recovery
3. **🟢 개선**: 테스트 강화, 정적 분석

---

**문서 끝**

**다음 문서**: [ANALYSIS_IMPROVEMENTS.md](./ANALYSIS_IMPROVEMENTS.md)
**이전 문서**: [ANALYSIS_COMPARISON.md](./ANALYSIS_COMPARISON.md)
