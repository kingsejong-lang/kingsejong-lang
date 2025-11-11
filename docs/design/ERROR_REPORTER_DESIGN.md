# ErrorReporter 설계 문서

> **작성일**: 2025-11-11
> **작업**: F5.NEW - 에러 메시지 개선

## 1. 목표

KingSejong 언어의 에러 메시지를 개선하여 교육용 언어로서의 사용성을 극대화합니다.

### 현재 문제점

```
❌ 현재: "no prefix parse function for INTEGER found."
```

### 목표 에러 메시지

```
📍 오류 위치: 3번째 줄, 5번째 문자

   2 | x = 10
   3 | 123변수 = 20
       ^^^
   4 | 출력(x)

❌ 구문 오류: 변수 이름은 숫자로 시작할 수 없습니다.

💡 도움말: 변수 이름은 한글, 영문, 밑줄(_)로 시작해야 합니다.
   예시: 변수123, _temp, 숫자
```

## 2. 시스템 아키텍처

### 2.1 컴포넌트 구조

```
┌─────────────────────────────────────────┐
│         ErrorReporter                    │
│  - formatError()                         │
│  - reportError()                         │
│  - addHint()                            │
└───────────────┬─────────────────────────┘
                │
        ┌───────┴────────┐
        │                │
┌───────▼──────┐  ┌─────▼──────────┐
│ SourceManager │  │ ErrorFormatter │
│ - loadSource()│  │ - formatLine() │
│ - getLine()   │  │ - colorize()   │
│ - getContext()│  │ - highlight()  │
└───────────────┘  └────────────────┘
```

### 2.2 클래스 다이어그램

```cpp
// SourceManager: 소스 코드 관리
class SourceManager {
public:
    void loadSource(const std::string& filename, const std::string& content);
    std::optional<std::string> getLine(const std::string& filename, int line) const;
    std::vector<std::string> getContext(const std::string& filename, int line, int contextLines) const;

private:
    struct SourceFile {
        std::string filename;
        std::vector<std::string> lines;
    };
    std::unordered_map<std::string, SourceFile> sources_;
};

// ErrorFormatter: 에러 메시지 포맷팅
class ErrorFormatter {
public:
    std::string formatError(
        const error::KingSejongError& error,
        const SourceManager& sourceMgr,
        bool useColor = true
    );

private:
    std::string formatLocation(const error::SourceLocation& loc);
    std::string formatContext(
        const std::vector<std::string>& lines,
        int errorLine,
        int errorCol,
        int startLine
    );
    std::string formatHint(const std::string& hint);
    std::string colorize(const std::string& text, AnsiColor color);
};

// ErrorReporter: 통합 에러 리포팅
class ErrorReporter {
public:
    ErrorReporter();

    // 소스 코드 등록
    void registerSource(const std::string& filename, const std::string& content);

    // 에러 리포트
    void report(const error::KingSejongError& error, std::ostream& out = std::cerr);

    // 에러 힌트 등록
    void registerHint(error::ErrorType type, const std::string& pattern, const std::string& hint);

    // 컬러 출력 설정
    void setColorEnabled(bool enabled);

private:
    std::unique_ptr<SourceManager> sourceMgr_;
    std::unique_ptr<ErrorFormatter> formatter_;
    std::unordered_map<error::ErrorType, std::vector<ErrorHint>> hints_;
    bool colorEnabled_;

    struct ErrorHint {
        std::string pattern;  // 에러 메시지 패턴
        std::string hint;     // 힌트 메시지
    };
};
```

## 3. 에러 메시지 체계

### 3.1 에러 메시지 구조

모든 에러 메시지는 다음 구조를 따릅니다:

```
1. 📍 위치 정보 (선택)
2. 소스 코드 컨텍스트 (선택)
3. ❌ 에러 타입 + 메시지 (필수)
4. 💡 힌트/제안 (선택)
```

### 3.2 에러 타입별 한글 메시지

| ErrorType | 한글 이름 | 영어 원문 |
|-----------|----------|-----------|
| LEXER_ERROR | 어휘 오류 | Lexer Error |
| PARSER_ERROR | 구문 오류 | Parser Error |
| RUNTIME_ERROR | 실행 오류 | Runtime Error |
| TYPE_ERROR | 타입 오류 | Type Error |
| NAME_ERROR | 이름 오류 | Name Error |
| VALUE_ERROR | 값 오류 | Value Error |
| ZERO_DIVISION_ERROR | 0으로 나누기 오류 | Zero Division Error |
| INDEX_ERROR | 인덱스 오류 | Index Error |
| ARGUMENT_ERROR | 인자 오류 | Argument Error |

### 3.3 주요 에러 메시지 예시

#### 구문 오류: 숫자로 시작하는 변수명

