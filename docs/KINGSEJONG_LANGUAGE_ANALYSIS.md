# KingSejong 언어 분석 보고서

> 작성일: 2025-11-16
> 분석 대상: KingSejong Programming Language v0.3.1
> 비교 대상: Python, JavaScript, Lua

---

## 목차

1. [개요](#개요)
2. [KingSejong 문법 분석](#kingsejong-문법-분석)
3. [파서 구조 분석](#파서-구조-분석)
4. [변수 스코핑 시스템](#변수-스코핑-시스템)
5. [주요 언어와의 비교](#주요-언어와의-비교)
6. [핵심 문제점](#핵심-문제점)
7. [개선 방안](#개선-방안)
8. [로드맵 제안](#로드맵-제안)

---

## 개요

### 분석 목적
KingSejong 언어의 문법 구조, 파서 구현, 변수 스코핑 시스템을 분석하고, 주요 인터프리터 언어(Lua, JavaScript, Python)와 비교하여 개선점을 도출합니다.

### 주요 발견사항
- ✅ **강점**: 한글 키워드 기반의 직관적 문법, Josa 표현식, 패턴 매칭
- ⚠️ **약점**: 파서 휴리스틱 의존, 변수 스코프 버그, 예외 처리 부재
- 📊 **테스트**: 1004/1004 테스트 100% 통과

---

## KingSejong 문법 분석

### 1.1 언어 기본 구조

**파일 위치**:
- Lexer: `/src/lexer/Lexer.cpp`, `/src/lexer/Token.h`
- Parser: `/src/parser/Parser.cpp/h`
- Evaluator: `/src/evaluator/Evaluator.cpp/h`
- Environment: `/src/evaluator/Environment.h`

**파싱 알고리즘**: Pratt Parsing (Top-Down Operator Precedence)

**룩어헤드**: LL(4) - 최대 4개 토큰 선행 확인 가능

### 1.2 토큰 타입 (70+ 종류)

#### 기본 타입
```
ILLEGAL, EOF_TOKEN, IDENTIFIER, INTEGER, FLOAT, STRING
```

#### 연산자
```
산술: PLUS(+), MINUS(-), ASTERISK(*), SLASH(/), PERCENT(%)
비교: EQ(==), NOT_EQ(!=), LT(<), GT(>), LE(<=), GE(>=)
논리: AND(&&), OR(||), NOT(!)
할당: ASSIGN(=)
```

#### 조사 (Josa Particles)
```
JOSA_EUL/REUL (을/를) - 목적격
JOSA_I/GA (이/가) - 주격
JOSA_EUN/NEUN (은/는) - 보조사
JOSA_UI (의) - 소유격
JOSA_RO/EURO (로/으로) - 방향
JOSA_ESO (에서) - 출처
JOSA_E (에) - 위치
```

#### 제어 키워드
```
조건문: MANYAK (만약), ANIMYEON (아니면)
반복문: BEON (번), BANBOKHANDA (반복한다)
함수: HAMSU (함수), BANHWAN (반환)
모듈: GAJYEOOGI (가져오기)
```

#### 타입 키워드
```
JEONGSU (정수)
SILSU (실수)
MUNJA (문자)
MUNJAYEOL (문자열)
NONLI (논리)
BAEYEOL (배열)
```

#### 범위 키워드
```
BUTEO (부터) - from
KKAJI (까지) - to (inclusive)
MIMAN (미만) - less than (exclusive)
CHOGA (초과) - greater than (exclusive)
IHA (이하) - less or equal (inclusive)
ISANG (이상) - greater or equal (inclusive)
```

### 1.3 문법 구조

#### 변수 선언
```kingsejong
# 타입 명시
정수 나이 = 25
실수 키 = 175.5
문자열 이름 = "홍길동"

# 타입 추론
점수 = 95
메시지 = "안녕"
```

#### 제어 구조
```kingsejong
# 조건문
만약 (점수 >= 90) {
    출력("A 학점")
} 아니면 만약 (점수 >= 80) {
    출력("B 학점")
} 아니면 {
    출력("C 학점")
}

# N회 반복
10번 반복한다 {
    출력("반복")
}

# 범위 반복
i가 1부터 10까지 반복한다 {
    출력(i)
}
```

#### 함수
```kingsejong
# 함수 리터럴
더하기 = 함수(a, b) {
    반환 a + b
}

# 함수 선언 (문법 설탕)
함수 더하기(a, b) {
    반환 a + b
}

# 고차 함수
적용 = 함수(함수, 값) {
    반환 함수(값)
}
```

#### 배열
```kingsejong
# 배열 리터럴
숫자들 = [1, 2, 3, 4, 5]
혼합 = [1, "문자", 참]

# 인덱싱
첫번째 = 숫자들[0]

# 슬라이싱
일부 = 숫자들[2부터 5까지]
```

#### Josa 표현식 (독특한 기능)
```kingsejong
# 조사 기반 메서드 체이닝
배열을 정렬한다
숫자가 크다
데이터는 중요하다

# 함수형 배열 메서드
짝수 = 숫자들을 걸러낸다(함수(x) { 반환 x % 2 == 0 })
제곱들 = 숫자들을 변환한다(함수(x) { 반환 x * x })
합계 = 숫자들을 축약한다(0, 함수(누적, 현재) { 반환 누적 + 현재 })
```

#### 패턴 매칭
```kingsejong
값에 대해 {
    1 -> "하나"
    2 -> "둘"
    x when x > 10 -> "큰 수"
    _ -> "기타"
}
```

### 1.4 연산자 우선순위

```cpp
enum Precedence {
    LOWEST = 0,        // 최저
    ASSIGN,            // = (할당)
    RANGE,             // 부터...까지 (범위)
    OR,                // || (논리 OR)
    AND,               // && (논리 AND)
    EQUALS,            // ==, != (동등)
    LESSGREATER,       // <, >, <=, >= (비교)
    SUM,               // +, - (덧셈/뺄셈)
    PRODUCT,           // *, /, % (곱셈/나눗셈)
    PREFIX,            // -X, !X (단항)
    CALL,              // func() (함수 호출)
    INDEX              // arr[i] (배열 인덱싱)
};
```

### 1.5 주석 및 세미콜론

**주석**:
- 단일 라인만 지원: `# 주석 내용`
- 블록 주석 없음

**세미콜론**:
- 선택적 (ASI - Automatic Semicolon Insertion 구현)
- 줄바꿈 기반 문장 종료

**문자열**:
- 작은따옴표: `'문자열'`
- 큰따옴표: `"문자열"`
- 이스케이프: `\n`, `\t`, `\"`, `\\`

---

## 파서 구조 분석

### 2.1 Pratt Parsing 구조

**파싱 방식**: Top-Down Operator Precedence Parsing

**핵심 메커니즘**:
```cpp
std::unique_ptr<Expression> Parser::parseExpression(
    Precedence precedence, ParseFeature features)
{
    // 1. Prefix 파서 호출
    auto prefixFn = prefixParseFns_[curToken_.type];
    auto leftExpr = prefixFn();

    // 2. Infix 파서 반복 (우선순위 기반)
    while (!peekTokenIs(SEMICOLON) &&
           peekPrecedence() > precedence)
    {
        auto infixFn = infixParseFns_[peekToken_.type];
        nextToken();
        leftExpr = infixFn(std::move(leftExpr));
    }

    return leftExpr;
}
```

### 2.2 등록된 파싱 함수

**Prefix 함수**:
- 식별자, 리터럴, 단항 연산자
- 괄호 표현식, 배열 리터럴
- 함수 리터럴

**Infix 함수**:
- 이항 연산자 (+, -, *, /, %, ==, !=, etc.)
- 함수 호출 `()`
- 배열 인덱싱 `[]`
- Josa 표현식 (조사 토큰)
- 범위 표현식 (부터, 까지, etc.)

### 2.3 LL(4) 룩어헤드 구조

```cpp
// Parser.h
Token curToken_;   // 현재 토큰
Token peekToken_;  // peek1
Token peek2_;      // peek2
Token peek3_;      // peek3
Token peek4_;      // peek4
```

**용도**: 범위 반복문 패턴 감지
```kingsejong
i가 1부터 10까지 반복한다
^  ^   ^
|  |   |
|  |   peek3/peek4에서 '부터' 감지
|  peek1에서 '가' 감지
curToken에서 'i' 감지
```

### 2.4 에러 복구 (Panic Mode)

**동기화 지점**:
- 세미콜론 (`;`)
- 문장 시작 키워드 (`정수`, `함수`, `만약`, `반환` 등)
- 블록 경계 (`{`, `}`)
- EOF 토큰

**동작**: 첫 에러에서 중단하지 않고 계속 파싱하여 여러 에러 수집

---

## 변수 스코핑 시스템

### 3.1 Environment 클래스 구조

**파일**: `/src/evaluator/Environment.h`

```cpp
class Environment : public std::enable_shared_from_this<Environment> {
private:
    std::unordered_map<std::string, Value> store_;  // 현재 스코프 변수
    std::shared_ptr<Environment> outer_;             // 부모 스코프
};
```

### 3.2 스코프 체인

**변수 설정** (`set()`):
- **현재 스코프에만** 저장
- 부모 스코프 업데이트 **안 함**
- 섀도잉 허용 (자식이 부모 변수를 숨김)

**변수 조회** (`get()`):
- **재귀적 탐색**: 현재 → 부모 → 조부모 → ...
- 렉시컬 스코핑 (정적 스코프)
- 못 찾으면 `std::runtime_error`

### 3.3 스코프 생성 시점

1. **전역 스코프**: `Evaluator` 생성자
2. **함수 호출**: `evalCallExpression()`
3. **블록 문장**: `evalBlockStatement()`
4. **반복 문장**: `evalRepeatStatement()`

### 3.4 클로저 구현

```cpp
class Value {
    std::shared_ptr<Environment> closure_env_;  // 캡처된 환경
};

// 함수 생성 시
func_value.closure_env_ = env_;  // 현재 환경 캡처

// 함수 호출 시
auto callEnv = func_value.closure_env_->createEnclosed();
// 캡처된 환경의 자식 스코프 생성
```

**예제**:
```kingsejong
카운터_생성 = 함수() {
    정수 count = 0                # count는 클로저 환경에 저장
    반환 함수() {                 # 내부 함수가 환경 캡처
        count = count + 1         # 클로저 환경의 count 접근
        반환 count
    }
}
```

### 3.5 스코프 문제점

**문제**: 재할당 시 부모 스코프 업데이트 안 됨

```kingsejong
정수 x = 10                       # 전역 스코프: x = 10
만약 (참) {
    x = 20                         # ❌ 블록 스코프에 새 x 생성
    출력(x)                        # 20 (블록 스코프의 x)
}
출력(x)                            # 10 (전역 x는 그대로)
```

**원인**: `evalAssignmentStatement()`가 현재 스코프에만 `set()` 호출

---

## 주요 언어와의 비교

### 4.1 변수 스코핑 비교

| 언어 | 스코핑 방식 | 블록 스코프 | 재할당 시 부모 업데이트 | 클로저 |
|------|------------|------------|---------------------|--------|
| **Python** | Lexical | ✅ 함수 단위 | ✅ `nonlocal` 필요 | ✅ |
| **JavaScript** | Lexical | ✅ `let`/`const` | ✅ 자동 | ✅ |
| **Lua** | Lexical | ✅ `local` 명시 | ✅ 자동 | ✅ |
| **KingSejong** | Lexical | ⚠️ 부분적 | ❌ **버그** | ✅ |

#### Python 예제
```python
x = 10
def func():
    nonlocal x      # 부모 스코프 참조 명시
    x = 20
func()
print(x)            # 20
```

#### JavaScript 예제
```javascript
let x = 10;
if (true) {
    x = 20;         // 자동으로 외부 x 업데이트
}
console.log(x);     // 20
```

#### Lua 예제
```lua
x = 10              -- 전역
function func()
    x = 20          -- 자동으로 전역 x 업데이트
end
func()
print(x)            -- 20
```

### 4.2 문법 명확성 비교

| 특징 | Python | JavaScript | Lua | KingSejong |
|------|--------|------------|-----|-----------|
| **문장 종료** | 줄바꿈 | 세미콜론 (선택) | 줄바꿈 | 줄바꿈 (ASI) |
| **블록 구분** | 들여쓰기 | `{}` | `do...end` | `{}` |
| **변수 선언** | 암묵적 | `let`/`const`/`var` | 암묵적 | 타입 (선택) |
| **휴리스틱 의존** | ❌ | ⚠️ ASI만 | ❌ | ⚠️ **높음** |

### 4.3 언어 기능 비교표

| 기능 | Python | JavaScript | Lua | KingSejong | 우선순위 |
|------|--------|------------|-----|-----------|---------|
| **클로저** | ✅ | ✅ | ✅ | ✅ | - |
| **고차 함수** | ✅ | ✅ | ✅ | ✅ | - |
| **배열/리스트** | ✅ | ✅ | ✅ | ✅ | - |
| **딕셔너리/객체** | ✅ dict | ✅ object | ✅ table | ❌ | 🔴 높음 |
| **클래스/OOP** | ✅ class | ✅ class | ✅ metatable | ❌ | 🟡 중간 |
| **모듈 시스템** | ✅ import | ✅ import | ✅ require | ✅ 가져오기 | - |
| **예외 처리** | ✅ try/except | ✅ try/catch | ✅ pcall | ❌ | 🔴 높음 |
| **문자열 보간** | ✅ f-string | ✅ template | ❌ | ❌ | 🟡 중간 |
| **정규표현식** | ✅ re | ✅ RegExp | ✅ patterns | ❌ | 🟡 중간 |
| **비동기** | ✅ async/await | ✅ async/await | ✅ coroutine | ❌ | 🟢 낮음 |
| **제너레이터** | ✅ yield | ✅ function* | ⚠️ coroutine | ❌ | 🟢 낮음 |
| **Spread/Unpack** | ✅ *args | ✅ ...rest | ✅ unpack | ❌ | 🟡 중간 |
| **Destructuring** | ✅ a,b=[1,2] | ✅ [a,b]=arr | ❌ | ❌ | 🟡 중간 |
| **패턴 매칭** | ✅ match | ❌ | ❌ | ✅ `에 대해` | - |

### 4.4 부족한 핵심 기능 상세

#### 딕셔너리/맵 (최우선)

**Python**:
```python
person = {
    "이름": "홍길동",
    "나이": 25
}
print(person["이름"])
```

**JavaScript**:
```javascript
const person = {
    이름: "홍길동",
    나이: 25
};
console.log(person.이름);
```

**KingSejong (현재 불가능)**:
```kingsejong
# ❌ 딕셔너리 문법 없음
# 배열만 가능
사람 = ["홍길동", 25]  # 인덱스로만 접근
```

#### 예외 처리 (최우선)

**Python**:
```python
try:
    result = 10 / 0
except ZeroDivisionError as e:
    print(f"오류: {e}")
finally:
    print("정리")
```

**JavaScript**:
```javascript
try {
    let result = 10 / 0;
} catch (error) {
    console.log(`오류: ${error}`);
} finally {
    console.log("정리");
}
```

**KingSejong (현재 불가능)**:
```kingsejong
# ❌ 예외 처리 없음
결과 = 10 / 0  # 에러 발생 → 프로그램 종료
```

#### 배열 연산자

**Python**:
```python
arr1 = [1, 2, 3]
arr2 = [4, 5, 6]
combined = arr1 + arr2  # [1, 2, 3, 4, 5, 6]
```

**KingSejong (현재 불가능)**:
```kingsejong
배열1 = [1, 2, 3]
배열2 = [4, 5, 6]
# ❌ 배열1 + 배열2 지원 안 됨
```

---

## 핵심 문제점

### 5.1 파서 휴리스틱 의존 문제

**위치**: `Parser.cpp:341-365`

**문제점**:
```cpp
bool isLikelyLoopVariable(const std::string& name) const {
    // 1-2글자 ASCII만 루프 변수로 인식
    if (name.length() <= 2 && isalpha(name[0])) return true;

    // 특정 이름만 인식
    if (name == "index" || name == "idx" || name == "i" || name == "j")
        return true;

    // 한글 1글자만 허용
    if (isKoreanChar(name) && koreanCharCount(name) == 1)
        return true;

    return false;  // ❌ 긴 변수명 거부
}
```

**결과**:
```kingsejong
# ✅ 작동
i가 1부터 10까지 { ... }
n가 0부터 5까지 { ... }

# ❌ 실패
데이터가 1부터 10까지 { ... }      # '데이터'는 너무 길어서 파싱 실패
counter가 0부터 100까지 { ... }    # 'counter'는 너무 길어서 파싱 실패
```

**근본 원인**:
- Semantic Analyzer 단계 부재
- 파싱과 의미 분석이 혼합됨
- 문법 규칙이 코드에 임베디드 (명시적 BNF/PEG 문법 없음)

**참고**: `KNOWN_ISSUES.md:11-36`

### 5.2 변수 스코프 버그

**위치**: `Environment.h`, `Evaluator.cpp`

**문제점**:
```kingsejong
정수 x = 10                    # 전역: x = 10
만약 (참) {
    x = 20                      # 의도: 전역 x를 20으로 변경
                                # 실제: 블록에 새 x 생성
    출력(x)                     # 20 (블록의 x)
}
출력(x)                         # 10 (전역 x는 그대로!)
```

**원인**:
```cpp
// evalAssignmentStatement()
void Evaluator::evalAssignmentStatement(AssignmentStatement* stmt) {
    auto value = eval(stmt->value);
    env_->set(stmt->name->value, value);  // ❌ 현재 스코프에만 set
    // 부모 스코프 탐색 안 함!
}
```

**올바른 동작** (Python/JS/Lua):
1. 현재 스코프에서 변수 찾기
2. 없으면 부모 스코프 탐색
3. 찾으면 해당 스코프에서 업데이트
4. 끝까지 못 찾으면 현재 스코프에 새로 생성

### 5.3 예외 처리 부재

**현재 상황**:
- `try`/`catch` 문법 없음
- 런타임 에러 시 `std::runtime_error` throw → 프로그램 종료
- 에러 복구 불가능

**영향**:
- 실용적 프로그램 작성 어려움
- 사용자 입력 검증 불가
- 파일 I/O 에러 처리 불가

### 5.4 타입 시스템 모호성

**문제점**:
```kingsejong
# 타입 키워드의 3가지 의미

# 1. 타입 선언
정수 x = 10

# 2. 함수 호출 (타입 변환)
정수(42.5)

# 3. 식별자 (Josa 표현식)
정수를 출력한다
```

**원인**:
- 타입 키워드가 예약어가 아님
- 컨텍스트에 따라 다르게 해석
- 파서가 다음 토큰으로 구분

**개선 필요**:
- 타입 체크 구현 (런타임에 타입 검증)
- 타입 변환 함수 분리 (`to_int`, `to_float` 등)

### 5.5 Semantic Analyzer 부재

**문제**: 파서와 의미 분석이 분리되지 않음

**표준 컴파일러 구조**:
```
소스 코드
  ↓
[Lexer] → 토큰
  ↓
[Parser] → AST (문법만 검사)
  ↓
[Semantic Analyzer] → 의미 검증 (타입, 스코프, 이름 해결)
  ↓
[Evaluator/Code Generator]
```

**KingSejong 현재 구조**:
```
소스 코드
  ↓
[Lexer] → 토큰
  ↓
[Parser + 휴리스틱] → AST (문법 + 일부 의미 분석)
  ↓
[Evaluator]
```

**개선 필요**:
- Semantic Analyzer 단계 추가
- Symbol Table 구현
- 타입 체크 분리
- 이름 해결 (Name Resolution) 단계

**참고**: `KNOWN_ISSUES.md:29-36`

---

## 개선 방안

### 6.1 Semantic Analyzer 구현 (P0 - 최우선)

**목표**: 파서와 의미 분석 분리

**구현 내용**:

1. **Symbol Table 구현**
   ```cpp
   class SymbolTable {
       std::unordered_map<std::string, Symbol> symbols_;
       std::shared_ptr<SymbolTable> parent_;
   public:
       void define(const std::string& name, SymbolType type);
       Symbol* resolve(const std::string& name);  // 스코프 체인 탐색
       bool isDefinedInCurrentScope(const std::string& name);
   };
   ```

2. **AST Visitor 패턴**
   ```cpp
   class SemanticAnalyzer : public ASTVisitor {
   public:
       void analyze(Program* program);
       void visit(VarDeclaration* node) override;
       void visit(AssignmentStatement* node) override;
       void visit(RangeForStatement* node) override;
       // ...
   };
   ```

3. **휴리스틱 제거**
   - `isLikelyLoopVariable()` 함수 삭제
   - 범위 반복문 파싱을 순수 문법 규칙으로 변경
   - Symbol Table에서 변수 존재 여부 확인

**예상 공수**: 2-3주

**파일 추가**:
- `src/semantic/SemanticAnalyzer.h/cpp`
- `src/semantic/SymbolTable.h/cpp`
- `src/semantic/Symbol.h`

### 6.2 변수 스코프 수정 (P0 - 최우선)

**옵션 1: 자동 탐색 (JavaScript 스타일)**

```cpp
// Environment::setWithLookup() - 새 메서드
void Environment::setWithLookup(const std::string& name, const Value& value) {
    // 현재 스코프에서 찾기
    if (store_.find(name) != store_.end()) {
        store_[name] = value;
        return;
    }

    // 부모 스코프에서 재귀적으로 찾기
    if (outer_ != nullptr && outer_->existsInChain(name)) {
        outer_->setWithLookup(name, value);
        return;
    }

    // 못 찾으면 현재 스코프에 새로 생성
    store_[name] = value;
}
```

**옵션 2: 키워드 명시 (Python 스타일)**

```kingsejong
x = 10
만약 (참) {
    외부 x         # 외부 스코프 변수 참조 명시
    x = 20        # 전역 x 업데이트
}

만약 (참) {
    지역 y = 30   # 새 지역 변수 명시
}
```

**권장**: 옵션 1 (자동 탐색) - 더 직관적

**예상 공수**: 1주

### 6.3 예외 처리 시스템 (P0 - 최우선)

**문법 설계**:
```kingsejong
시도 {
    결과 = 10 / 0
} 오류 (e) {
    출력("에러 발생:", e)
} 마지막 {
    출력("정리 작업")
}
```

**구현 요소**:

1. **키워드 추가**
   - `시도` (TRY)
   - `오류` (CATCH)
   - `마지막` (FINALLY)
   - `던지다` (THROW)

2. **AST 노드**
   ```cpp
   struct TryStatement : Statement {
       std::unique_ptr<BlockStatement> try_block;
       std::string error_variable;
       std::unique_ptr<BlockStatement> catch_block;
       std::unique_ptr<BlockStatement> finally_block;
   };
   ```

3. **Error Value 타입**
   ```cpp
   class Value {
       // ...
       ValueType type_;  // 추가: ERROR
       std::string error_message_;
   };
   ```

**예상 공수**: 2주

### 6.4 딕셔너리 자료구조 (P1 - 중요)

**문법 설계**:
```kingsejong
# 딕셔너리 리터럴
사람 = {
    "이름": "홍길동",
    "나이": 25,
    "직업": "개발자"
}

# 접근
출력(사람["이름"])      # 대괄호
출력(사람.이름)         # 점 표기법 (선택)

# 수정
사람["나이"] = 26
```

**구현 요소**:

1. **토큰 추가** (이미 존재할 수 있음)
   - `LBRACE` `{`
   - `RBRACE` `}`
   - `COLON` `:`

2. **AST 노드**
   ```cpp
   struct DictLiteral : Expression {
       std::vector<std::pair<
           std::unique_ptr<Expression>,  // key
           std::unique_ptr<Expression>   // value
       >> pairs;
   };
   ```

3. **Value 타입 확장**
   ```cpp
   class Value {
       ValueType type_;  // 추가: DICT
       std::unordered_map<std::string, Value> dict_value_;
   };
   ```

**예상 공수**: 1-2주

### 6.5 배열 연산자 (P1 - 중요)

**문법 설계**:
```kingsejong
배열1 = [1, 2, 3]
배열2 = [4, 5, 6]
합쳐진배열 = 배열1 + 배열2  # [1, 2, 3, 4, 5, 6]
```

**구현**:

1. **이항 연산자 확장**
   ```cpp
   // evalBinaryExpression()에서
   if (left.type_ == ARRAY && op == "+" && right.type_ == ARRAY) {
       return concatenateArrays(left, right);
   }
   ```

2. **stdlib 함수**
   ```kingsejong
   합쳐진배열 = 배열1을 배열2와 결합한다
   ```

**예상 공수**: 3-5일

### 6.6 문자열 보간 (P1 - 중요)

**문법 설계**:
```kingsejong
이름 = "홍길동"
나이 = 25
메시지 = "$(이름)님의 나이는 $(나이)세입니다"
# 또는
메시지 = "{이름}님의 나이는 {나이}세입니다"
```

**구현**:

1. **Lexer 확장**
   - 문자열 내부에서 `$()` 또는 `{}` 감지
   - 보간 토큰 생성

2. **Parser**
   - StringLiteral → InterpolatedString
   - 표현식 파싱

**예상 공수**: 3-5일

### 6.7 Destructuring (P2 - 보완)

**문법 설계**:
```kingsejong
# 배열 언패킹
[a, b, c] = [1, 2, 3]

# 딕셔너리 언패킹
{이름, 나이} = 사람
```

**예상 공수**: 1주

---

## 로드맵 제안

### Phase 1: 핵심 구조 개선 (1-2개월)

**목표**: 언어 안정성 확보

| 작업 | 우선순위 | 예상 공수 | 담당 |
|------|---------|----------|------|
| Semantic Analyzer 구현 | P0 | 2-3주 | Backend |
| Symbol Table 구현 | P0 | 1주 | Backend |
| 변수 스코프 수정 | P0 | 1주 | Backend |
| 예외 처리 시스템 | P0 | 2주 | Backend |
| 테스트 케이스 추가 | P0 | 1주 | QA |

**완료 기준**:
- ✅ 휴리스틱 의존 제거
- ✅ 모든 변수명 길이 지원
- ✅ 스코프 버그 해결
- ✅ try/catch 동작
- ✅ 1200+ 테스트 통과

### Phase 2: 실용 기능 추가 (1개월)

**목표**: 언어 실용성 확보

| 작업 | 우선순위 | 예상 공수 | 담당 |
|------|---------|----------|------|
| 딕셔너리 자료구조 | P1 | 1-2주 | Backend |
| 배열 연산자 | P1 | 3-5일 | Backend |
| 문자열 보간 | P1 | 3-5일 | Backend |
| stdlib 확장 (파일 I/O, 네트워크) | P1 | 1주 | Backend |
| 에러 메시지 개선 | P1 | 3일 | Backend |

**완료 기준**:
- ✅ 딕셔너리 CRUD 동작
- ✅ 배열 결합 가능
- ✅ 문자열 보간 동작
- ✅ 실용적 예제 작성 가능

### Phase 3: 편의 기능 (1개월)

**목표**: 개발자 경험 향상

| 작업 | 우선순위 | 예상 공수 | 담당 |
|------|---------|----------|------|
| Destructuring | P2 | 1주 | Backend |
| Spread/Rest 연산자 | P2 | 3-5일 | Backend |
| 정규표현식 지원 | P2 | 1주 | Backend |
| stdlib 확장 (날짜/시간, JSON) | P2 | 1주 | Backend |
| LSP 기능 확장 | P2 | 1주 | Tools |

**완료 기준**:
- ✅ 모던 언어 기능 지원
- ✅ stdlib 200+ 함수
- ✅ IDE 지원 강화

### Phase 4: 최적화 & 생태계 (진행 중)

| 작업 | 우선순위 | 예상 공수 | 담당 |
|------|---------|----------|------|
| 바이트코드 컴파일러 | P3 | 1-2개월 | Backend |
| JIT 컴파일 | P3 | 2-3개월 | Backend |
| 패키지 관리자 | P2 | 1개월 | Tools |
| 표준 라이브러리 확장 | P2 | 지속적 | Community |
| 문서화 & 튜토리얼 | P1 | 1개월 | Docs |

---

## 테스트 현황

**현재 상태** (v0.3.1 기준):
```
Total Tests: 1004/1004 (100% pass rate)

Categories:
- Parser Tests: 271 tests ✅
- Evaluator Tests: 268 tests ✅
- Lexer Tests: Comprehensive ✅
- Loop Statement Tests: 21 tests ✅
- Array Tests: Full coverage ✅
- Closure Tests: Working ✅
- Pattern Matching Tests: Implemented ✅
- LSP Tests: 56 tests ✅
```

**개선 필요**:
- [ ] Semantic Analyzer 테스트 (0개)
- [ ] 스코프 버그 테스트 (미탐지)
- [ ] 예외 처리 테스트 (0개)
- [ ] 딕셔너리 테스트 (0개)
- [ ] 문자열 보간 테스트 (0개)

---

## 참고 문서

**KingSejong 소스**:
- 저장소: `/Users/kevin/work/github/kingsejong-lang/kingsejonglang`
- 주요 파일:
  - `src/lexer/Lexer.cpp` - 어휘 분석
  - `src/parser/Parser.cpp` - 구문 분석
  - `src/evaluator/Evaluator.cpp` - 실행
  - `src/evaluator/Environment.h` - 스코프 관리
  - `KNOWN_ISSUES.md` - 알려진 문제
  - `CHANGELOG.md` - 변경 이력

**비교 언어 참고**:
- Python: [PEP 8](https://peps.python.org/pep-0008/), [Language Reference](https://docs.python.org/3/reference/)
- JavaScript: [ECMAScript Specification](https://tc39.es/ecma262/)
- Lua: [Lua 5.4 Reference Manual](https://www.lua.org/manual/5.4/)

---

## 결론

### 강점
1. ✅ **한글 키워드**: 직관적이고 자연스러운 문법
2. ✅ **Josa 표현식**: 독창적인 메서드 체이닝
3. ✅ **패턴 매칭**: 현대적 언어 기능
4. ✅ **클로저 지원**: 함수형 프로그래밍 가능
5. ✅ **높은 테스트 커버리지**: 1004개 테스트 100% 통과

### 개선 필요 사항
1. 🔴 **파서 휴리스틱 제거**: Semantic Analyzer 구현
2. 🔴 **변수 스코프 버그 수정**: 부모 스코프 자동 탐색
3. 🔴 **예외 처리**: 시도/오류/마지막 문법
4. 🟡 **딕셔너리 자료구조**: 키-값 쌍 저장
5. 🟡 **배열/문자열 연산자**: 실용성 향상

### 권장 조치
1. **즉시 착수** (P0): Semantic Analyzer, 스코프 수정, 예외 처리
2. **단기 목표** (P1): 딕셔너리, 배열 연산자, 문자열 보간
3. **중기 목표** (P2): Destructuring, stdlib 확장
4. **장기 목표** (P3): 최적화, 생태계 구축

---

**문서 버전**: 1.0
**최종 수정**: 2025-11-16
**작성자**: AI Code Analysis
**리뷰 필요**: KingSejong Core Team
