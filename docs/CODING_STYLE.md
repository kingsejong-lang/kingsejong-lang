# KingSejong 코딩 스타일 가이드

**버전**: 2.0
**최종 수정**: 2025-11-26

## 개요

이 문서는 KingSejong 프로젝트의 실제 코드베이스에서 사용되는 코딩 스타일을 정리한 가이드입니다. 모든 기여자는 일관성을 위해 이 가이드를 따라야 합니다.

**핵심 원칙**:
- **일관성**: 기존 코드 스타일 유지
- **가독성**: 명확하고 이해하기 쉬운 코드
- **모던 C++23**: 최신 C++ 기능 적극 활용
- **Doxygen 문서화**: 모든 공개 API 문서화
- **RAII & 스마트 포인터**: 안전한 메모리 관리

---

## 1. 네이밍 규칙

### 1.1 클래스 및 구조체

**PascalCase** 사용

```cpp
// Good
class TokenType { };
class JosaRecognizer { };
struct ParseResult { };

// Bad
class token_type { };
class josaRecognizer { };
```

### 1.2 함수 및 메서드

**camelCase** 사용

```cpp
// Good
void parseExpression();
bool isValidToken();
int calculatePrecedence();

// Bad
void ParseExpression();
void parse_expression();
```

### 1.3 변수

**camelCase** 사용

```cpp
// Good
int tokenCount;
std::string currentLine;
auto parseResult = parser.parse();

// Bad
int token_count;
int TokenCount;
```

### 1.4 멤버 변수

**camelCase + 언더스코어 접미사** (private/protected 멤버)

```cpp
class Lexer {
private:
    std::string input;              // public은 접미사 없음
    size_t position;
    morphology::MorphologicalAnalyzer morphAnalyzer_;  // private은 '_' 접미사

public:
    std::string filename;           // public 멤버는 접미사 없음
};

// 실제 예제 (Lexer.h)
class Lexer {
private:
    std::string input;
    std::string filename;
    size_t position;
    size_t readPosition;
    char ch;
    int currentLine;
    int currentColumn;
    morphology::MorphologicalAnalyzer morphAnalyzer_;  // RAII 객체는 '_'
};
```

### 1.5 상수

**UPPER_SNAKE_CASE** 사용

```cpp
// Good (실제 예제: VM.h)
constexpr size_t VM_DEFAULT_MAX_INSTRUCTIONS = 10000000;
constexpr int VM_DEFAULT_MAX_EXECUTION_TIME_MS = 5000;
constexpr size_t VM_DEFAULT_MAX_STACK_SIZE = 10000;
constexpr int JIT_LOOP_THRESHOLD = 100;
constexpr uint8_t NO_CONSTRUCTOR_FLAG = 0xFF;

// Bad
const int vmDefaultMaxInstructions = 10000000;
const int vm_default_max_instructions = 10000000;
```

### 1.6 네임스페이스

**소문자 snake_case** 사용

```cpp
// Good
namespace kingsejong {
namespace lexer {
    // ...
}
}

// Bad
namespace KingSejong { }
namespace KingSejong_Lexer { }
```

### 1.7 Enum Class

**PascalCase** (타입), **PascalCase** 또는 **UPPER_CASE** (값)

```cpp
// Good (실제 예제: Parser.h, VM.h)
enum class ParseFeature : uint32_t {
    None       = 0,
    Range      = 1 << 0,
    All        = 0xFFFFFFFFu
};

enum class Precedence {
    LOWEST = 0,
    ASSIGN,
    RANGE,
    OR,
    AND,
    EQUALS
};

enum class VMResult {
    OK,                 ///< 인라인 주석 스타일
    HALT,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

// Bad
enum class token_type {    // 타입은 PascalCase
    identifier,            // 값은 PascalCase 또는 UPPER_CASE
    INTEGER
};
```

### 1.8 파일명

- 헤더: **PascalCase.h**
- 구현: **PascalCase.cpp**
- 테스트: **PascalCaseTest.cpp**

```
src/lexer/Lexer.h
src/lexer/Lexer.cpp
src/parser/Parser.h
src/parser/Parser.cpp
src/bytecode/VM.h
src/bytecode/VM.cpp
tests/lexer/LexerTest.cpp
tests/parser/ParserTest.cpp
tests/bytecode/VMTest.cpp
```