```
📍 오류 위치: example.ksj:3:5

   2 | x = 10
   3 | 123변수 = 20
       ^^^
   4 | 출력(x)

❌ 구문 오류: 변수 이름은 숫자로 시작할 수 없습니다.

💡 도움말: 변수 이름은 한글, 영문, 밑줄(_)로 시작해야 합니다.
   예시: 변수123, _temp, 숫자
```

#### 이름 오류: 정의되지 않은 변수

```
📍 오류 위치: example.ksj:5:8

   4 | x = 10
   5 | y = z + 5
           ^
   6 | 출력(y)

❌ 이름 오류: 정의되지 않은 변수 'z'를 사용하려고 합니다.

💡 도움말: 변수를 먼저 선언하세요.
   예시: 정수 z = 0
```

#### 타입 오류: 잘못된 연산

```
📍 오류 위치: example.ksj:3:9

   2 | 이름 = "홍길동"
   3 | 결과 = 이름 + 10
              ^^^^
   4 | 출력(결과)

❌ 타입 오류: 문자열과 정수를 더할 수 없습니다.

💡 도움말: 같은 타입끼리만 연산할 수 있습니다.
   문자열과 문자열: "안녕" + "하세요"
   숫자와 숫자: 10 + 20
```

#### 0으로 나누기 오류

```
📍 오류 위치: example.ksj:2:9

   1 | x = 10
   2 | y = x / 0
           ^^^^^
   3 | 출력(y)

❌ 0으로 나누기 오류: 0으로 나눌 수 없습니다.

💡 도움말: 나누는 값(분모)이 0이 아닌지 확인하세요.
   예시:
   만약 (b != 0) {
       결과 = a / b
   } 아니면 {
       출력("0으로 나눌 수 없습니다")
   }
```

#### 인덱스 오류: 범위 초과

```
📍 오류 위치: example.ksj:3:9

   2 | 배열 = [1, 2, 3]
   3 | 값 = 배열[5]
            ^^^^^^
   4 | 출력(값)

❌ 인덱스 오류: 인덱스 5가 범위를 벗어났습니다 (배열 크기: 3).

💡 도움말: 배열의 인덱스는 0부터 시작하며, 크기-1까지 접근할 수 있습니다.
   이 배열의 유효한 인덱스: 0, 1, 2
```

## 4. ANSI 컬러 출력

### 4.1 컬러 체계

```cpp
enum class AnsiColor {
    RESET,
    RED,         // 에러 메시지
    GREEN,       // 성공 메시지
    YELLOW,      // 경고 메시지
    BLUE,        // 정보 메시지
    MAGENTA,     // 위치 표시
    CYAN,        // 힌트 메시지
    BOLD,        // 강조
    DIM          // 약한 표시 (줄 번호 등)
};
```

### 4.2 컬러 적용 예시

```
📍 오류 위치: example.ksj:3:5    [MAGENTA + BOLD]

   2 | x = 10                      [DIM] (줄 번호)
   3 | 123변수 = 20                [DIM] (줄 번호)
       ^^^                          [RED + BOLD] (에러 위치)
   4 | 출력(x)                     [DIM] (줄 번호)

❌ 구문 오류: ...                   [RED + BOLD]

💡 도움말: ...                      [CYAN]
```

### 4.3 컬러 비활성화

다음 경우 자동으로 컬러 비활성화:
- 터미널이 아닌 파일로 리다이렉트될 때
- 환境 변수 `NO_COLOR` 설정 시
- 명시적으로 `setColorEnabled(false)` 호출 시

## 5. 에러 힌트 시스템

### 5.1 힌트 등록 방식

```cpp
// 초기화 시 힌트 등록
errorReporter.registerHint(
    ErrorType::PARSER_ERROR,
    "변수 이름은 숫자로 시작할 수 없습니다",
    "변수 이름은 한글, 영문, 밑줄(_)로 시작해야 합니다.\n"
    "예시: 변수123, _temp, 숫자"
);

errorReporter.registerHint(
    ErrorType::NAME_ERROR,
    "정의되지 않은 변수",
    "변수를 먼저 선언하세요.\n"
    "예시: 정수 {name} = 0"
);
```

### 5.2 동적 힌트 생성

일부 힌트는 에러 컨텍스트에 따라 동적으로 생성:

```cpp
// IndexError: 배열 크기를 포함한 힌트
std::string generateIndexErrorHint(int index, int size) {
    return "배열의 인덱스는 0부터 시작하며, 크기-1까지 접근할 수 있습니다.\n"
           "이 배열의 유효한 인덱스: 0부터 " + std::to_string(size - 1) + "까지";
}
```

## 6. 구현 단계

### Phase 1: 기본 인프라 (Week 1, Day 1-3)
- [x] 현재 에러 처리 코드 분석
- [ ] SourceManager 클래스 구현
- [ ] ErrorFormatter 기본 구현
- [ ] ANSI 컬러 유틸리티
- [ ] 테스트 작성 (TDD)

