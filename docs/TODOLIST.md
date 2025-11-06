# KingSejong 언어 구현 작업 목록

> **프로젝트**: KingSejong Programming Language (`.ksj`)
> **시작일**: 2025-11-06
> **상태 기호**: ✅ 완료 | 🚧 진행중 | 📝 대기 | ⏸️ 보류 | ❌ 취소

---

## 📋 목차

1. [Phase 0: 프로젝트 초기화 (Week 1-2)](#phase-0-프로젝트-초기화)
2. [Phase 1: 핵심 차별화 기능 (Month 1-3)](#phase-1-핵심-차별화-기능)
3. [Phase 2: 실용 기능 (Month 4-6)](#phase-2-실용-기능)
4. [Phase 3: 고급 기능 (Month 7-12)](#phase-3-고급-기능)

---

## Git 워크플로우

### 브랜치 전략
```
main (보호됨)
├── feature/phase0-project-setup
├── feature/token-system
├── feature/josa-recognizer
├── feature/lexer
├── feature/parser
└── ...
```

### 작업 프로세스
1. **Feature 브랜치 생성**: `git checkout -b feature/[feature-name]`
2. **작업 & 커밋**: 세부 작업마다 커밋
3. **PR 생성**: main으로 Pull Request
4. **스쿼시 머지**: PR 병합 시 하나의 커밋으로 통합
5. **문서 업데이트**: 이 파일의 상태 업데이트

---

## Phase 0: 프로젝트 초기화

**목표**: 개발 환경 구축 및 기본 인프라 설정
**기간**: Week 1-2 (2주)
**브랜치**: `feature/phase0-project-setup`

### F0.1: 프로젝트 기본 설정
- 상태: ✅ 완료
- 브랜치: `feature/f0.1-project-setup`
- 작업:
  - [x] README.md 작성
    - 프로젝트 소개
    - 빌드 방법
    - 예제 코드
  - [x] .gitignore 작성
    - C++ 빌드 파일
    - IDE 설정 파일
    - 테스트 결과 파일
  - [x] CMakeLists.txt 작성
    - C++23 설정
    - 소스 파일 설정
    - 테스트 설정
  - [x] .github/workflows/ci.yml 작성
    - Ubuntu, macOS, Windows 빌드
    - 자동 테스트 실행
  - [x] src/main.cpp 기본 구조 작성
- 완료 조건:
  - [x] 로컬 macOS에서 빌드 성공
  - [x] 실행 파일 동작 확인
  - [ ] CI/CD 파이프라인 동작 (main 머지 후)

### F0.2: 코딩 표준 문서
- 상태: 📝 대기
- 브랜치: `feature/phase0-project-setup`
- 작업:
  - [ ] docs/CODING_STYLE.md 작성
    - 네이밍 규칙
    - 포맷팅 규칙
    - 주석 작성 규칙
  - [ ] docs/CONTRIBUTING.md 작성
    - PR 프로세스
    - 커밋 메시지 규칙
    - 코드 리뷰 가이드
- 완료 조건:
  - [ ] 문서 작성 완료

### F0.3: 테스트 프레임워크 설정
- 상태: 📝 대기
- 브랜치: `feature/test-framework`
- 작업:
  - [ ] GoogleTest 통합
  - [ ] tests/CMakeLists.txt 작성
  - [ ] 더미 테스트 작성 (빌드 확인용)
- 완료 조건:
  - [ ] `make test` 실행 성공
  - [ ] CI에서 테스트 자동 실행

---

## Phase 1: 핵심 차별화 기능

**목표**: KingSejong만의 혁신적 기능 구현
**기간**: Month 1-3 (12주)

---

### Week 1-2: Token 시스템 & 조사 인식

#### F1.1: Token 시스템 구현
- 상태: 📝 대기
- 브랜치: `feature/token-system`
- 우선순위: CRITICAL
- 작업:
  - [ ] src/lexer/Token.h 구현
    ```cpp
    enum class TokenType {
        // 기본
        ILLEGAL, EOF_TOKEN, IDENTIFIER, INTEGER,

        // 연산자
        ASSIGN, PLUS, MINUS, ASTERISK, SLASH,
        EQ, NOT_EQ, LT, GT, LE, GE,

        // 조사
        JOSA_EUL, JOSA_REUL,  // 을/를
        JOSA_I, JOSA_GA,      // 이/가
        JOSA_EUN, JOSA_NEUN,  // 은/는
        JOSA_UI,              // 의
        JOSA_RO, JOSA_EURO,   // 로/으로
        JOSA_ESO,             // 에서
        JOSA_E,               // 에

        // 범위 키워드
        BUTEO,    // 부터
        KKAJI,    // 까지
        MIMAN,    // 미만
        CHOGA,    // 초과
        IHA,      // 이하
        ISANG,    // 이상

        // 반복 키워드
        BEON,     // 번
        BANBOKK,  // 반복
        GAKGAK,   // 각각

        // 키워드
        FUNCTION, IF, ELSE, RETURN, TRUE, FALSE,
        INTEGER_TYPE, FLOAT_TYPE, STRING_TYPE, BOOLEAN_TYPE,

        // 괄호/구분자
        LPAREN, RPAREN, LBRACE, RBRACE,
        LBRACKET, RBRACKET,
        COMMA, SEMICOLON
    };
    ```
  - [ ] Token 구조체 정의
  - [ ] 한글 키워드 매핑
- 테스트:
  - [ ] tests/token_test.cpp 작성
- 완료 조건:
  - [ ] 모든 TokenType 정의 완료
  - [ ] 테스트 통과

#### F1.2: JosaRecognizer 구현
- 상태: 📝 대기
- 브랜치: `feature/josa-recognizer`
- 우선순위: CRITICAL
- 작업:
  - [ ] src/lexer/JosaRecognizer.h 설계
    ```cpp
    class JosaRecognizer {
    public:
        enum class JosaType {
            EUL_REUL,   // 을/를 - 목적격
            I_GA,       // 이/가 - 주격
            EUN_NEUN,   // 은/는 - 보조사
            UI,         // 의 - 소유격
            RO_EURO,    // 로/으로 - 수단/방법
            ESO,        // 에서 - 출처
            E           // 에 - 위치/대상
        };

        static bool isJosa(const std::string& str);
        static JosaType getType(const std::string& str);
        static bool hasFinalConsonant(const std::string& noun);
        static std::string select(const std::string& noun, JosaType type);
    };
    ```
  - [ ] src/lexer/JosaRecognizer.cpp 구현
    - 조사 인식 로직
    - 받침 유무 판단 (`hasFinalConsonant`)
    - 자동 조사 선택 (`select`)
- 테스트:
  - [ ] tests/josa_recognizer_test.cpp
    - 조사 인식 테스트
    - 받침 판단 테스트
    - 조사 선택 테스트
- 완료 조건:
  - [ ] 모든 조사 타입 인식
  - [ ] 받침 유무 정확히 판단
  - [ ] 테스트 통과율 100%

---

### Week 3-4: Lexer 기본 구현

#### F1.3: 기본 Lexer 구현
- 상태: 📝 대기
- 브랜치: `feature/lexer`
- 우선순위: CRITICAL
- 작업:
  - [ ] src/lexer/Lexer.h 설계
    ```cpp
    class Lexer {
    private:
        std::string input;
        size_t position;
        size_t readPosition;
        char ch;

        void readChar();
        char peekChar();
        std::string readIdentifier();
        std::string readNumber();
        void skipWhitespace();

        // UTF-8 한글 처리
        int getCharLength(char c);
        std::string readUTF8Char();

    public:
        explicit Lexer(const std::string& input);
        Token nextToken();
    };
    ```
  - [ ] src/lexer/Lexer.cpp 구현
    - UTF-8 한글 처리
    - 기본 토큰화
    - 연산자 인식
  - [ ] 조사 통합
    - JosaRecognizer 활용
    - 조사 토큰 생성
- 테스트:
  - [ ] tests/lexer_test.cpp
    - 기본 토큰화 테스트
    - 한글 키워드 테스트
    - 조사 인식 테스트
    - 숫자/식별자 테스트
- 완료 조건:
  - [ ] UTF-8 한글 올바르게 처리
  - [ ] 모든 토큰 타입 인식
  - [ ] 조사 정확히 인식
  - [ ] 테스트 통과율 100%

---

### Week 5-6: AST 및 기본 Parser

#### F1.4: AST 노드 설계
- 상태: 📝 대기
- 브랜치: `feature/ast`
- 우선순위: CRITICAL
- 작업:
  - [ ] src/ast/Node.h 구현
    ```cpp
    enum class NodeType {
        PROGRAM,
        EXPRESSION_STATEMENT,
        IDENTIFIER,
        INTEGER_LITERAL,
        BINARY_EXPRESSION,
        JOSA_EXPRESSION,
        RANGE_EXPRESSION,
        // ...
    };

    class Node {
    public:
        virtual ~Node() = default;
        virtual NodeType type() const = 0;
        virtual std::string toString() const = 0;
    };
    ```
  - [ ] src/ast/Expression.h
    - Expression 기본 클래스
    - Identifier
    - IntegerLiteral
    - BinaryExpression
  - [ ] src/ast/JosaExpression.h
    ```cpp
    class JosaExpression : public Expression {
    private:
        std::unique_ptr<Expression> object;
        JosaRecognizer::JosaType josaType;
        std::unique_ptr<Expression> method;
    public:
        // ...
    };
    ```
  - [ ] src/ast/RangeExpression.h
    ```cpp
    class RangeExpression : public Expression {
    private:
        std::unique_ptr<Expression> start;
        std::unique_ptr<Expression> end;
        bool startInclusive;
        bool endInclusive;
    public:
        // ...
    };
    ```
- 테스트:
  - [ ] tests/ast_test.cpp
- 완료 조건:
  - [ ] 모든 노드 타입 정의
  - [ ] 스마트 포인터 사용 (메모리 안전)
  - [ ] 테스트 통과

#### F1.5: 기본 Parser 구현
- 상태: 📝 대기
- 브랜치: `feature/parser`
- 우선순위: CRITICAL
- 작업:
  - [ ] src/parser/Parser.h 설계
    ```cpp
    class Parser {
    private:
        Lexer& lexer;
        Token curToken;
        Token peekToken;

        // Pratt Parsing
        enum Precedence {
            LOWEST,
            EQUALS,      // ==
            LESSGREATER, // < >
            SUM,         // +
            PRODUCT,     // *
            PREFIX,      // -X, !X
            CALL         // func()
        };

        std::unique_ptr<Expression> parseExpression(Precedence prec);
        std::unique_ptr<Expression> parseIdentifier();
        std::unique_ptr<Expression> parseIntegerLiteral();
        std::unique_ptr<Expression> parseBinaryExpression(std::unique_ptr<Expression> left);

    public:
        explicit Parser(Lexer& lexer);
        std::unique_ptr<Program> parseProgram();
    };
    ```
  - [ ] src/parser/Parser.cpp 구현
    - Pratt Parsing 알고리즘
    - 연산자 우선순위 처리
- 테스트:
  - [ ] tests/parser_test.cpp
    - 기본 표현식 파싱
    - 연산자 우선순위
- 완료 조건:
  - [ ] 기본 표현식 파싱
  - [ ] 테스트 통과

---

### Week 7-8: 조사 체인 파싱

#### F1.6: 조사 체인 Parser 구현
- 상태: 📝 대기
- 브랜치: `feature/josa-parser`
- 우선순위: CRITICAL (핵심 차별화!)
- 작업:
  - [ ] src/parser/JosaParser.h
    ```cpp
    class JosaParser {
    public:
        static std::unique_ptr<JosaExpression>
            parseJosaChain(Parser& parser, std::unique_ptr<Expression> object);
    };
    ```
  - [ ] Parser에 조사 체인 통합
    - `parseExpression`에서 조사 감지
    - JosaParser 호출
  - [ ] 조사 체인 파싱 로직
    ```
    배열을 정렬한다
    ↓
    JosaExpression {
        object: Identifier("배열")
        josaType: EUL_REUL
        method: Identifier("정렬한다")
    }
    ```
- 테스트:
  - [ ] tests/josa_parser_test.cpp
    - `"배열을 정렬한다"` 파싱
    - `"사용자의 이름"` 파싱
    - `"문자열로 변환한다"` 파싱
    - 체인: `"배열을 정렬하고 역순으로 뒤집는다"`
- 완료 조건:
  - [ ] 모든 조사 타입 파싱
  - [ ] 조사 체인 파싱
  - [ ] 테스트 통과

---

### Week 9-10: 범위 표현

#### F1.7: 범위 표현 Parser 구현
- 상태: 📝 대기
- 브랜치: `feature/range-expression`
- 우선순위: CRITICAL (핵심 차별화!)
- 작업:
  - [ ] src/parser/RangeParser.h
    ```cpp
    class RangeParser {
    public:
        static std::unique_ptr<RangeExpression>
            parseRange(Parser& parser);
    };
    ```
  - [ ] 범위 파싱 로직
    ```
    1부터 10까지    → [1, 10] inclusive both
    1부터 10미만    → [1, 10) inclusive start, exclusive end
    1초과 10이하    → (1, 10] exclusive start, inclusive end
    ```
- 테스트:
  - [ ] tests/range_parser_test.cpp
    - `"1부터 10까지"` 파싱
    - `"1부터 10미만"` 파싱
    - `"1초과 10이하"` 파싱
- 완료 조건:
  - [ ] 모든 범위 형태 파싱
  - [ ] 테스트 통과

---

### Week 11-12: 타입 시스템 기초

#### F1.8: Type 시스템 구현
- 상태: 📝 대기
- 브랜치: `feature/type-system`
- 우선순위: CRITICAL
- 작업:
  - [ ] src/types/Type.h
    ```cpp
    enum class TypeKind {
        INTEGER,   // 정수
        FLOAT,     // 실수
        STRING,    // 문자열
        BOOLEAN,   // 참거짓
        NULL_TYPE, // 없음
        ARRAY,
        FUNCTION
    };

    class Type {
    private:
        TypeKind kind;
        std::string koreanName;
    public:
        static Type* getBuiltin(const std::string& koreanName);
        // "정수" → INTEGER
        // "실수" → FLOAT
        // "문자열" → STRING
    };
    ```
  - [ ] 타입 선언 파싱
    ```
    정수 나이 = 15
    ↓
    VarDeclaration {
        type: Type(INTEGER, "정수")
        name: "나이"
        value: IntegerLiteral(15)
    }
    ```
- 테스트:
  - [ ] tests/type_test.cpp
    - 타입 생성
    - 한글 이름 매핑
  - [ ] tests/type_declaration_test.cpp
    - `"정수 나이 = 15"` 파싱
- 완료 조건:
  - [ ] 기본 타입 정의
  - [ ] 타입 선언 파싱
  - [ ] 테스트 통과

---

### Week 13-16: Evaluator 기초

#### F1.9: Value 시스템 구현
- 상태: 📝 대기
- 브랜치: `feature/value-system`
- 우선순위: CRITICAL
- 작업:
  - [ ] src/evaluator/Value.h
    ```cpp
    class Value {
    private:
        TypeKind type;
        std::variant<
            int64_t,          // INTEGER
            double,           // FLOAT
            std::string,      // STRING
            bool,             // BOOLEAN
            std::nullptr_t    // NULL_TYPE
        > data;
    public:
        static Value createInteger(int64_t val);
        static Value createFloat(double val);
        static Value createString(const std::string& val);
        static Value createBoolean(bool val);
        static Value createNull();

        TypeKind getType() const;
        int64_t asInteger() const;
        double asFloat() const;
        std::string asString() const;
        bool asBoolean() const;
    };
    ```
- 테스트:
  - [ ] tests/value_test.cpp
- 완료 조건:
  - [ ] 모든 기본 타입 Value 생성
  - [ ] 타입 변환
  - [ ] 테스트 통과

#### F1.10: Environment 구현
- 상태: 📝 대기
- 브랜치: `feature/environment`
- 우선순위: CRITICAL
- 작업:
  - [ ] src/evaluator/Environment.h
    ```cpp
    class Environment {
    private:
        std::unordered_map<std::string, Value> store;
        std::shared_ptr<Environment> outer;  // 외부 스코프
    public:
        void set(const std::string& name, Value value);
        Value get(const std::string& name);
        bool exists(const std::string& name);

        std::shared_ptr<Environment> createEnclosed();
    };
    ```
- 테스트:
  - [ ] tests/environment_test.cpp
    - 변수 저장/조회
    - 스코프 체인
- 완료 조건:
  - [ ] 변수 저장/조회
  - [ ] 스코프 체인 동작
  - [ ] 테스트 통과

#### F1.11: 기본 Evaluator 구현
- 상태: 📝 대기
- 브랜치: `feature/evaluator`
- 우선순위: CRITICAL
- 작업:
  - [ ] src/evaluator/Evaluator.h
    ```cpp
    class Evaluator {
    private:
        std::shared_ptr<Environment> env;
    public:
        Value eval(Node* node);
        Value evalProgram(Program* program);
        Value evalExpression(Expression* expr);
        Value evalBinaryExpression(BinaryExpression* expr);
        Value evalIntegerLiteral(IntegerLiteral* lit);
        Value evalIdentifier(Identifier* ident);
    };
    ```
  - [ ] 기본 표현식 평가
    - 정수 리터럴
    - 식별자
    - 이항 연산 (+, -, *, /)
- 테스트:
  - [ ] tests/evaluator_test.cpp
    - `"5 + 3"` → 8
    - `"10 - 4"` → 6
    - 변수 할당/참조
- 완료 조건:
  - [ ] 기본 표현식 실행
  - [ ] 변수 동작
  - [ ] 테스트 통과

---

### Week 17-18: 반복문 구현

#### F1.12: 반복문 Parser & Evaluator
- 상태: 📝 대기
- 브랜치: `feature/loop-statements`
- 우선순위: HIGH
- 작업:
  - [ ] AST 노드 추가
    - RepeatStatement: `"10번 반복한다"`
    - RangeForStatement: `"i가 1부터 10까지"`
    - ForEachStatement: `"각각에 대해"`
  - [ ] Parser 구현
  - [ ] Evaluator 구현
- 테스트:
  - [ ] tests/loop_test.cpp
    ```ksj
    10번 반복한다 {
        출력("안녕")
    }

    i가 1부터 10까지 반복한다 {
        출력(i)
    }
    ```
- 완료 조건:
  - [ ] 3가지 반복문 동작
  - [ ] 범위 표현 통합
  - [ ] 테스트 통과

---

### Week 19-20: 조건문 & 함수

#### F1.13: 조건문 구현
- 상태: 📝 대기
- 브랜치: `feature/if-statement`
- 우선순위: CRITICAL
- 작업:
  - [ ] IfStatement AST
  - [ ] Parser 구현
  - [ ] Evaluator 구현
- 테스트:
  - [ ] tests/if_test.cpp
- 완료 조건:
  - [ ] if/else 동작
  - [ ] 테스트 통과

#### F1.14: 함수 구현
- 상태: 📝 대기
- 브랜치: `feature/function`
- 우선순위: CRITICAL
- 작업:
  - [ ] FunctionLiteral AST
  - [ ] CallExpression AST
  - [ ] Parser 구현
  - [ ] Evaluator 구현
    - 함수 정의
    - 함수 호출
    - 매개변수 바인딩
    - return 처리
- 테스트:
  - [ ] tests/function_test.cpp
    ```ksj
    함수 덧셈(a, b) {
        반환 a + b
    }

    결과 = 덧셈(5, 3)
    출력(결과)  # 8
    ```
- 완료 조건:
  - [ ] 함수 정의/호출 동작
  - [ ] 재귀 함수 지원
  - [ ] 테스트 통과

---

### Week 21-22: 표준 라이브러리

#### F1.15: 내장 함수 시스템
- 상태: 📝 대기
- 브랜치: `feature/builtin-functions`
- 우선순위: CRITICAL
- 작업:
  - [ ] src/evaluator/Builtin.h
    ```cpp
    using BuiltinFn = std::function<Value(std::vector<Value>)>;

    class Builtin {
    public:
        static void registerBuiltin(const std::string& name, BuiltinFn fn);
        static Value call(const std::string& name, std::vector<Value> args);
    };
    ```
  - [ ] 출력() 함수
  - [ ] 입력() 함수
  - [ ] 타입() 함수
  - [ ] 길이() 함수
- 테스트:
  - [ ] tests/builtin_test.cpp
- 완료 조건:
  - [ ] 기본 내장 함수 동작
  - [ ] UTF-8 한글 출력
  - [ ] 테스트 통과

---

### Week 23-24: 통합 & 예제

#### F1.16: REPL 구현
- 상태: 📝 대기
- 브랜치: `feature/repl`
- 우선순위: HIGH
- 작업:
  - [ ] src/repl/Repl.h/cpp
  - [ ] 대화형 입력
  - [ ] 실행 결과 출력
  - [ ] 에러 처리
- 완료 조건:
  - [ ] REPL 동작
  - [ ] 변수 유지

#### F1.17: 파일 실행
- 상태: 📝 대기
- 브랜치: `feature/file-execution`
- 우선순위: HIGH
- 작업:
  - [ ] main.cpp 구현
    - 파일 실행 모드
    - REPL 모드
    - 버전/도움말
  - [ ] 파일 읽기 및 실행
- 테스트:
  - [ ] examples/hello.ksj 실행
- 완료 조건:
  - [ ] `.ksj` 파일 실행
  - [ ] 에러 처리

#### F1.18: 예제 프로그램 작성
- 상태: 📝 대기
- 브랜치: `feature/examples`
- 우선순위: MEDIUM
- 작업:
  - [ ] examples/hello.ksj
  - [ ] examples/calculator.ksj
  - [ ] examples/fibonacci.ksj
  - [ ] examples/loop_demo.ksj
  - [ ] examples/josa_chain_demo.ksj
- 완료 조건:
  - [ ] 10개 이상 예제
  - [ ] 모든 예제 실행 성공

---

## Phase 2: 실용 기능

**목표**: 실제 프로그래밍 가능한 수준
**기간**: Month 4-6 (12주)

### F2.1: 배열 구현
- 상태: 📝 대기
- 브랜치: `feature/array`
- 우선순위: HIGH
- 작업:
  - [ ] 배열 리터럴 `[1, 2, 3]`
  - [ ] 인덱싱 `arr[0]`
  - [ ] 슬라이싱 `arr[0부터 5까지]`
  - [ ] 배열 메서드 (조사 활용)
- 완료 조건:
  - [ ] 배열 동작
  - [ ] 테스트 통과

### F2.2: 문자열 타입
- 상태: 📝 대기
- 브랜치: `feature/string`
- 우선순위: CRITICAL
- 작업:
  - [ ] 문자열 리터럴
  - [ ] 문자열 연결
  - [ ] 문자열 메서드
- 완료 조건:
  - [ ] 문자열 동작
  - [ ] UTF-8 처리

### F2.3: 실수 타입
- 상태: 📝 대기
- 브랜치: `feature/float`
- 우선순위: HIGH
- 작업:
  - [ ] 실수 리터럴
  - [ ] 실수 연산
  - [ ] 정수-실수 변환
- 완료 조건:
  - [ ] 실수 동작
  - [ ] 테스트 통과

### F2.4: 에러 처리 시스템
- 상태: 📝 대기
- 브랜치: `feature/error-system`
- 우선순위: HIGH
- 작업:
  - [ ] src/error/Error.h
  - [ ] 에러 클래스 계층
  - [ ] 스택 트레이스
  - [ ] 에러 위치 표시
- 완료 조건:
  - [ ] 에러 시스템 동작
  - [ ] 한글 에러 메시지

### F2.5: 한글 에러 메시지
- 상태: 📝 대기
- 브랜치: `feature/korean-error-messages`
- 우선순위: HIGH
- 작업:
  - [ ] src/error/ErrorMessage.h
  - [ ] 에러 메시지 한글화
  - [ ] 해결 방법 제안
  - [ ] 코드 컨텍스트 표시
- 완료 조건:
  - [ ] 모든 에러 한글 메시지
  - [ ] 교육적 피드백

### F2.6: 타입 검사
- 상태: 📝 대기
- 브랜치: `feature/type-checker`
- 우선순위: HIGH
- 작업:
  - [ ] src/types/TypeChecker.h
  - [ ] 타입 추론
  - [ ] 타입 검사
  - [ ] 타입 변환 함수
- 완료 조건:
  - [ ] 타입 불일치 감지
  - [ ] 명확한 에러 메시지

---

## Phase 3: 고급 기능

**목표**: 프로덕션 수준의 언어
**기간**: Month 7-12 (24주)

### F3.1: 가비지 컬렉터
- 상태: 📝 대기
- 브랜치: `feature/garbage-collector`
- 우선순위: MEDIUM
- 작업:
  - [ ] src/memory/GC.h
  - [ ] Mark & Sweep 알고리즘
  - [ ] 루트 객체 관리
- 완료 조건:
  - [ ] 자동 메모리 관리
  - [ ] 메모리 누수 없음

### F3.2: 바이트코드 컴파일러
- 상태: 📝 대기
- 브랜치: `feature/bytecode-compiler`
- 우선순위: MEDIUM
- 작업:
  - [ ] OpCode 정의
  - [ ] AST → Bytecode 컴파일러
  - [ ] VM 구현
- 완료 조건:
  - [ ] 바이트코드 실행
  - [ ] 성능 2-5배 향상

### F3.3: 모듈 시스템
- 상태: 📝 대기
- 브랜치: `feature/module-system`
- 우선순위: MEDIUM
- 작업:
  - [ ] `가져오기` 키워드
  - [ ] 모듈 검색
  - [ ] 순환 참조 방지
- 완료 조건:
  - [ ] 모듈 import/export
  - [ ] 네임스페이스

### F3.4: 시제 기반 비동기
- 상태: 📝 대기
- 브랜치: `feature/async-tense`
- 우선순위: LOW (혁신적이지만 복잡)
- 작업:
  - [ ] 시제 분석
  - [ ] Promise 구현
  - [ ] async/await 로직
- 완료 조건:
  - [ ] 비동기 동작
  - [ ] 에러 처리

### F3.5: 디버거
- 상태: 📝 대기
- 브랜치: `feature/debugger`
- 우선순위: LOW
- 작업:
  - [ ] 브레이크포인트
  - [ ] 단계별 실행
  - [ ] 변수 검사
- 완료 조건:
  - [ ] 디버거 동작

### F3.6: 온라인 플레이그라운드
- 상태: 📝 대기
- 브랜치: `feature/web-playground`
- 우선순위: LOW
- 작업:
  - [ ] WebAssembly 빌드
  - [ ] 웹 인터페이스
  - [ ] 코드 에디터
- 완료 조건:
  - [ ] 브라우저에서 실행

---

## 현재 작업 상태

### 진행 중
- 없음 (시작 전)

### 다음 작업
1. F0.1: 프로젝트 기본 설정
2. F0.2: 코딩 표준 문서
3. F0.3: 테스트 프레임워크 설정

### 최근 완료
- 프로젝트 디렉토리 구조 생성
- 문서 분석 및 설계 완료

---

## 마일스톤

### M1: Hello World (Week 24)
```ksj
출력("안녕, KingSejong!")
```

### M2: 핵심 차별화 (Week 12)
```ksj
숫자들 = [1, 2, 3, 4, 5]
결과 = 숫자들에서 짝수만_걸러내고 각각을_제곱한다
출력(결과)
```

### M3: 실용 수준 (Week 24)
```ksj
함수 피보나치(n: 정수) -> 정수 {
    만약 (n <= 1) {
        반환 n
    }
    반환 피보나치(n-1) + 피보나치(n-2)
}

i가 0부터 10까지 반복한다 {
    출력(피보나치(i))
}
```

### M4: 프로덕션 (Week 48)
- 가비지 컬렉터
- 바이트코드 컴파일러
- 모듈 시스템
- 온라인 플레이그라운드

---

## 메트릭스

### 코드 품질
- [ ] 테스트 커버리지 ≥ 80%
- [ ] 모든 기능 테스트 통과
- [ ] 메모리 누수 0
- [ ] Valgrind 클린

### 성능
- [ ] 기본 벤치마크 < 100ms
- [ ] 바이트코드 2-5배 빠름
- [ ] 메모리 사용량 < 50MB

### 문서화
- [ ] README 완성
- [ ] API 문서 완성
- [ ] 예제 30개 이상
- [ ] 튜토리얼 완성

---

**마지막 업데이트**: 2025-11-06
**다음 작업**: F0.1 프로젝트 기본 설정