---

## 2. Doxygen 문서화

### 2.1 파일 헤더

모든 헤더 파일은 Doxygen 주석으로 시작합니다.

```cpp
#pragma once

/**
 * @file Parser.h
 * @brief KingSejong 언어 파서 (Pratt Parsing 알고리즘)
 * @author KingSejong Team
 * @date 2025-11-07
 */
```

### 2.2 클래스 문서화

```cpp
/**
 * @class Lexer
 * @brief 소스 코드를 토큰으로 분해하는 어휘 분석기
 *
 * UTF-8 인코딩된 한글 소스 코드를 읽어서 토큰 스트림으로 변환합니다.
 * 한글 키워드, 조사, 식별자를 올바르게 인식합니다.
 */
class Lexer {
    // ...
};
```

### 2.3 함수 문서화

```cpp
/**
 * @brief Lexer 생성자
 * @param input 분석할 소스 코드 문자열 (UTF-8 인코딩)
 * @param filename 소스 파일 이름
 */
Lexer(const std::string& input, const std::string& filename);

/**
 * @brief 다음 토큰을 반환
 * @return Token 객체
 *
 * 입력 문자열에서 다음 토큰을 읽어서 반환합니다.
 * 파일 끝에 도달하면 EOF_TOKEN을 반환합니다.
 */
Token nextToken();
```

### 2.4 멤버 변수 인라인 주석

```cpp
class Lexer {
private:
    std::string input;          ///< 입력 소스 코드
    std::string filename;       ///< 소스 파일 이름 (에러 메시지용)
    size_t position;            ///< 현재 읽는 위치
    size_t readPosition;        ///< 다음 읽을 위치
    char ch;                    ///< 현재 문자
    int currentLine;            ///< 현재 줄 번호 (1부터 시작)
    int currentColumn;          ///< 현재 열 번호 (1부터 시작)
};
```

### 2.5 Enum 문서화

```cpp
/**
 * @enum VMResult
 * @brief VM 실행 결과
 */
enum class VMResult {
    OK,                 ///< 성공
    HALT,               ///< 정상 종료 (HALT 명령)
    COMPILE_ERROR,      ///< 컴파일 에러
    RUNTIME_ERROR       ///< 런타임 에러
};
```

---

## 3. 포맷팅 규칙

### 3.1 헤더 가드

**`#pragma once`** 사용 (전통적인 include guard 대신)

```cpp
// Good
#pragma once

// Bad
#ifndef KINGSEJONG_LEXER_H
#define KINGSEJONG_LEXER_H
// ...
#endif
```

### 3.2 들여쓰기

**4칸 스페이스** 사용 (탭 금지)

```cpp
// Good
class Parser {
    void parse() {
        if (condition) {
            doSomething();
        }
    }
};

// Bad (2칸)
class Parser {
  void parse() {
    doSomething();
  }
};
```

### 2.2 중괄호 스타일

**Allman 스타일** 사용

```cpp
// Good
void function()
{
    if (condition)
    {
        doSomething();
    }
    else
    {
        doOtherThing();
    }
}

// Bad (K&R 스타일)
void function() {
    if (condition) {
        doSomething();
    }
}
```

### 2.3 한 줄 길이

**최대 100자** (권장 80자)

```cpp
// Good
auto result = performComplexOperation(
    parameter1,
    parameter2,
    parameter3
);

// Bad - 너무 긴 줄
auto result = performComplexOperation(parameter1, parameter2, parameter3, parameter4, parameter5);
```

### 2.4 공백

```cpp
// 연산자 주변 공백
int x = a + b;  // Good
int x=a+b;      // Bad

// 쉼표 뒤 공백
function(a, b, c);  // Good
function(a,b,c);    // Bad

// 괄호 내부 공백 없음
if (condition)  // Good
if ( condition )  // Bad

// 포인터/참조 선언
int* ptr;       // Good
int *ptr;       // Acceptable
int * ptr;      // Bad
```

### 2.5 빈 줄

```cpp
// 함수 사이 1줄
void functionA()
{
    // ...
}

void functionB()
{
    // ...
}

// 논리적 블록 사이 1줄
void complexFunction()
{
    // 초기화
    int x = 0;
    int y = 0;

    // 처리
    processData(x, y);

    // 정리
    cleanup();
}
```