### Phase 2: 에러 포맷팅 (Week 1, Day 4-5)
- [ ] 위치 정보 포맷팅
- [ ] 소스 코드 컨텍스트 표시
- [ ] 에러 위치 하이라이트 (화살표)
- [ ] 테스트 작성

### Phase 3: 힌트 시스템 (Week 2, Day 1-2)
- [ ] 에러 힌트 등록 시스템
- [ ] 주요 에러별 힌트 작성 (20+ 힌트)
- [ ] 동적 힌트 생성
- [ ] 테스트 작성

### Phase 4: Lexer/Parser 통합 (Week 2, Day 3-4)
- [ ] Lexer 에러 메시지 한글화
- [ ] Parser 에러 메시지 한글화
- [ ] 위치 정보 추적 개선
- [ ] 테스트 작성

### Phase 5: Evaluator 통합 (Week 2, Day 5)
- [ ] Evaluator 에러 메시지 한글화
- [ ] 런타임 스택 트레이스 개선
- [ ] 테스트 작성

## 7. 메모리 안전성 및 코드 품질

### 7.1 RAII 준수

```cpp
// GOOD - unique_ptr 사용
class ErrorReporter {
private:
    std::unique_ptr<SourceManager> sourceMgr_;
    std::unique_ptr<ErrorFormatter> formatter_;
};

// BAD - raw pointer (사용 금지)
class ErrorReporter {
private:
    SourceManager* sourceMgr_;  // 누가 소유? 누가 삭제?
};
```

### 7.2 예외 안전성

```cpp
// 모든 public 메서드는 예외에 안전해야 함
void ErrorReporter::report(const KingSejongError& error, std::ostream& out) {
    try {
        std::string formatted = formatter_->formatError(error, *sourceMgr_, colorEnabled_);
        out << formatted << std::endl;
    } catch (const std::exception& e) {
        // 에러 리포팅 중 예외 발생 시 최소 정보라도 출력
        out << "[ErrorReporter 내부 오류] " << e.what() << std::endl;
        out << "원본 에러: " << error.what() << std::endl;
    }
}
```

### 7.3 테스트 우선 개발 (TDD)

모든 기능은 테스트를 먼저 작성:

```cpp
// tests/error/ErrorReporterTest.cpp
TEST(ErrorReporterTest, ShouldFormatBasicError) {
    ErrorReporter reporter;
    reporter.registerSource("test.ksj", "x = 10\n123변수 = 20\n출력(x)");

    auto error = ParserError("변수 이름은 숫자로 시작할 수 없습니다",
                             SourceLocation("test.ksj", 2, 1));

    std::ostringstream oss;
    reporter.report(error, oss);

    std::string output = oss.str();
    EXPECT_THAT(output, HasSubstr("test.ksj:2:1"));
    EXPECT_THAT(output, HasSubstr("123변수"));
    EXPECT_THAT(output, HasSubstr("구문 오류"));
}
```

## 8. 성공 기준

- [ ] 모든 에러 메시지가 한글로 출력됨
- [ ] 에러 위치가 정확히 표시됨 (줄, 컬럼, 화살표)
- [ ] 소스 코드 컨텍스트가 표시됨 (에러 전후 3줄)
- [ ] 주요 에러 20개 이상 힌트 제공
- [ ] ANSI 컬러 출력 지원
- [ ] 100% 테스트 통과
- [ ] ASan/UBSan 클린
- [ ] 기존 567개 테스트 모두 통과

## 9. 참고 자료

### 좋은 에러 메시지 예시

**Rust:**
```rust
error[E0425]: cannot find value `x` in this scope
 --> src/main.rs:2:13
  |
2 |     println!("{}", x);
  |                    ^ not found in this scope
```

**Elm:**
```
-- NAMING ERROR -------------------------------------------- Main.elm

Cannot find variable `x`

3|     x + 1
       ^
Maybe you want one of the following?

    y
```

**KingSejong 목표:**
```
📍 오류 위치: Main.ksj:3:5

   2 | y = 10
   3 | 출력(x)
          ^
   4 |

❌ 이름 오류: 정의되지 않은 변수 'x'를 사용하려고 합니다.

💡 도움말: 변수를 먼저 선언하세요.
   비슷한 이름: y
```

## 10. 결론

이 설계를 통해 KingSejong 언어는:
1. **교육 친화적**: 명확한 한글 에러 메시지
2. **위치 정확성**: 에러 발생 위치를 정확히 표시
3. **학습 지원**: 힌트를 통한 해결 방법 제시
4. **시각적**: 컬러 출력으로 가독성 향상

이는 초보자가 프로그래밍을 배우기에 이상적인 환경을 제공합니다.
