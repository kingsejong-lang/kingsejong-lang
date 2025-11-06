# 한글 프로그래밍 언어 구현 플랜

> **프로젝트**: KoreanLanguage 교육용 인터프리터
> **최종 수정**: 2025-11-06
> **상태**: ☐ 미시작 | 🚧 진행 중 | ✅ 완료 | ⏸️ 보류

---

## 📋 목차

1. [Phase 0: 기술 부채 해결 (1주)](#phase-0-기술-부채-해결-1주)
2. [Phase 1: MVP 구현 (12주)](#phase-1-mvp-구현-12주)
3. [Phase 2: 교육용 완성 (12주)](#phase-2-교육용-완성-12주)
4. [Phase 3: 고급 기능 (24주)](#phase-3-고급-기능-24주)

---

## Phase 0: 기술 부채 해결 (1주)

### 목표
기존 코드의 문제점 해결 및 개발 환경 정비

### 작업 목록

#### 0.1 코드 품질 개선

**0.1.1 메모리 관리 개선** ⏰ 2-3일
- ☐ Raw pointer를 스마트 포인터로 전환
  - `Parser.cpp` 의 `new`/`delete` 제거
  - `Repl.cpp` 의 메모리 관리 개선
  - `std::unique_ptr`, `std::shared_ptr` 사용
- ☐ AST 노드에 스마트 포인터 적용
  - `Expression*` → `std::unique_ptr<Expression>`
  - `Statement*` → `std::unique_ptr<Statement>`
- ☐ 메모리 누수 테스트 (Valgrind/AddressSanitizer)

**파일 위치**:
- `parser/Parser.cpp`
- `repl/Repl.cpp`
- `ast/Ast.h`

---

**0.1.2 타입 안정성 개선** ⏰ 1일
- ☐ `#define TokenType std::string` 제거
- ☐ `enum class TokenType` 정의
  ```cpp
  enum class TokenType {
      ILLEGAL,
      EOF_TOKEN,
      IDENTIFIER,
      INTEGER,
      // ...
  };
  ```
- ☐ 모든 파일에서 TokenType 사용 코드 수정

**파일 위치**: `token/Token.h`

---

**0.1.3 효율성 개선** ⏰ 1일
- ☐ `Parser.cpp:269-281` map::find() 중복 호출 수정
  ```cpp
  auto it = precedences.find(curToken.Type);
  if (it != precedences.end()) {
      return it->second;
  }
  ```
- ☐ 불필요한 문자열 복사 제거 (Pass by const reference)
- ☐ Move semantics 적용

**파일 위치**: `parser/Parser.cpp`

---

#### 0.2 기존 TODO 해결

**0.2.1 Parser TODO 수정** ⏰ 1일
- ☐ `Parser.cpp:172-176`: 할당문 우변 표현식 파싱 완성
- ☐ `Parser.cpp:186-192`: 반환문 표현식 파싱 완성
- ☐ `Parser.cpp:138-143`: EOF 처리 로직 개선
- ☐ `Parser.cpp:200-202`: 표현식 문장 EOF 처리

**파일 위치**: `parser/Parser.cpp`

---

**0.2.2 Lexer TODO 수정** ⏰ 1일
- ☐ `Lexer.cpp:90-91`: Space/Tab 혼용 처리 개선
- ☐ `Lexer.cpp:93`: 개행 문자 처리 완성
- ☐ `Lexer.cpp:77-88`: Space 4개 → Tab 로직 테스트

**파일 위치**: `lexer/Lexer.cpp`

---

#### 0.3 개발 환경 정비

**0.3.1 테스트 프레임워크 도입** ⏰ 2일
- ☐ GoogleTest 또는 Catch2 통합
- ☐ CMakeLists.txt에 테스트 타겟 추가
- ☐ 기본 테스트 작성
  - Lexer 토큰화 테스트
  - Parser 파싱 테스트
  - AST 생성 테스트

**새 파일**:
- `tests/LexerTest.cpp`
- `tests/ParserTest.cpp`
- `tests/AstTest.cpp`

---

**0.3.2 CI/CD 설정** ⏰ 1일
- ☐ GitHub Actions 워크플로우 생성
  - Windows, macOS, Linux 빌드
  - 자동 테스트 실행
  - 코드 포맷 검사 (clang-format)

**새 파일**: `.github/workflows/ci.yml`

---

**0.3.3 문서화** ⏰ 1일
- ☐ README.md 업데이트
  - 프로젝트 목적 및 목표
  - 빌드 방법
  - 기여 가이드
- ☐ CONTRIBUTING.md 작성
- ☐ 코드 주석 개선

---

## Phase 1: MVP 구현 (12주)

### 목표
기본 프로그래밍이 가능한 실행 가능 언어

---

### Week 1-2: Evaluator 기초 구조

**1.1 Value 타입 시스템** ⏰ 3일
- ☐ `Value` 클래스 설계
  ```cpp
  enum class ValueType {
      INTEGER,
      FLOAT,
      STRING,
      BOOLEAN,
      NULL_TYPE
  };

  class Value {
  public:
      ValueType type;
      // Union 또는 std::variant 사용
      // 타입별 값 저장
  };
  ```
- ☐ 타입별 값 생성/관리 함수
- ☐ 타입 변환 함수

**새 파일**: `evaluator/Value.h`, `evaluator/Value.cpp`

---

**1.2 Environment (변수 환경)** ⏰ 2일
- ☐ 변수 저장소 구현
  ```cpp
  class Environment {
  private:
      std::unordered_map<std::string, Value> store;
      std::shared_ptr<Environment> outer;  // 외부 스코프

  public:
      void set(const std::string& name, Value value);
      Value get(const std::string& name);
      bool exists(const std::string& name);
  };
  ```
- ☐ 스코프 체인 구현 (전역/지역)

**새 파일**: `evaluator/Environment.h`, `evaluator/Environment.cpp`

---

**1.3 Evaluator 클래스 골격** ⏰ 2일
- ☐ Evaluator 클래스 생성
  ```cpp
  class Evaluator {
  private:
      std::shared_ptr<Environment> env;

  public:
      Value eval(Node* node);
      Value evalProgram(Program* program);
      Value evalStatement(Statement* stmt);
      Value evalExpression(Expression* expr);
  };
  ```
- ☐ AST 노드 타입별 평가 함수 스텁 생성

**새 파일**: `evaluator/Evaluator.h`, `evaluator/Evaluator.cpp`

---

### Week 3-4: 기본 표현식 평가

**1.4 리터럴 평가** ⏰ 2일
- ☐ 정수 리터럴 평가
  ```cpp
  Value evalIntegerLiteral(IntegerLiteral* node) {
      return Value::createInteger(node->value);
  }
  ```
- ☐ 불린 리터럴 평가
- ☐ 테스트 작성

---

**1.5 산술 연산 평가** ⏰ 3일
- ☐ 중위 표현식 평가 (InfixExpression)
  - 덧셈, 뺄셈, 곱셈, 나눗셈
- ☐ 전위 표현식 평가 (PrefixExpression)
  - 음수 (`-x`)
  - NOT (`!x`)
- ☐ 타입 검사 및 에러 처리
- ☐ 테스트 작성

---

**1.6 비교 연산 평가** ⏰ 2일
- ☐ 같음/다름 (`==`, `!=`)
- ☐ 크기 비교 (`<`, `>`)
- ☐ 불린 결과 반환
- ☐ 테스트 작성

---

### Week 5-6: 문자열과 변수

**1.7 문자열 타입 추가** ⏰ 4일
- ☐ Lexer에 문자열 리터럴 토큰화 추가
  ```cpp
  // "안녕하세요", '안녕하세요' 지원
  std::string readString(char quote);
  ```
- ☐ Parser에 문자열 파싱 추가
- ☐ AST에 StringLiteral 노드 추가
- ☐ Evaluator에 문자열 평가 추가
- ☐ 문자열 연결 연산 (`"안녕" + "하세요"`)
- ☐ 테스트 작성

**파일 수정**:
- `lexer/Lexer.cpp`
- `parser/Parser.cpp`
- `ast/Ast.h`
- `evaluator/Evaluator.cpp`

---

**1.8 변수 할당 및 참조** ⏰ 3일
- ☐ 할당문 평가 (AssignStatement)
  ```cpp
  Value evalAssignStatement(AssignStatement* stmt) {
      Value value = eval(stmt->value);
      env->set(stmt->name->value, value);
      return value;
  }
  ```
- ☐ 식별자 평가 (Identifier)
  - 변수 조회
  - 미정의 변수 에러
- ☐ REPL에 변수 저장 기능 통합
- ☐ 테스트 작성

---

### Week 7-8: 제어 구조

**1.9 조건문 (if/else) 실행** ⏰ 4일
- ☐ IfExpression 평가
  ```cpp
  Value evalIfExpression(IfExpression* expr) {
      Value condition = eval(expr->Condition);

      if (isTruthy(condition)) {
          return eval(expr->Consequence);
      } else if (expr->Alternative != nullptr) {
          return eval(expr->Alternative);
      }
      return Value::createNull();
  }
  ```
- ☐ 불린 값 진위 판단 (Truthy/Falsy)
- ☐ 블록문 평가 (BlockStatement)
- ☐ 테스트 작성 (다양한 조건문 시나리오)

---

**1.10 While 반복문** ⏰ 3일
- ☐ Lexer에 `반복` 키워드 추가
- ☐ Parser에 WhileStatement 파싱 추가
  ```cpp
  WhileStatement* parseWhileStatement();
  ```
- ☐ AST에 WhileStatement 노드 추가
- ☐ Evaluator에 while 평가 추가
  ```cpp
  Value evalWhileStatement(WhileStatement* stmt) {
      while (isTruthy(eval(stmt->Condition))) {
          eval(stmt->Body);
      }
      return Value::createNull();
  }
  ```
- ☐ 무한 루프 방지 (최대 반복 횟수 설정)
- ☐ 테스트 작성

---

### Week 9-11: 함수

**1.11 함수 정의** ⏰ 5일
- ☐ Parser에 함수 파싱 추가
  ```
  함수 덧셈(a, b) {
    return a + b
  }
  ```
- ☐ AST에 FunctionLiteral 노드 추가
  ```cpp
  class FunctionLiteral : public Expression {
  public:
      std::vector<Identifier*> parameters;
      BlockStatement* body;
  };
  ```
- ☐ Value에 Function 타입 추가
- ☐ 함수를 변수로 저장
- ☐ 테스트 작성

**파일 수정**:
- `parser/Parser.cpp`
- `ast/Ast.h`
- `evaluator/Value.h`

---

**1.12 함수 호출** ⏰ 5일
- ☐ Parser에 함수 호출 파싱
  ```cpp
  CallExpression* parseCallExpression(Expression* function);
  ```
- ☐ AST에 CallExpression 노드 추가
- ☐ Evaluator에 함수 호출 평가
  ```cpp
  Value evalCallExpression(CallExpression* expr) {
      Value func = eval(expr->Function);
      std::vector<Value> args;
      for (auto arg : expr->Arguments) {
          args.push_back(eval(arg));
      }
      return applyFunction(func, args);
  }
  ```
- ☐ 함수 스코프 생성 (새 Environment)
- ☐ 인자 바인딩
- ☐ Return 처리
- ☐ 재귀 함수 지원
- ☐ 테스트 작성 (일반 호출, 재귀 등)

---

**1.13 Return 문 실행** ⏰ 2일
- ☐ ReturnStatement 평가
- ☐ Return 값을 함수 밖으로 전파
  - 예외 또는 특별한 Value 타입 사용
- ☐ 테스트 작성

---

### Week 12: 표준 라이브러리 기초

**1.14 내장 함수 시스템** ⏰ 2일
- ☐ 내장 함수 등록 메커니즘
  ```cpp
  class BuiltinFunction {
  public:
      using BuiltinFn = std::function<Value(std::vector<Value>)>;
      static void registerBuiltin(const std::string& name, BuiltinFn fn);
  };
  ```
- ☐ 전역 환경에 내장 함수 등록

**새 파일**: `evaluator/Builtin.h`, `evaluator/Builtin.cpp`

---

**1.15 출력 함수 (출력)** ⏰ 2일
- ☐ `출력()` 함수 구현
  ```cpp
  Value builtinPrint(std::vector<Value> args) {
      for (size_t i = 0; i < args.size(); i++) {
          std::cout << args[i].toString();
          if (i < args.size() - 1) std::cout << " ";
      }
      std::cout << std::endl;
      return Value::createNull();
  }
  ```
- ☐ UTF-8 한글 출력 보장 (Windows 콘솔 설정)
- ☐ 여러 인자 지원
- ☐ 테스트 작성

---

**1.16 입력 함수 (입력)** ⏰ 2일
- ☐ `입력()` 함수 구현
  ```cpp
  Value builtinInput(std::vector<Value> args) {
      if (args.size() > 0) {
          std::cout << args[0].toString();
      }
      std::string line;
      std::getline(std::cin, line);
      return Value::createString(line);
  }
  ```
- ☐ 프롬프트 메시지 지원
- ☐ 테스트 작성

---

**1.17 에러 처리 개선** ⏰ 2일
- ☐ 런타임 에러 클래스 정의
  ```cpp
  class RuntimeError : public std::exception {
  public:
      std::string message;
      Token token;  // 에러 위치
  };
  ```
- ☐ 에러 메시지 한글화
  - "변수 'x'가 정의되지 않았습니다."
  - "타입 오류: 정수와 문자열을 더할 수 없습니다."
- ☐ 에러 위치 표시
- ☐ 테스트 작성

---

**1.18 파일 실행 기능** ⏰ 2일
- ☐ main.cpp 인자 처리
  ```cpp
  if (argc > 1) {
      std::string filename = argv[1];
      executeFile(filename);
  } else {
      startREPL();
  }
  ```
- ☐ 파일 읽기 및 실행
- ☐ 에러 처리 및 종료 코드
- ☐ 테스트 작성

**파일 수정**: `main.cpp`

---

**1.19 MVP 통합 테스트** ⏰ 2일
- ☐ 종단간(E2E) 테스트 작성
  - 피보나치 수열
  - 팩토리얼
  - 구구단
  - 간단한 게임 (숫자 맞추기)
- ☐ 성능 벤치마크
- ☐ 메모리 누수 테스트

---

## Phase 2: 교육용 완성 (12주)

### 목표
초등/중등 교육에 사용 가능한 수준

---

### Week 13-14: 실수와 타입 시스템

**2.1 실수 타입 추가** ⏰ 4일
- ☐ Lexer에 실수 리터럴 토큰화 (`3.14`, `.5`, `1e-10`)
- ☐ Parser에 실수 파싱
- ☐ Value에 FLOAT 타입 추가
- ☐ 정수-실수 자동 변환
- ☐ 실수 연산 (나눗셈 결과 실수)
- ☐ 테스트 작성

---

**2.2 타입 변환 함수** ⏰ 3일
- ☐ `정수로(값)` 내장 함수
- ☐ `실수로(값)` 내장 함수
- ☐ `문자열로(값)` 내장 함수
- ☐ `타입(값)` 내장 함수
- ☐ 변환 실패 에러 처리
- ☐ 테스트 작성

---

### Week 15-16: 배열

**2.3 배열 리터럴** ⏰ 4일
- ☐ Lexer에 `[`, `]` 토큰 추가
- ☐ Parser에 배열 파싱
  ```cpp
  ArrayLiteral* parseArrayLiteral();
  ```
- ☐ AST에 ArrayLiteral 노드 추가
- ☐ Value에 ARRAY 타입 추가
- ☐ Evaluator에 배열 평가
- ☐ 테스트 작성

---

**2.4 배열 인덱싱** ⏰ 3일
- ☐ Parser에 인덱스 표현식 파싱 (`배열[0]`)
- ☐ AST에 IndexExpression 노드
- ☐ Evaluator에 인덱스 평가
  - 범위 검사
  - 음수 인덱스 (Python 스타일)
- ☐ 테스트 작성

---

**2.5 배열 메서드** ⏰ 4일
- ☐ `길이(배열)` - 배열 크기
- ☐ `추가(배열, 값)` - 끝에 추가
- ☐ `제거(배열, 인덱스)` - 요소 제거
- ☐ `정렬(배열)` - 정렬
- ☐ `역순(배열)` - 뒤집기
- ☐ 테스트 작성

---

### Week 17-18: For 반복문

**2.6 For 반복문 (C 스타일)** ⏰ 4일
- ☐ Parser에 for 문법 파싱
  ```
  반복 (i = 0; i < 10; i = i + 1) {
    출력(i)
  }
  ```
- ☐ AST에 ForStatement 노드
- ☐ Evaluator에 for 평가
- ☐ 테스트 작성

---

**2.7 For-each 반복문 (선택)** ⏰ 3일
- ☐ Parser에 for-each 문법
  ```
  반복 (각각 배열에서 항목) {
    출력(항목)
  }
  ```
- ☐ AST 노드 추가
- ☐ Evaluator 평가
- ☐ 테스트 작성

---

### Week 19-20: 표준 라이브러리 확장

**2.8 수학 함수** ⏰ 3일
- ☐ `절대값(x)`
- ☐ `제곱근(x)`
- ☐ `거듭제곱(x, y)`
- ☐ `올림(x)`, `내림(x)`, `반올림(x)`
- ☐ `최댓값(...)`, `최솟값(...)`
- ☐ 테스트 작성

---

**2.9 문자열 함수** ⏰ 4일
- ☐ `길이(문자열)`
- ☐ `대문자(문자열)`, `소문자(문자열)`
- ☐ `분리(문자열, 구분자)` → 배열
- ☐ `합치기(배열, 구분자)` → 문자열
- ☐ `찾기(문자열, 패턴)` → 인덱스
- ☐ `바꾸기(문자열, 이전, 이후)`
- ☐ `잘라내기(문자열, 시작, 끝)`
- ☐ 테스트 작성

---

### Week 21-22: REPL 개선

**2.10 여러 줄 입력** ⏰ 2일
- ☐ 불완전한 입력 감지 (괄호 미완성 등)
- ☐ 계속 입력 받기 (... 프롬프트)
- ☐ 테스트

---

**2.11 REPL 명령어** ⏰ 3일
- ☐ `:변수목록` - 정의된 변수 표시
- ☐ `:함수목록` - 정의된 함수 표시
- ☐ `:도움말` - 도움말 표시
- ☐ `:종료` - REPL 종료
- ☐ `:지우기` - 화면 지우기

---

**2.12 자동 완성 (선택)** ⏰ 3일
- ☐ GNU Readline 또는 linenoise 통합
- ☐ 키워드 자동 완성
- ☐ 변수명 자동 완성
- ☐ 명령 히스토리

---

### Week 23-24: 에러 메시지 및 패키징

**2.13 명확한 에러 메시지** ⏰ 4일
- ☐ 에러 컨텍스트 표시 (소스 코드 라인)
  ```
  오류: 변수 'x'가 정의되지 않았습니다.

  위치: 프로그램.한글 (3번째 줄)
     2 | 함수 계산() {
     3 |   return x + 5
              ^
     4 | }
  ```
- ☐ 해결 방법 제안
- ☐ 모든 에러 메시지 한글화
- ☐ 에러 카테고리별 정리

---

**2.14 크로스 플랫폼 패키징** ⏰ 4일
- ☐ Windows 실행 파일 빌드 및 테스트
  - UTF-8 콘솔 자동 설정
  - NSIS 설치 프로그램
- ☐ macOS 실행 파일 빌드
  - Universal Binary (arm64 + x64)
  - .dmg 패키지
  - Homebrew Formula
- ☐ Linux 실행 파일 빌드
  - .deb 패키지
  - .rpm 패키지
  - AppImage

---

**2.15 예제 프로그램 작성** ⏰ 3일
- ☐ `examples/` 디렉토리 구조 생성
- ☐ 카테고리별 예제 작성 (20-30개)
  - 01_변수와_출력/
  - 02_조건문/
  - 03_반복문/
  - 04_함수/
  - 05_배열/
  - 06_프로젝트/
- ☐ 각 예제에 주석 추가 (설명)
- ☐ README에 예제 목록 추가

---

## Phase 3: 고급 기능 (24주)

### 목표
실전 프로그래밍 입문 수준

---

### Week 25-28: 파일 I/O 및 모듈

**3.1 파일 읽기/쓰기** ⏰ 4주
- ☐ `파일읽기(경로)` 함수
- ☐ `파일쓰기(경로, 내용)` 함수
- ☐ `파일존재(경로)` 함수
- ☐ 에러 처리 (권한, 경로 오류 등)
- ☐ 테스트 작성

---

**3.2 모듈 시스템** ⏰ 4주
- ☐ `가져오기` 키워드 설계
  ```
  가져오기 수학도구
  가져오기 수학도구 (제곱, 제곱근)
  ```
- ☐ 모듈 파일 탐색 (표준 경로)
- ☐ 모듈 파싱 및 로드
- ☐ 네임스페이스 관리
- ☐ 순환 참조 방지
- ☐ 테스트 작성

---

### Week 29-32: 디버거

**3.3 디버거 기본** ⏰ 4주
- ☐ 브레이크포인트 시스템
- ☐ 단계별 실행 (Step Into, Step Over)
- ☐ 변수 검사
- ☐ 호출 스택 표시
- ☐ REPL 모드 통합
- ☐ 테스트

---

### Week 33-40: 성능 최적화

**3.4 바이트코드 컴파일러** ⏰ 8주
- ☐ Bytecode 명령어 집합 설계
  ```cpp
  enum class OpCode {
      LOAD_CONST,
      LOAD_VAR,
      STORE_VAR,
      ADD,
      SUB,
      MUL,
      DIV,
      JUMP,
      JUMP_IF_FALSE,
      CALL,
      RETURN,
  };
  ```
- ☐ AST → Bytecode 컴파일러
- ☐ VM (가상 머신) 구현
  - 스택 기반 실행
  - 명령어 디스패치
- ☐ 성능 비교 (Tree-Walking vs Bytecode)
- ☐ 테스트 작성

---

### Week 41-44: 교육 도구

**3.5 온라인 플레이그라운드** ⏰ 4주
- ☐ WebAssembly 빌드 설정 (Emscripten)
- ☐ 간단한 웹 인터페이스 (HTML/CSS/JS)
- ☐ 코드 에디터 통합 (Monaco Editor)
- ☐ 실행 결과 표시
- ☐ 코드 공유 기능 (선택)

---

**3.6 코드 시각화 도구 (선택)** ⏰ 4주
- ☐ 실행 흐름 시각화
- ☐ 변수 상태 표시
- ☐ 함수 호출 스택 시각화

---

### Week 45-48: 생태계

**3.7 표준 라이브러리 확장** ⏰ 2주
- ☐ JSON 파싱
- ☐ 날짜/시간
- ☐ 정규표현식 (선택)
- ☐ HTTP 클라이언트 (선택)

---

**3.8 문서화 및 튜토리얼** ⏰ 2주
- ☐ 공식 문서 사이트 (GitHub Pages)
- ☐ 언어 레퍼런스
- ☐ 튜토리얼 작성
- ☐ API 문서
- ☐ FAQ

---

**3.9 커뮤니티 구축** ⏰ 계속
- ☐ Discord 서버 또는 포럼
- ☐ 기여자 가이드
- ☐ 이슈 템플릿
- ☐ 코드 리뷰 프로세스

---

## 우선순위 레이블

- 🔥 **CRITICAL**: 없으면 동작 불가
- ⚡ **HIGH**: 기본 기능에 필수
- 🔧 **MEDIUM**: 사용성 개선
- 💡 **LOW**: Nice-to-have
- 🎨 **POLISH**: 완성도 향상

---

## 예상 타임라인

| Phase | 기간 | 완료 예정 | 주요 마일스톤 |
|-------|------|-----------|-------------|
| Phase 0 | 1주 | Week 1 | 기술 부채 해소 |
| Phase 1 | 12주 | Week 13 | MVP - 코드 실행 가능 |
| Phase 2 | 12주 | Week 25 | 교육용 언어 완성 |
| Phase 3 | 24주 | Week 49 | 고급 기능 및 생태계 |

**총 예상 기간**: 약 12개월 (1년)

---

## 개발 가이드

### 브랜치 전략
```
main - 안정 버전
develop - 개발 브랜치
feature/* - 기능 개발
fix/* - 버그 수정
```

### 커밋 메시지 규칙
```
feat: 새 기능 추가
fix: 버그 수정
refactor: 코드 리팩토링
test: 테스트 추가
docs: 문서 수정
style: 코드 포맷팅
```

### 코드 리뷰 체크리스트
- [ ] 테스트 작성 완료
- [ ] 메모리 누수 없음
- [ ] 에러 처리 완료
- [ ] 주석 및 문서화
- [ ] 성능 영향 확인

---

## 다음 단계

1. **Phase 0 시작**: 기술 부채 해결부터 시작
2. **작은 단위로 작업**: 각 작업을 2-3일 이내로 완료
3. **테스트 우선**: TDD 방식으로 개발
4. **지속적 통합**: 매일 빌드 및 테스트
5. **문서화**: 코드와 함께 문서 작성

**첫 번째 작업**: [0.1.1 메모리 관리 개선](#011-메모리-관리-개선--2-3일)부터 시작하세요!

---

**관련 문서**:
- [ANALYSIS.md](./ANALYSIS.md) - 프로젝트 분석 보고서
- [FEATURES.md](./FEATURES.md) - 기능 명세서
- [README.md](./README.md) - 프로젝트 개요