---

## 3. C++23 모던 스타일

### 3.1 타입 추론

**auto** 적극 활용

```cpp
// Good
auto token = lexer.nextToken();
auto result = std::make_unique<ParseResult>();

// Bad - 불필요한 타입 명시
std::unique_ptr<ParseResult> result = std::make_unique<ParseResult>();
```

### 3.2 스마트 포인터

**절대 raw pointer 금지** (소유권이 있는 경우)

```cpp
// Good
std::unique_ptr<Node> node = std::make_unique<Node>();
std::shared_ptr<Environment> env = std::make_shared<Environment>();

// Bad
Node* node = new Node();  // 메모리 누수 위험!
```

### 3.3 범위 기반 for 루프

```cpp
// Good
for (const auto& token : tokens)
{
    process(token);
}

// Bad
for (size_t i = 0; i < tokens.size(); ++i)
{
    process(tokens[i]);
}
```

### 3.4 nullptr 사용

```cpp
// Good
int* ptr = nullptr;

// Bad
int* ptr = NULL;
int* ptr = 0;
```

### 3.5 enum class 사용

```cpp
// Good
enum class TokenType
{
    Integer,
    String
};

// Bad
enum TokenType
{
    INTEGER,
    STRING
};
```

### 3.6 const 적극 활용

```cpp
// Good
const int MAX_SIZE = 100;
void process(const std::string& input);

// Bad
int MAX_SIZE = 100;  // 변경 가능
void process(std::string input);  // 복사 발생
```

---

## 4. 주석 규칙

### 4.1 파일 헤더

```cpp
/**
 * @file Lexer.h
 * @brief 어휘 분석기 클래스 정의
 * @author KingSejong Team
 * @date 2025-11-06
 */
```

### 4.2 클래스 주석

```cpp
/**
 * @class Lexer
 * @brief UTF-8 한글을 지원하는 어휘 분석기
 *
 * 소스 코드를 토큰으로 분해하며, 한글 조사를 인식합니다.
 */
class Lexer
{
    // ...
};
```

### 4.3 함수 주석

```cpp
/**
 * @brief 다음 토큰을 반환합니다
 * @return 다음 토큰 (더 이상 없으면 EOF 토큰)
 * @throws LexerException 잘못된 문자가 있는 경우
 */
Token nextToken();
```

### 4.4 인라인 주석

```cpp
// Good - 왜(Why)를 설명
// UTF-8의 첫 바이트를 확인하여 문자 길이를 계산
int charLength = getUtf8CharLength(firstByte);

// Bad - 무엇(What)을 반복
// charLength에 getUtf8CharLength 결과를 저장
int charLength = getUtf8CharLength(firstByte);
```

### 4.5 TODO 주석

```cpp
// TODO(username): 설명
// FIXME(username): 설명
// NOTE: 설명

// Good
// TODO(mhha): 여기에 에러 처리 추가 필요

// Bad
// TODO: 나중에 고칠 것
```

---

## 5. 클래스 구조

### 5.1 멤버 순서

```cpp
class Example
{
public:
    // 1. 생성자/소멸자
    Example();
    ~Example();

    // 2. public 메서드
    void publicMethod();

protected:
    // 3. protected 메서드
    void protectedMethod();

private:
    // 4. private 메서드
    void privateMethod();

    // 5. private 멤버 변수 (접두사 m_)
    int m_count;
    std::string m_name;
};
```

### 5.2 멤버 변수 네이밍

**m_ 접두사** 사용

```cpp
class Parser
{
private:
    Lexer m_lexer;
    Token m_currentToken;
    int m_position;
};
```

---

## 6. 에러 처리

### 6.1 예외 사용

```cpp
// Good
if (!isValid)
{
    throw LexerException("Invalid token: " + token);
}

// Bad - 에러 코드 반환
int parse()
{
    if (!isValid)
    {
        return -1;  // 매직 넘버
    }
    return 0;
}
```

### 6.2 커스텀 예외

```cpp
class KingSejongException : public std::exception
{
public:
    explicit KingSejongException(const std::string& message)
        : m_message(message)
    {
    }

    const char* what() const noexcept override
    {
        return m_message.c_str();
    }

private:
    std::string m_message;
};
```

---

## 7. 파일 구조

### 7.1 헤더 파일 구조

```cpp
#pragma once

// 1. 시스템 헤더
#include <string>
#include <vector>

// 2. 외부 라이브러리 헤더
// (현재 없음)

// 3. 프로젝트 헤더
#include "Token.h"

// 4. 네임스페이스
namespace kingsejong {
namespace lexer {

// 5. 클래스 선언
class Lexer
{
    // ...
};

} // namespace lexer
} // namespace kingsejong
```

### 7.2 Include Guard

**#pragma once** 사용 (전통적인 include guard 대신)

```cpp
// Good
#pragma once

// Bad
#ifndef LEXER_H
#define LEXER_H
// ...
#endif
```

---

## 8. 테스트 코드

### 8.1 테스트 네이밍

```cpp
// Good
TEST(LexerTest, ShouldRecognizeIntegerToken)
{
    // Arrange
    Lexer lexer("123");

    // Act
    auto token = lexer.nextToken();

    // Assert
    EXPECT_EQ(token.type, TokenType::Integer);
    EXPECT_EQ(token.literal, "123");
}

// Bad
TEST(LexerTest, Test1)
{
    // ...
}
```

### 8.2 AAA 패턴

**Arrange-Act-Assert** 패턴 사용

```cpp
TEST(ParserTest, ShouldParseAdditionExpression)
{
    // Arrange - 준비
    std::string input = "1 + 2";
    Lexer lexer(input);
    Parser parser(lexer);

    // Act - 실행
    auto expr = parser.parseExpression();

    // Assert - 검증
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->type(), NodeType::BinaryExpression);
}
```

---

## 9. 성능 가이드

### 9.1 복사 최소화

```cpp
// Good - 참조 전달
void process(const std::string& str);

// Bad - 값 복사
void process(std::string str);
```

### 9.2 이동 시맨틱

```cpp
// Good
std::unique_ptr<Node> node = std::move(oldNode);

// Bad
std::unique_ptr<Node> node = oldNode;  // 컴파일 에러
```

### 9.3 Reserve 사용

```cpp
// Good
std::vector<Token> tokens;
tokens.reserve(estimatedSize);

// Bad - 반복적인 재할당
std::vector<Token> tokens;
for (int i = 0; i < 1000; ++i)
{
    tokens.push_back(token);  // 재할당 발생 가능
}
```

---

## 10. 금지 사항

### 10.1 절대 금지

- **매크로 남용** (#define 대신 const, constexpr, enum class 사용)
- **goto 사용**
- **전역 변수** (상수 제외)
- **C 스타일 캐스팅** (static_cast, dynamic_cast 사용)
- **raw pointer 소유권** (스마트 포인터 사용)
- **매직 넘버** (named constant 사용)

```cpp
// Bad
#define MAX_SIZE 100  // const 사용
goto error;           // 예외 사용
int globalVar;        // 전역 변수
int x = (int)value;   // static_cast 사용
int* ptr = new int;   // unique_ptr 사용
if (status == -1)     // enum 사용
```

### 10.2 권장하지 않음

- **using namespace std** (헤더 파일에서 절대 금지)
- **상속 남용** (컴포지션 우선)
- **복잡한 삼항 연산자**

---

## 11. 도구 설정

### 11.1 clang-format

프로젝트 루트에 `.clang-format` 파일 사용

```yaml
BasedOnStyle: Microsoft
IndentWidth: 4
ColumnLimit: 100
```

### 11.2 clang-tidy

CI/CD에서 자동 실행

---

## 12. 예제

### 12.1 완전한 클래스 예제

```cpp
#pragma once

#include <string>
#include <memory>

namespace kingsejong {

/**
 * @class Token
 * @brief 어휘 분석 결과 토큰
 */
class Token
{
public:
    /**
     * @brief 토큰 생성자
     * @param type 토큰 타입
     * @param literal 토큰 문자열
     */
    Token(TokenType type, const std::string& literal)
        : m_type(type)
        , m_literal(literal)
    {
    }

    TokenType getType() const { return m_type; }
    const std::string& getLiteral() const { return m_literal; }

private:
    TokenType m_type;
    std::string m_literal;
};

} // namespace kingsejong
```

---

## 참고 자료

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [Clean Code in C++](https://www.amazon.com/Clean-Code-Handbook-Software-Craftsmanship/dp/0132350882)
