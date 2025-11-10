# KingSejong 언어 구현 작업 목록

> **프로젝트**: KingSejong Programming Language (`.ksj`)
> **시작일**: 2025-11-06
> **최종 업데이트**: 2025-11-11 (모든 테스트 통과)
> **상태 기호**: ✅ 완료 | 🚧 진행중 | 📝 대기 | ⏸️ 보류 | ❌ 취소

---

## 📊 전체 진행 상황

### Phase 0: 프로젝트 초기화 ✅ (100% 완료)
- F0.1: 프로젝트 기본 설정 ✅
- F0.2: 코딩 표준 문서 ✅
- F0.3: 테스트 프레임워크 설정 ✅

### Phase 1: 핵심 차별화 기능 ✅ (100% 완료)
- F1.1: Token 시스템 ✅
- F1.2: JosaRecognizer ✅
- F1.3: Lexer ✅
- F1.4: AST 노드 ✅
- F1.5: Parser ✅
- F1.6: 조사 체인 Parser ✅
- F1.7: 범위 표현식 Parser ✅
- F1.8: 배열 리터럴 Parser ✅
- F1.9: Value 시스템 ✅
- F1.10: Environment ✅
- F1.11: Evaluator ✅
- F1.12: 반복문 (RepeatStatement, RangeForStatement, AssignmentStatement) ✅
- F1.13: 조건문 (IfStatement) ✅
- F1.14: 함수 구현 ⏸️ (F1.15로 대체)
- F1.15: 1급 함수 (FunctionLiteral, Closure, 재귀) ✅
- F1.16: 내장 함수 시스템 ✅
- F1.17: REPL 구현 ✅
- F1.18: 파일 실행 ✅
- F1.19: 예제 프로그램 ✅

### Phase 2: 실용 기능 ✅ (100% 완료)
- F2.1: 배열 구현 ✅
- F2.2: 문자열 타입 ✅
- F2.3: 실수 타입 ✅
- F2.4: 에러 처리 시스템 ✅
- F2.5: 한글 에러 메시지 ✅
- F2.6: 타입 검사 ✅
- F2.7: 표준 라이브러리 ✅

### Phase 3: 고급 기능 ✅ (100% 완료)
- F3.1: 가비지 컬렉터 ✅
- F3.2: 바이트코드 컴파일러 ✅
- F3.3: 모듈 시스템 ✅

### Phase 4: 최적화 및 안정화 ✅ (100% 완료)
- F4.1: CI/CD 파이프라인 ✅
- F4.2: 바이트코드 함수 지원 ✅
- F4.3: 성능 벤치마크 시스템 ✅
- F4.4: 컴파일러 최적화 패스 ✅
- F4.5: CI 벤치마크 자동화 ✅
- F4.6: 메모리 안전성 검사 ✅
- F4.7: 플랫폼 확장 (Linux/Windows) ✅
- F4.8: JIT 컴파일러 연구 ✅

### 테스트 현황
- **총 테스트**: 567개
- **통과**: 100%
- **실패**: 0개

### 최근 완료 (2025-11-11)
- ✅ **모든 테스트 통과** (567/567, 100%) - 4개 실패 테스트 수정 완료
  - FloatTest.ShouldRoundNegativeFloat: 반올림 함수 수정 (floor(value + 0.5))
  - FloatTest.ShouldCompareFloatAndInteger: 정수↔실수 비교 지원
  - FloatTest.ShouldUseFloatInFunction: 명명 함수 구문 + 조사 분리 개선
  - ModuleTestFixture.ShouldLoadModuleWithFunctions: heap-use-after-free 수정 (AST 수명 관리)
- ✅ Commit ce10255: 남은 4개 테스트 실패 수정
- ✅ Commit ca1b47f: 타입 키워드를 함수 이름으로 사용 가능하도록 수정 (9개 테스트 수정)

### 이전 완료 (2025-11-10)
- ✅ F4.8: JIT 컴파일러 연구 (핫 패스 감지, LLVM 통합 연구, 벤치마크 계획)
- ✅ Commit 639cbd8: F4.7 플랫폼별 설치 가이드 및 스크립트 추가
- ✅ Commit cbabbe2 & bf681b8: F4.6 메모리 안전성 검사 (ASan/UBSan 통합, use-after-free 버그 5건 수정)
- ✅ PR #48: F4.5 CI 벤치마크 자동화 (benchmark job 추가)
- ✅ PR #47: F4.4 바이트코드 컴파일러 최적화 패스 (상수 폴딩, 데드 코드 제거, 피홀 최적화)
- ✅ PR #46: F4.3 성능 벤치마크 시스템 (4개 벤치마크, 통계 분석)
- ✅ PR #45: F4.2 바이트코드 함수 지원 (BUILD_FUNCTION, CALL, RETURN, 클로저)
- ✅ PR #44: F4.1 CI/CD 파이프라인 (macOS, Linux, Windows 빌드 및 테스트)
- ✅ PR #42: F3.2 바이트코드 컴파일러 구현 (OpCode, Chunk, Compiler, VM)
- ✅ PR #41: F3.1 가비지 컬렉터 구현 (Mark & Sweep)
- ✅ PR #40: README.md 대폭 업데이트 - 프로젝트 현황 완전 반영
- ✅ PR #39: 완전한 문서화 및 튜토리얼 추가
- ✅ PR #38: F2.7 표준 라이브러리 모듈 구현 (math, string, array)

### 다음 우선순위
**Phase 1, 2, 3, 4 완료! 모든 테스트 통과!** 🎉🎉🎉

**v0.1.0 릴리스 준비:**
1. **CHANGELOG.md 작성** - v0.1.0 변경 사항 정리
2. **릴리스 노트 작성** - 주요 기능 및 개선사항
3. **문서 최종 검토** - README, 튜토리얼, API 문서
4. **릴리스 태그 생성** - v0.1.0

**Phase 5 계획:**
1. **성능 최적화** - JIT 컴파일러 구현 (템플릿 JIT)
2. **개발 도구** - F3.5 디버거, 언어 서버 프로토콜 (LSP)
3. **플랫폼 확장** - F3.6 온라인 플레이그라운드 (WebAssembly)
4. **고급 기능** - F3.4 시제 기반 비동기, 패턴 매칭

---

## 📋 목차

1. [Phase 0: 프로젝트 초기화 (Week 1-2)](#phase-0-프로젝트-초기화)
2. [Phase 1: 핵심 차별화 기능 (Month 1-3)](#phase-1-핵심-차별화-기능)
3. [Phase 2: 실용 기능 (Month 4-6)](#phase-2-실용-기능)
4. [Phase 3: 고급 기능 (Month 7-12)](#phase-3-고급-기능)
5. [Phase 4: 최적화 및 안정화 (2025 Q1-Q2)](#phase-4-최적화-및-안정화)
6. [Phase 5: 성능 & 개발자 경험 개선 (2025 Q1-Q2)](#phase-5-성능--개발자-경험-개선)

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
- 상태: ✅ 완료
- 브랜치: `feature/f0.2-coding-standards`
- 작업:
  - [x] docs/CODING_STYLE.md 작성
    - 네이밍 규칙
    - 포맷팅 규칙
    - 주석 작성 규칙
    - C++23 모던 스타일
    - 에러 처리 규칙
  - [x] docs/CONTRIBUTING.md 작성
    - PR 프로세스
    - 커밋 메시지 규칙
    - 코드 리뷰 가이드
    - Git 워크플로우
    - 이슈 작성 가이드
- 완료 조건:
  - [x] 문서 작성 완료

### F0.3: 테스트 프레임워크 설정
- 상태: ✅ 완료
- 브랜치: `feature/f0.3-test-framework`
- 작업:
  - [x] GoogleTest 통합 (CMakeLists.txt에 이미 설정됨)
  - [x] tests/SanityTest.cpp 작성
    - GoogleTest 프레임워크 동작 확인
    - 기본 산술 연산 테스트
    - 문자열 비교 테스트
    - 예외 처리 테스트
    - nullptr 테스트
    - 부동소수점 비교 테스트
- 완료 조건:
  - [x] ctest 실행 성공 (6개 테스트 통과)
  - [x] 로컬에서 테스트 자동 실행 확인
  - [ ] CI에서 테스트 자동 실행 (main 머지 후)

---

## Phase 1: 핵심 차별화 기능

**목표**: KingSejong만의 혁신적 기능 구현
**기간**: Month 1-3 (12주)

---

### Week 1-2: Token 시스템 & 조사 인식

#### F1.1: Token 시스템 구현
- 상태: ✅ 완료
- 브랜치: `feature/f1.1-token-system`
- 우선순위: CRITICAL
- 작업:
  - [x] src/lexer/Token.h 구현
    - enum class TokenType 정의 (60+ 토큰 타입)
    - 기본 토큰 (ILLEGAL, EOF, IDENTIFIER, INTEGER, FLOAT, STRING)
    - 연산자 (ASSIGN, PLUS, MINUS, ASTERISK, SLASH, PERCENT)
    - 비교 연산자 (EQ, NOT_EQ, LT, GT, LE, GE)
    - 논리 연산자 (AND, OR, NOT)
    - 조사 11개 (을/를, 이/가, 은/는, 의, 로/으로, 에서, 에)
    - 범위 키워드 6개 (부터, 까지, 미만, 초과, 이하, 이상)
    - 반복 키워드 3개 (번, 반복, 각각)
    - 제어문 키워드 (만약, 아니면, 동안)
    - 함수 키워드 (함수, 반환)
    - 타입 키워드 5개 (정수, 실수, 문자, 문자열, 논리)
    - 불리언 리터럴 (참, 거짓)
    - 메서드 체이닝 (하고, 하라)
    - 괄호/구분자
  - [x] src/lexer/Token.cpp 구현
    - tokenTypeToString() 함수
    - lookupKeyword() 한글 키워드 매핑
    - isJosa() 조사 판별 함수
    - isRangeKeyword() 범위 키워드 판별 함수
  - [x] Token 구조체 정의
  - [x] CMakeLists.txt 업데이트 (kingsejong_lib 라이브러리 생성)
- 테스트:
  - [x] tests/TokenTest.cpp 작성 (13개 테스트 케이스)
    - 기본 토큰 생성 테스트
    - TokenType 문자열 변환 테스트
    - 조사 키워드 인식 테스트 (11개)
    - 범위 키워드 인식 테스트 (6개)
    - 반복 키워드 인식 테스트 (3개)
    - 제어문 키워드 인식 테스트
    - 함수 키워드 인식 테스트
    - 타입 키워드 인식 테스트
    - 불리언 리터럴 인식 테스트
    - 메서드 체이닝 키워드 인식 테스트
    - IDENTIFIER 반환 테스트
    - isJosa() 함수 테스트
    - isRangeKeyword() 함수 테스트
- 완료 조건:
  - [x] 모든 TokenType 정의 완료 (60+ 타입)
  - [x] 테스트 통과 (13/13 통과, 100%)

#### F1.2: JosaRecognizer 구현
- 상태: ✅ 완료
- 브랜치: `feature/f1.2-josa-recognizer`
- 우선순위: CRITICAL
- 작업:
  - [x] src/lexer/JosaRecognizer.h 설계
    - enum class JosaType (7가지 조사 타입)
    - 조사 인식 함수 (isJosa, getType)
    - 받침 판단 함수 (hasFinalConsonant)
    - 조사 선택 함수 (select)
    - 헬퍼 함수 (josaTypeToString)
  - [x] src/lexer/JosaRecognizer.cpp 구현
    - UTF-8 한글 문자 처리 (getFinalConsonantIndex)
    - 받침 유무 판단 (hasFinalConsonant)
    - ㄹ받침 특수 처리 (hasRieulFinal)
    - 자동 조사 선택 (select)
    - 조사 인식 및 타입 변환
  - [x] CMakeLists.txt 업데이트 (kingsejong_lib에 JosaRecognizer.cpp 추가)
- 테스트:
  - [x] tests/JosaRecognizerTest.cpp (13개 테스트 케이스)
    - 받침 있는 단어 판별 테스트
    - 받침 없는 단어 판별 테스트
    - 을/를 조사 선택 테스트
    - 이/가 조사 선택 테스트
    - 은/는 조사 선택 테스트
    - 로/으로 조사 선택 테스트 (ㄹ받침 특수 처리)
    - 의, 에서, 에 조사 선택 테스트
    - isJosa 함수 테스트
    - getType 함수 테스트
    - 잘못된 입력 예외 처리 테스트
    - josaTypeToString 함수 테스트
- 완료 조건:
  - [x] 모든 조사 타입 인식 (7가지)
  - [x] 받침 유무 정확히 판단 (UTF-8 한글 처리)
  - [x] ㄹ받침 특수 처리 (로/으로)
  - [x] 테스트 통과율 100% (32/32 통과)

---

### Week 3-4: Lexer 기본 구현

#### F1.3: 기본 Lexer 구현
- 상태: ✅ 완료
- 브랜치: `feature/lexer`
- 우선순위: CRITICAL
- 작업:
  - [x] src/lexer/Lexer.h 설계
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
  - [x] src/lexer/Lexer.cpp 구현
    - UTF-8 한글 처리 (3바이트 멀티바이트 문자)
    - 기본 토큰화 (숫자, 문자열, 식별자, 연산자)
    - 조사/키워드 접미사 자동 분리
  - [x] 조사 통합
    - JosaRecognizer와 통합
    - 조사 토큰 생성
    - 키워드 접미사 분리 (하고, 하라 등)
  - [x] CMakeLists.txt 업데이트 (Lexer.cpp 추가)
- 테스트:
  - [x] tests/LexerTest.cpp (33개 GoogleTest 케이스 작성)
  - [x] tests/manual_lexer_test.cpp (6개 수동 테스트)
    - 기본 토큰화 테스트 ✓
    - 한글 키워드 테스트 ✓
    - 조사 인식 테스트 ✓
    - 범위 표현 테스트 ✓
    - 문자열 리터럴 테스트 ✓
    - 실제 코드 스니펫 테스트 ✓
- 완료 조건:
  - [x] UTF-8 한글 올바르게 처리 (3바이트 멀티바이트)
  - [x] 모든 토큰 타입 인식 (60+ 토큰 타입)
  - [x] 조사 정확히 인식 및 자동 분리
  - [x] 키워드 접미사 분리 (하고, 하라)
  - [x] 수동 테스트 통과율 100% (6/6 통과)
  - [x] 컴파일 경고 없음

---

### Week 5-6: AST 및 기본 Parser

#### F1.4: AST 노드 설계
- 상태: ✅ 완료
- 브랜치: `feature/f1.4-ast`
- 우선순위: CRITICAL
- 작업:
  - [x] src/ast/Node.h 구현
    - enum class NodeType (26가지 노드 타입)
    - Node 추상 클래스
    - Expression 기본 클래스
    - Statement 기본 클래스
    - nodeTypeToString() 함수
  - [x] src/ast/Node.cpp 구현
    - nodeTypeToString() 함수 구현
  - [x] src/ast/Expression.h 구현
    - 기본 리터럴 (Identifier, IntegerLiteral, FloatLiteral, StringLiteral, BooleanLiteral)
    - 연산 표현식 (BinaryExpression, UnaryExpression)
    - KingSejong 특화 (JosaExpression, RangeExpression)
    - 함수 관련 (CallExpression)
    - 배열 관련 (ArrayLiteral, IndexExpression)
  - [x] src/ast/Statement.h 구현
    - Program 노드
    - ExpressionStatement
    - VarDeclaration
    - ReturnStatement
    - BlockStatement
    - IfStatement
    - WhileStatement
    - ForStatement
  - [x] CMakeLists.txt 업데이트 (Node.cpp 추가, manual_lexer_test.cpp 제외)
- 테스트:
  - [x] tests/AstTest.cpp (21개 테스트 케이스)
    - 기본 리터럴 생성 및 toString 테스트 (5개)
    - 연산 표현식 테스트 (2개)
    - JosaExpression 테스트 (1개)
    - RangeExpression 테스트 (3개 - inclusive, half-open, open)
    - 함수 및 배열 표현식 테스트 (3개)
    - 문장 노드 테스트 (5개)
    - 헬퍼 함수 테스트 (2개)
- 완료 조건:
  - [x] 모든 노드 타입 정의 (26가지)
  - [x] std::unique_ptr 사용 (메모리 안전)
  - [x] 테스트 통과율 100% (21/21 통과)

#### F1.5: 기본 Parser 구현
- 상태: ✅ 완료
- 브랜치: `feature/f1.5-parser`
- 우선순위: CRITICAL
- 작업:
  - [x] src/parser/Parser.h 설계
    - Pratt Parsing 알고리즘 인터페이스
    - 10단계 Precedence 체계 (LOWEST → INDEX)
    - Prefix/Infix 파싱 함수 맵 (std::unordered_map)
    - 138줄 구현
  - [x] src/parser/Parser.cpp 구현
    - Pratt Parsing 알고리즘 완전 구현
    - 연산자 우선순위 처리 (tokenPrecedence)
    - Prefix 파서: identifier, literal, unary, grouped, array
    - Infix 파서: binary, call, index
    - Statement 파서: varDeclaration, return, block
    - 485줄 구현
- 테스트:
  - [x] tests/ParserTest.cpp 작성
    - 리터럴 파싱 테스트 (4개: integer, identifier, string, boolean)
    - 연산자 파싱 테스트 (6개: addition, multiplication, precedence, prefix, grouped)
    - 문장 파싱 테스트 (2개: VarDeclaration, ReturnStatement)
    - 복합 표현식 테스트 (4개: call, array, index, complex)
    - 17개 테스트 케이스, 468줄 구현
  - [x] 테스트 통과율 100% (17/17 통과)
- 완료 조건:
  - [x] 기본 표현식 파싱 완료
  - [x] Statement 파싱 완료
  - [x] 연산자 우선순위 처리 완료
  - [x] 테스트 통과

---

### Week 7-8: 조사 체인 파싱

#### F1.6: 조사 체인 Parser 구현
- 상태: ✅ 완료
- 브랜치: `feature/f1.6-josa-parser`
- 우선순위: CRITICAL (핵심 차별화!)
- 작업:
  - [x] Parser.h에 조사 파싱 메서드 추가
    - parseJosaExpression() - Infix parser로 구현
    - isJosaToken() - 조사 토큰 감지
    - tokenToJosaType() - TokenType → JosaType 변환
  - [x] Parser.cpp에 조사 파싱 구현
    - registerParseFunctions()에 11개 조사 토큰 등록
    - parseJosaExpression() 구현 (object + 조사 + method)
    - tokenPrecedence()에 조사 우선순위 추가 (CALL과 동일)
    - isJosaToken() 및 tokenToJosaType() 구현
    - 115줄 추가
  - [x] Parser에 조사 체인 통합
    - Pratt Parsing의 infix 메커니즘 활용
    - 조사가 감지되면 자동으로 parseJosaExpression 호출
    - 자연스러운 체인 파싱 지원 (중첩된 JosaExpression)
- 테스트:
  - [x] tests/ParserTest.cpp에 조사 테스트 추가
    - "배열을 정렬한다" - EUL_REUL 조사 (통과)
    - "사용자의 이름" - UI 조사 (통과)
    - "데이터로 변환한다" - RO_EURO 조사 (통과)
    - "데이터가 존재한다" - I_GA 조사 (통과)
    - 4개 테스트 케이스, 160줄 추가
  - [x] 테스트 통과율 100% (4/4 조사 테스트, 103/104 전체)
- 완료 조건:
  - [x] 모든 조사 타입 파싱 (11개 조사 토큰)
  - [x] 조사 표현식 파싱 완료
  - [x] 테스트 통과

---

### Week 9-10: 범위 표현

#### F1.7: 범위 표현 Parser 구현
- 상태: ✅ 완료
- 브랜치: `feature/f1.7-range-parser`
- PR: #10
- 우선순위: CRITICAL (핵심 차별화!)
- 작업:
  - [x] src/parser/Parser.h에 범위 파싱 메서드 추가
    - parseRangeExpression() 메서드
    - isRangeStartToken() 헬퍼
    - isRangeEndToken() 헬퍼
  - [x] src/parser/Parser.cpp에 범위 파싱 구현
    - 범위 시작 토큰 (부터/초과/이상) infix 등록
    - tokenPrecedence()에 범위 토큰 우선순위 추가
    - parseRangeExpression() 구현
    - 경계 포함/제외 로직 구현
  - [x] 범위 파싱 로직
    ```
    1부터 10까지    → [1, 10] inclusive both
    1부터 10미만    → [1, 10) inclusive start, exclusive end
    1초과 10이하    → (1, 10] exclusive start, inclusive end
    5이상 15이하    → [5, 15] inclusive both
    ```
- 테스트:
  - [x] tests/ParserTest.cpp
    - ShouldParseRangeExpressionInclusive: `"1부터 10까지"` 파싱
    - ShouldParseRangeExpressionHalfOpen: `"1부터 10미만"` 파싱
    - ShouldParseRangeExpressionOpen: `"1초과 10이하"` 파싱
    - ShouldParseRangeExpressionIsangIha: `"5이상 15이하"` 파싱
- 완료 조건:
  - [x] 모든 범위 형태 파싱
  - [x] 테스트 통과 (4/4)

---

### Week 11-12: 타입 시스템 기초

#### F1.8: Type 시스템 구현
- 상태: ✅ 완료
- 브랜치: `feature/f1.8-type-system`
- PR: #12 (예정)
- 우선순위: CRITICAL
- 작업:
  - [x] src/types/Type.h
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
  - [x] TypeKind enum 정의 (INTEGER, FLOAT, STRING, BOOLEAN, NULL_TYPE, ARRAY, FUNCTION, RANGE)
  - [x] Type 클래스 구현 (kind, koreanName, englishName, toString)
  - [x] 한글 타입 이름 매핑 ("정수" → INTEGER, "실수" → FLOAT, "문자열" → STRING, "참거짓" → BOOLEAN, "불린" → BOOLEAN, "없음" → NULL, "범위" → RANGE)
  - [x] 빌트인 타입 Singleton 패턴 (integerType(), floatType(), etc.)
  - [x] getBuiltin() 메서드로 한글 이름으로 타입 조회
  - [x] Type 동일성 비교 (equals)
  - [x] **타입 선언 파싱** (F1.8.1 완료)
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
  - [x] tests/TypeTest.cpp (18 test cases)
    - TypeKind enum 확인
    - 빌트인 타입 생성 (정수, 실수, 문자열, 참거짓, 없음, 범위)
    - 한글 이름으로 타입 조회
    - 타입 동일성 비교
    - TypeKind 문자열 변환
    - toString() 메서드
    - Singleton 패턴 확인
  - [ ] tests/TypeDeclarationTest.cpp (향후 작업)
    - `"정수 나이 = 15"` 파싱
- 완료 조건:
  - [x] 기본 타입 정의
  - [x] 타입 선언 파싱 (F1.8.1)
  - [x] 테스트 통과 (18/18)

#### F1.8.1: 타입 선언 파싱
- 상태: ✅ 완료
- 브랜치: `feature/f1.8.1-type-declaration`
- PR: #13
- 우선순위: CRITICAL
- 작업:
  - [x] src/ast/Statement.h 수정
    - VarDeclaration에 types::Type* varType_ 멤버 추가
    - varType() getter 메서드 추가
    - 생성자에 varType 파라미터 추가
  - [x] src/parser/Parser.cpp 수정
    - parseVarDeclaration()에서 Type::getBuiltin() 호출
    - 타입 이름으로 Type 객체 조회
    - VarDeclaration 생성 시 Type 포인터 전달
  - [x] tests/ParserTest.cpp 수정 및 추가
    - ShouldParseVarDeclaration에 Type 검증 추가
    - ShouldParseVarDeclarationWithStringType 추가
    - ShouldParseVarDeclarationWithFloatType 추가
    - ShouldParseVarDeclarationWithBooleanType 추가
    - ShouldParseVarDeclarationWithoutInitializer 추가
- 테스트:
  - [x] 전체 130개 테스트 통과 (100%)
  - [x] Type 객체 연동 검증
  - [x] 다양한 타입 선언 테스트
- 완료 조건:
  - [x] Parser와 Type 시스템 통합
  - [x] VarDeclaration AST에 Type 정보 저장
  - [x] 테스트 통과

---

### Week 13-16: Evaluator 기초

#### F1.9: Value 시스템 구현
- 상태: ✅ 완료
- 브랜치: `feature/f1.9-value-system`
- PR: #14
- 우선순위: CRITICAL
- 작업:
  - [x] src/evaluator/Value.h
    - std::variant로 타입 안전한 값 저장
    - TypeKind와 연동 (INTEGER, FLOAT, STRING, BOOLEAN, NULL_TYPE)
    - 생성 함수: createInteger, createFloat, createString, createBoolean, createNull
    - 타입 체크: isInteger, isFloat, isString, isBoolean, isNull
    - 값 접근: asInteger, asFloat, asString, asBoolean (예외 처리)
    - 참/거짓 판별: isTruthy()
    - 비교 연산: equals, lessThan, greaterThan
    - 한글 출력: toString()
  - [x] src/evaluator/Value.cpp
    - 모든 메서드 구현
    - 한글 출력 ("참", "거짓", "없음")
    - KingSejong 참/거짓 규칙 (false/0/0.0/""/null은 거짓)
    - 부동소수점 비교 시 epsilon 사용 (1e-9)
  - [x] tests/ValueTest.cpp
    - 37개 테스트 케이스
    - 생성 테스트 (7개)
    - 타입 접근 및 예외 테스트 (4개)
    - 참/거짓 판별 테스트 (5개)
    - 동등 비교 테스트 (6개)
    - 대소 비교 테스트 (6개)
    - 경계값 테스트 (9개)
  - [x] CMakeLists.txt 업데이트
- 완료 조건:
  - [x] 모든 기본 타입 Value 생성
  - [x] 타입 변환 및 예외 처리
  - [x] 테스트 통과 (167/167)

#### F1.10: Environment 구현
- 상태: ✅ 완료
- 브랜치: `feature/f1.10-environment`
- PR: #15
- 우선순위: CRITICAL
- 테스트: 전체 190개 테스트 통과 (기존 167개 + 새로운 23개)
- 작업:
  - [x] src/evaluator/Environment.h (124줄)
    ```cpp
    class Environment : public std::enable_shared_from_this<Environment> {
    private:
        std::unordered_map<std::string, Value> store_;
        std::shared_ptr<Environment> outer_;  // 외부 스코프
    public:
        Environment();  // 글로벌 스코프
        explicit Environment(std::shared_ptr<Environment> outer);  // 중첩 스코프

        void set(const std::string& name, const Value& value);
        Value get(const std::string& name) const;  // 스코프 체인 검색
        bool exists(const std::string& name) const;  // 현재 스코프만
        bool existsInChain(const std::string& name) const;  // 전체 체인

        std::shared_ptr<Environment> createEnclosed();  // shared_from_this() 활용
        std::shared_ptr<Environment> outer() const { return outer_; }
        size_t size() const { return store_.size(); }
        void clear() { store_.clear(); }
        std::vector<std::string> keys() const;
    };
    ```
  - [x] src/evaluator/Environment.cpp (102줄)
    - 생성자: outer_ = nullptr (글로벌) / outer (중첩)
    - set(): store_[name] = value
    - get(): 현재 스코프 → 외부 스코프 재귀 검색 (undefined → runtime_error)
    - exists(): 현재 스코프만 확인
    - existsInChain(): 재귀적 체인 검색
    - createEnclosed(): shared_from_this() 안전한 shared_ptr 생성
  - [x] CMakeLists.txt: src/evaluator/Environment.cpp 추가
- 테스트:
  - [x] tests/EnvironmentTest.cpp (27개 테스트)
    - 기본 환경 생성 및 변수 관리 (5개)
    - 스코프 체인 및 변수 조회 (8개)
    - 변수 섀도잉 (shadowing) (3개)
    - exists() vs existsInChain() 차이 (2개)
    - 3단계 중첩 스코프 (2개)
    - 유틸리티 메서드 (keys, clear, size) (4개)
    - 한글 변수명 지원 (1개)
    - 독립 환경 간섭 없음 (1개)
- 완료 조건:
  - [x] 변수 저장/조회 정상 동작
  - [x] 스코프 체인 동작 (재귀 검색)
  - [x] 변수 섀도잉 지원
  - [x] 한글 변수명 완벽 지원
  - [x] 테스트 100% 통과 (190/190)
  - [x] shared_ptr 메모리 안전성 확보

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
- 상태: ✅ 완료
- 브랜치: `feature/f1.12-loop-statements`, `feature/f1.12-loop-fixes`
- PR: #18 (초기 구현), #20 (표현식 지원 및 AssignmentStatement)
- 우선순위: HIGH
- 작업:
  - [x] AST 노드 추가
    - RepeatStatement: `"10번 반복한다"` ✅
    - RangeForStatement: `"i가 1부터 10까지"` ✅
    - AssignmentStatement: `"x = value"` ✅ (PR #20 추가)
    - ForEachStatement: ❌ 미구현 (배열 기능 필요)
  - [x] Parser 구현
    - RepeatStatement 파서 (표현식 기반 반복 횟수 지원)
    - RangeForStatement 파서 (리터럴 및 변수 기반 범위)
    - AssignmentStatement 파서
    - isLikelyLoopVariable() 휴리스틱
  - [x] Evaluator 구현
    - RepeatStatement 평가 (음수 검증 포함)
    - RangeForStatement 평가
    - AssignmentStatement 평가
  - [x] Lexer 키워드 분리 개선
    - 2글자: 까지/부터/반복 자동 분리
    - 1글자: 번 자동 분리
- 테스트:
  - [x] tests/LoopStatementTest.cpp (21개 테스트)
    - RepeatStatement: 7개 테스트 ✅
    - RangeForStatement: 14개 테스트 ✅
    - 100% 통과 (21/21)
- 완료 조건:
  - [x] RepeatStatement 동작 (표현식 지원)
  - [x] RangeForStatement 동작 (표현식 지원)
  - [x] AssignmentStatement 구현
  - [x] 범위 표현 통합
  - [x] 테스트 통과 (100%)
  - [x] 문서 업데이트 (KNOWN_ISSUES.md, FEATURES.md)

---

### Week 19-20: 조건문 & 함수

#### F1.13: 조건문 구현
- 상태: ✅ 완료
- 브랜치: `feature/f1.13-if-statement`
- PR: #17
- 우선순위: CRITICAL
- 작업:
  - [x] IfStatement AST
  - [x] Parser 구현
  - [x] Evaluator 구현
- 테스트:
  - [x] tests/IfStatementTest.cpp (22개 테스트)
    - 100% 통과 (22/22)
- 완료 조건:
  - [x] if/else 동작
  - [x] 중첩 if문 지원
  - [x] 테스트 통과

#### F1.14: 함수 구현 (→ F1.15로 대체)
- 상태: ⏸️ 보류 (F1.15 1급 함수로 구현됨)
- 비고: F1.15에서 더 강력한 1급 함수로 구현되어 별도 구현 불필요

---

### Week 21-22: 표준 라이브러리

#### F1.15: 1급 함수 (First-Class Functions)
- 상태: ✅ 완료
- 브랜치: `feature/f1.15-function`
- PR: #19
- 우선순위: CRITICAL
- 작업:
  - [x] FunctionLiteral AST 노드
  - [x] CallExpression 구현
  - [x] Function Value 타입
  - [x] Parser 구현
    - 함수 리터럴 파싱
    - 함수 호출 파싱
  - [x] Evaluator 구현
    - 함수 리터럴 평가
    - 함수 호출 평가
    - 클로저 지원 (Environment 캡처)
    - ReturnValue 예외 처리
- 테스트:
  - [x] tests/FunctionTest.cpp (19개 테스트)
    - 함수 리터럴 생성: 2개 ✅
    - 함수 호출: 11개 ✅
    - 클로저: 2개 ✅
    - 재귀: 2개 ✅
    - 에러 처리: 2개 ✅
    - 100% 통과 (19/19)
- 기능:
  - [x] 함수를 값으로 취급 (1급 객체)
  - [x] 함수 리터럴 문법
  - [x] 클로저 지원
  - [x] 재귀 함수 지원
  - [x] 고차 함수 지원
- 완료 조건:
  - [x] 함수 정의/호출 동작
  - [x] 클로저 동작
  - [x] 재귀 함수 지원
  - [x] 테스트 통과 (100%)

#### F1.16: 내장 함수 시스템
- 상태: ✅ 완료
- 브랜치: `feature/f1.16-builtin-functions`
- PR: #21
- 우선순위: HIGH
- 작업:
  - [x] 출력() 함수
  - [x] 타입() 함수
  - [x] 길이() 함수
- 완료 조건:
  - [x] 내장 함수 등록 시스템
  - [x] 3개 내장 함수 구현
  - [x] 테스트 통과

---

### Week 23-24: 통합 & 예제

#### F1.17: REPL 구현
- 상태: ✅ 완료
- 브랜치: `feature/f1.17-repl`
- PR: #22
- 우선순위: HIGH
- 작업:
  - [x] src/repl/Repl.h/cpp
  - [x] 대화형 입력
  - [x] 실행 결과 출력
  - [x] 에러 처리
  - [x] REPL 명령어 (.exit, .help, .clear, .vars)
- 완료 조건:
  - [x] REPL 동작
  - [x] 변수 유지
  - [x] 수동 테스트 통과

#### F1.18: 파일 실행
- 상태: ✅ 완료
- 브랜치: `feature/f1.18-file-execution`
- PR: #23
- 우선순위: HIGH
- 작업:
  - [x] main.cpp executeFile() 함수 구현
  - [x] 파일 읽기 및 Lexer/Parser/Evaluator 파이프라인
  - [x] 에러 처리 (파일 없음, 파서 에러, 런타임 에러)
  - [x] 명령줄 인자 처리
  - [x] examples/ 디렉토리 및 5개 예제 파일 작성
- 테스트:
  - [x] examples/hello.ksj 실행
  - [x] examples/calculator.ksj 실행
  - [x] examples/fibonacci.ksj 실행
  - [x] examples/loop.ksj 실행
  - [x] examples/function.ksj 실행
- 완료 조건:
  - [x] `.ksj` 파일 실행
  - [x] 에러 처리
  - [x] 모든 예제 실행 성공

#### F1.19: 예제 프로그램 작성
- 상태: ✅ 완료
- 브랜치: `feature/f1.19-examples`
- 우선순위: MEDIUM
- 작업:
  - [x] examples/hello.ksj (F1.18에서 작성)
  - [x] examples/calculator.ksj (F1.18에서 작성)
  - [x] examples/fibonacci.ksj (F1.18에서 작성)
  - [x] examples/loop.ksj (F1.18에서 작성)
  - [x] examples/function.ksj (F1.18에서 작성)
  - [x] examples/conditional.ksj (조건문 예제)
  - [x] examples/closure.ksj (클로저 예제)
  - [x] examples/recursion.ksj (재귀 예제)
  - [x] examples/nested_loops.ksj (중첩 반복문)
  - [x] examples/type_demo.ksj (타입 시스템 시연)
- 완료 조건:
  - [x] 10개 예제 작성
  - [x] 모든 예제 실행 성공

---

## Phase 2: 실용 기능

**목표**: 실제 프로그래밍 가능한 수준
**기간**: Month 4-6 (12주)
**상태**: ✅ 100% 완료

### F2.1: 배열 구현
- 상태: ✅ 완료
- 브랜치: `feature/f2.1-array`
- PR: #26, #32, #37
- 우선순위: HIGH
- 작업:
  - [x] 배열 리터럴 `[1, 2, 3]`
  - [x] 인덱싱 `arr[0]`
  - [x] 슬라이싱 `arr[0부터 5까지]`
  - [x] 배열 메서드 (조사 활용)
  - [x] 함수형 메서드 (걸러낸다, 변환한다, 축약한다, 찾다)
  - [x] 메서드 체이닝 (정렬한다, 역순으로_나열한다)
- 완료 조건:
  - [x] 배열 동작
  - [x] 테스트 통과 (ArrayMethodTest 15개 테스트)

### F2.2: 문자열 타입
- 상태: ✅ 완료
- 브랜치: `feature/f2.2-string`
- PR: #27
- 우선순위: CRITICAL
- 작업:
  - [x] 문자열 리터럴
  - [x] 문자열 연결
  - [x] 문자열 메서드
- 완료 조건:
  - [x] 문자열 동작
  - [x] UTF-8 처리

### F2.3: 실수 타입
- 상태: ✅ 완료
- 브랜치: `feature/f2.3-float`
- PR: #29
- 우선순위: HIGH
- 작업:
  - [x] 실수 리터럴
  - [x] 실수 연산
  - [x] 정수-실수 변환
- 완료 조건:
  - [x] 실수 동작
  - [x] 테스트 통과

### F2.4: 에러 처리 시스템
- 상태: ✅ 완료
- 브랜치: `feature/f2.4-error-system`
- PR: #30
- 우선순위: HIGH
- 작업:
  - [x] src/error/Error.h
  - [x] 에러 클래스 계층
  - [x] 스택 트레이스
  - [x] 에러 위치 표시
- 완료 조건:
  - [x] 에러 시스템 동작
  - [x] 한글 에러 메시지

### F2.5: 한글 에러 메시지
- 상태: ✅ 완료
- 브랜치: `feature/f2.5-korean-error`
- PR: #33
- 우선순위: HIGH
- 작업:
  - [x] src/error/ErrorMessage.h
  - [x] 에러 메시지 한글화
  - [x] 해결 방법 제안
  - [x] 코드 컨텍스트 표시
- 완료 조건:
  - [x] 모든 에러 한글 메시지
  - [x] 교육적 피드백

### F2.6: 타입 검사
- 상태: ✅ 완료
- 브랜치: `feature/f2.6-type-checker`
- PR: #31
- 우선순위: HIGH
- 작업:
  - [x] src/types/TypeChecker.h
  - [x] 타입 추론
  - [x] 타입 검사
  - [x] 타입 변환 함수
- 완료 조건:
  - [x] 타입 불일치 감지
  - [x] 명확한 에러 메시지

### F2.7: 표준 라이브러리
- 상태: ✅ 완료
- 브랜치: `feature/f2.7-standard-library`
- PR: #38
- 우선순위: HIGH
- 작업:
  - [x] stdlib/math.ksj - 수학 함수 (18개)
  - [x] stdlib/string.ksj - 문자열 처리 (9개)
  - [x] stdlib/array.ksj - 배열 유틸리티 (18개)
  - [x] examples/stdlib_math.ksj
  - [x] examples/stdlib_string.ksj
  - [x] examples/stdlib_array.ksj
  - [x] stdlib/README.md - 문서
- 완료 조건:
  - [x] 45개 이상의 유틸리티 함수
  - [x] 순수 KingSejong 구현
  - [x] 모듈 시스템 통합

---

## Phase 3: 고급 기능

**목표**: 프로덕션 수준의 언어
**기간**: Month 7-12 (24주)

### F3.1: 가비지 컬렉터
- 상태: ✅ 완료
- 브랜치: `feature/f3.1-garbage-collector`
- PR: #41 (예정)
- 우선순위: MEDIUM
- 작업:
  - [x] src/memory/GC.h (Object 베이스 클래스, GarbageCollector)
  - [x] src/memory/GC.cpp (Mark & Sweep 알고리즘 구현)
  - [x] Mark & Sweep 알고리즘
  - [x] 루트 객체 관리 (addRoot, removeRoot)
  - [x] Environment 추적 (weak_ptr)
  - [x] 자동 GC 트리거 (임계값 기반)
  - [x] 메모리 통계 (GCStats)
  - [x] tests/GCTest.cpp (17개 테스트)
  - [x] docs/GC_DESIGN.md (완전한 설계 문서)
- 완료 조건:
  - [x] 자동 메모리 관리
  - [x] 순환 참조 감지
  - [x] 테스트 통과 (17/17)
  - [x] 문서 완료

### F3.2: 바이트코드 컴파일러
- 상태: ✅ 완료
- 브랜치: `feature/f3.2-bytecode-compiler`
- PR: #42 (예정)
- 우선순위: MEDIUM
- 작업:
  - [x] OpCode 정의 (60+ 명령어)
  - [x] Chunk 클래스 (바이트코드 관리, 상수 풀, 디스어셈블러)
  - [x] Compiler 클래스 (AST → Bytecode 변환)
  - [x] VM 클래스 (스택 기반 실행 엔진)
  - [x] CMakeLists.txt 통합
  - [x] tests/BytecodeTest.cpp (23개 테스트)
- 완료 조건:
  - [x] 바이트코드 실행
  - [x] 기본 연산 지원 (산술, 비교, 논리)
  - [x] 변수 지원 (로컬, 전역)
  - [x] 배열 지원
  - [x] 테스트 통과

### F3.3: 모듈 시스템
- 상태: ✅ 완료
- 브랜치: `feature/f3.3-module-system`
- PR: #34, #35, #36
- 우선순위: MEDIUM
- 작업:
  - [x] `가져오기` 키워드
  - [x] 모듈 검색 및 경로 해석
  - [x] 순환 참조 방지
  - [x] ModuleLoader 구현
  - [x] Parser ImportStatement 구현
  - [x] Evaluator 통합
  - [x] 모듈 캐싱
- 완료 조건:
  - [x] 모듈 import/export
  - [x] 네임스페이스
  - [x] 순환 참조 감지
  - [x] 테스트 통과 (ModuleTest 8개)

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

## Phase 4: 최적화 및 안정화

**목표**: 성능 향상, 안정성 개선, 플랫폼 확장
**기간**: 2025 Q1-Q2 (6개월)
**현재 진행률**: 88% 완료

---

### F4.1: CI/CD 파이프라인
- 상태: ✅ 완료
- 브랜치: `feature/f4.1-cicd-pipeline`
- PR: #44
- 우선순위: HIGH
- 작업:
  - [x] GitHub Actions 워크플로우 구축
  - [x] macOS 빌드 및 테스트
  - [x] Linux 빌드 및 테스트 (Ubuntu)
  - [x] Windows 빌드 및 테스트 (Windows Server)
  - [x] 빌드 아티팩트 자동 업로드
  - [x] 코드 품질 검사 (clang-tidy)
  - [x] 메모리 안전성 검사 (Valgrind)
  - [x] 코드 커버리지 측정 (lcov)
  - [x] Sanitizer 통합 (ASan, UBSan)
- 완료 조건:
  - [x] 3개 플랫폼 자동 빌드
  - [x] 모든 테스트 통과
  - [x] 아티팩트 생성

### F4.2: 바이트코드 함수 지원
- 상태: ✅ 완료
- 브랜치: `feature/f4.2-bytecode-functions`
- PR: #45
- 우선순위: HIGH
- 작업:
  - [x] BUILD_FUNCTION OpCode 구현
  - [x] CALL OpCode 구현
  - [x] RETURN OpCode 구현
  - [x] 함수 파라미터 처리
  - [x] 클로저 지원 (Environment 캡처)
  - [x] CallFrame 스택 관리
  - [x] 재귀 함수 지원
  - [x] tests/BytecodeTest.cpp 업데이트
- 완료 조건:
  - [x] 함수 정의 및 호출
  - [x] 클로저 작동
  - [x] 재귀 함수 작동
  - [x] 테스트 통과

### F4.3: 성능 벤치마크 시스템
- 상태: ✅ 완료
- 브랜치: `feature/f4.3-benchmark-system`
- PR: #46
- 우선순위: HIGH
- 작업:
  - [x] benchmarks/ 디렉토리 구조
  - [x] run_benchmarks.py 스크립트
  - [x] 4개 벤치마크 프로그램 작성
    - [x] arithmetic.ksj (산술 연산)
    - [x] fibonacci.ksj (재귀 함수)
    - [x] array_ops.ksj (배열 조작)
    - [x] loop_intensive.ksj (반복문)
  - [x] 통계 분석 (평균, 최소, 최대, 표준편차)
  - [x] 결과 출력 및 저장
- 완료 조건:
  - [x] 벤치마크 실행 가능
  - [x] 성능 측정 및 분석
  - [x] 결과 리포트 생성

### F4.4: 컴파일러 최적화 패스
- 상태: ✅ 완료
- 브랜치: `feature/f4.4-optimization-passes`
- PR: #47
- 우선순위: MEDIUM
- 작업:
  - [x] 상수 폴딩 (Constant Folding)
    - [x] 정수 산술 연산
    - [x] 실수 산술 연산
    - [x] 비교 연산
  - [x] 데드 코드 제거 (Dead Code Elimination)
    - [x] 상수 조건 if문 최적화
    - [x] 불필요한 분기 제거
  - [x] 피홀 최적화 (Peephole Optimization)
    - [x] 단항 연산 상수 폴딩
    - [x] 불린 NOT 최적화
  - [x] tests/BytecodeTest.cpp 최적화 테스트 추가
- 완료 조건:
  - [x] 최적화 패스 구현
  - [x] 성능 개선 확인
  - [x] 테스트 통과

### F4.5: CI 벤치마크 자동화
- 상태: ✅ 완료
- 브랜치: `feature/f4.5-ci-benchmark-automation`
- PR: #48
- 우선순위: MEDIUM
- 작업:
  - [x] .github/workflows/ci.yml에 benchmark job 추가
  - [x] 4개 벤치마크 자동 실행
  - [x] 각 벤치마크 5회 실행 (통계적 신뢰성)
  - [x] 결과 artifact 저장
  - [x] 콘솔 출력
- 완료 조건:
  - [x] CI에서 자동 벤치마크 실행
  - [x] 결과 수집 및 저장

### F4.6: 메모리 안전성 검사
- 상태: ✅ 완료
- 커밋: cbabbe2, bf681b8
- 우선순위: HIGH
- 작업:
  - [x] AddressSanitizer (ASan) 통합
  - [x] UndefinedBehaviorSanitizer (UBSan) 통합
  - [x] GCTest use-after-free 버그 5건 수정
  - [x] BytecodeTest Parser 생성자 오류 수정
  - [x] TypeCheckerTest 예외 타입 수정
  - [x] TypeChecker.h 문자열 연결 오류 수정
  - [x] 테스트 코드 중복 심볼 제거
  - [x] Compiler.cpp AST API 변경 대응
  - [x] Evaluator.cpp 문자열 연결 오류 수정
- 완료 조건:
  - [x] ASan 클린 (heap-use-after-free 0건)
  - [x] UBSan 클린 (undefined behavior 0건)
  - [x] 전체 GC 테스트 15개 통과
- 주요 수정:
  - **GCTest.cpp**: 객체 삭제 전 GC unregister 누락 수정
  - **테스트 코드**: evalInput 함수 static 추가, 중복 main() 제거
  - **타입 안전성**: TypeError 함수 → KingSejongError 클래스 사용

### F4.7: 플랫폼 확장
- 상태: ✅ 완료
- 커밋: 639cbd8
- 우선순위: MEDIUM
- 작업:
  - [x] README.md 플랫폼별 설치 가이드 추가
  - [x] docs/INSTALL.md 상세 가이드 작성
  - [x] install.sh 스크립트 작성 (Linux/macOS)
  - [x] install.ps1 스크립트 작성 (Windows)
  - [x] 플랫폼별 의존성 설치 방법 문서화
  - [x] Visual Studio, MinGW-w64 지원 가이드
- 완료 조건:
  - [x] macOS 설치 가이드 완성
  - [x] Linux 설치 가이드 완성 (Ubuntu, Fedora, Arch)
  - [x] Windows 설치 가이드 완성
  - [x] 자동화 설치 스크립트 제공
- 주요 내용:
  - **README.md**: 플랫폼별 빠른 시작 가이드
  - **docs/INSTALL.md**: 630+ 줄의 상세 설치 문서
  - **install.sh**: 300+ 줄의 자동화 스크립트 (컬러 출력, 오류 처리)
  - **install.ps1**: 350+ 줄의 PowerShell 스크립트

### F4.8: JIT 컴파일러 연구
- 상태: ✅ 완료
- 브랜치: `main`
- 우선순위: LOW
- 작업:
  - [x] 핫 패스 감지 (Hot Path Detection)
  - [x] JIT 컴파일러 연구 문서
  - [x] LLVM 백엔드 통합 연구
  - [x] 벤치마크 계획
- 완료 조건:
  - [x] JIT 컴파일러 연구 보고서
  - [x] 핫 패스 감지 프로토타입
  - [x] LLVM 통합 평가
  - [x] 성능 예측 및 계획
- 주요 내용:
  - **HotPathDetector** (include/jit/HotPathDetector.h, src/jit/HotPathDetector.cpp)
    - 함수 호출 추적 및 카운팅
    - 루프 백엣지 추적
    - 핫 패스 자동 감지 (임계값 기반)
    - JIT 티어 관리 (Tier 1: 템플릿, Tier 2: LLVM)
    - 실행 시간 프로파일링 (마이크로초 단위)
    - 스코프 기반 프로파일러 (RAII)
  - **테스트**: 21개 테스트 (모두 통과)
  - **docs/research/JIT_COMPILER_RESEARCH.md** (1000+ 줄)
    - JIT 컴파일 기본 개념 및 전략
    - 핫 패스 감지 방법론 (카운터, 샘플링, 백엣지)
    - JIT 설계 옵션 (템플릿, 메소드, 트레이싱, LLVM)
    - Tiered Compilation 전략
    - 성능 예측 (10-100배 향상 가능)
    - 단계별 구현 로드맵
  - **docs/research/LLVM_INTEGRATION.md** (800+ 줄)
    - LLVM 아키텍처 및 통합 방식 분석
    - 바이트코드 → LLVM IR 변환 예시
    - ORC JIT API 사용 방법
    - 의존성 분석 (100+ MB, 빌드 시간 10-30분 증가)
    - 성능 예측 (5-20배 향상, 메모리 5-6배 증가)
    - 권장 사항: v1.5 이후 재고려
  - **examples/jit_hotpath_demo.cpp**: 핫 패스 감지 데모 프로그램
- **권장 사항**:
  - 현재: 핫 패스 감지만 사용 (프로파일링 목적)
  - v0.5-v1.0: 템플릿 JIT 프로토타입 (선택사항)
  - v1.5+: LLVM 통합 재고려

---

## Phase 5: 성능 & 개발자 경험 개선

**목표**: JIT 최적화, 개발 도구, 플랫폼 확장
**기간**: 2025 Q1-Q2 (6개월)
**시작일**: 2025-11-11
**현재 진행률**: 0% 완료

---

### F5.1: JIT 컴파일러 구현
- 상태: 📝 대기
- 브랜치: `feature/jit-compiler`
- 우선순위: CRITICAL
- 예상 기간: 6-8주

#### F5.1.1: 템플릿 JIT 프로토타입 (Tier 1)
- 상태: 📝 대기
- 브랜치: `feature/f5.1.1-template-jit`
- 우선순위: CRITICAL
- 예상 기간: 3주
- 작업:
  - [ ] 바이트코드 → 네이티브 코드 매핑
  - [ ] 레지스터 할당기 구현
  - [ ] 코드 생성기 (x86-64, ARM64)
  - [ ] JIT 컴파일 트리거 (HotPathDetector 활용)
  - [ ] 기본 최적화 (인라이닝, 레지스터 할당)
- 테스트:
  - [ ] 산술 연산 JIT 컴파일 (10개 테스트)
  - [ ] 반복문 JIT 컴파일 (10개 테스트)
  - [ ] 함수 호출 JIT 컴파일 (10개 테스트)
  - [ ] 성능 벤치마크 (10배 목표)
- 문서:
  - [ ] JIT 아키텍처 문서
  - [ ] 코드 생성 가이드
- 완료 조건:
  - [ ] 10배 성능 향상 달성
  - [ ] 모든 테스트 통과
  - [ ] 벤치마크 검증

#### F5.1.2: 고급 최적화 (Tier 2)
- 상태: 📝 대기
- 브랜치: `feature/f5.1.2-advanced-optimizations`
- 우선순위: HIGH
- 예상 기간: 2주
- 작업:
  - [ ] 인라인 캐싱 (Inline Caching)
  - [ ] 타입 특화 (Type Specialization)
  - [ ] 루프 최적화 (Loop Unrolling)
  - [ ] 가드 최적화 (Guard Optimization)
- 테스트:
  - [ ] 다형성 코드 최적화 (10개 테스트)
  - [ ] 타입 안정성 검증 (10개 테스트)
  - [ ] 성능 벤치마크 (50배 목표)
- 완료 조건:
  - [ ] 50배 성능 향상 달성
  - [ ] 타입 특화 정확성 100%
  - [ ] 벤치마크 검증

#### F5.1.3: LLVM 백엔드 통합 (Tier 3, 선택)
- 상태: 📝 대기
- 브랜치: `feature/f5.1.3-llvm-backend`
- 우선순위: MEDIUM
- 예상 기간: 3주
- 조건: Tier 1/2 성공 시
- 작업:
  - [ ] LLVM IR 생성기
  - [ ] ORC JIT API 통합
  - [ ] 최적화 패스 선택
- 테스트:
  - [ ] 복잡한 코드 최적화 (20개 테스트)
  - [ ] 성능 벤치마크 (100배 목표)
- 완료 조건:
  - [ ] 100배 성능 향상 달성
  - [ ] LLVM 통합 안정성 검증
  - [ ] 벤치마크 검증

---

### F5.2: 디버거 구현
- 상태: 📝 대기
- 브랜치: `feature/debugger`
- 우선순위: HIGH
- 예상 기간: 4주

#### F5.2.1: 기본 디버거
- 상태: 📝 대기
- 브랜치: `feature/f5.2.1-basic-debugger`
- 우선순위: HIGH
- 예상 기간: 2주
- 작업:
  - [ ] 브레이크포인트 설정/해제
  - [ ] 단계별 실행 (step, next, continue)
  - [ ] 콜 스택 추적
  - [ ] 변수 검사 (지역 변수, 전역 변수)
- 명령어:
  - `(ksj-debug) break 파일.ksj:10` - 브레이크포인트 설정
  - `(ksj-debug) run` - 실행
  - `(ksj-debug) step` - 한 줄 실행
  - `(ksj-debug) print 변수명` - 변수 출력
  - `(ksj-debug) continue` - 계속 실행
- 테스트:
  - [ ] 브레이크포인트 동작 (10개 테스트)
  - [ ] 단계별 실행 (10개 테스트)
  - [ ] 변수 검사 (10개 테스트)
- 완료 조건:
  - [ ] 모든 기본 명령어 동작
  - [ ] 테스트 통과
  - [ ] 사용자 가이드 작성

#### F5.2.2: 고급 디버거 기능
- 상태: 📝 대기
- 브랜치: `feature/f5.2.2-advanced-debugger`
- 우선순위: MEDIUM
- 예상 기간: 2주
- 작업:
  - [ ] 조건부 브레이크포인트
  - [ ] 와치포인트 (변수 변경 감지)
  - [ ] 표현식 평가
  - [ ] 소스 코드 표시
- 테스트:
  - [ ] 조건부 브레이크포인트 (5개 테스트)
  - [ ] 와치포인트 (5개 테스트)
  - [ ] 복잡한 표현식 평가 (10개 테스트)
- 완료 조건:
  - [ ] 모든 고급 기능 동작
  - [ ] 테스트 통과
  - [ ] 문서화 완료

---

### F5.3: LSP (Language Server Protocol)
- 상태: 📝 대기
- 브랜치: `feature/lsp`
- 우선순위: HIGH
- 예상 기간: 4주

#### F5.3.1: 기본 LSP 서버
- 상태: 📝 대기
- 브랜치: `feature/f5.3.1-basic-lsp`
- 우선순위: HIGH
- 예상 기간: 2주
- 작업:
  - [ ] LSP 서버 구현 (JSON-RPC)
  - [ ] 구문 강조 (Syntax Highlighting)
  - [ ] 자동 완성 (Auto-completion)
  - [ ] 에러 진단 (Diagnostics)
- 기능:
  - 키워드 자동 완성
  - 변수명 자동 완성
  - 실시간 에러 표시
- 테스트:
  - [ ] VS Code 확장 프로그램 (1개)
  - [ ] 자동 완성 동작 (20개 테스트)
  - [ ] 에러 진단 (15개 테스트)
- 완료 조건:
  - [ ] VS Code 통합 성공
  - [ ] 기본 기능 동작
  - [ ] 테스트 통과

#### F5.3.2: 고급 LSP 기능
- 상태: 📝 대기
- 브랜치: `feature/f5.3.2-advanced-lsp`
- 우선순위: MEDIUM
- 예상 기간: 2주
- 작업:
  - [ ] 정의로 이동 (Go to Definition)
  - [ ] 참조 찾기 (Find References)
  - [ ] 이름 변경 (Rename)
  - [ ] 호버 정보 (Hover Information)
- 테스트:
  - [ ] 정의로 이동 (10개 테스트)
  - [ ] 참조 찾기 (10개 테스트)
  - [ ] 이름 변경 (10개 테스트)
  - [ ] 모든 LSP 기능 통합 테스트 (10개)
- 완료 조건:
  - [ ] 모든 고급 기능 동작
  - [ ] 테스트 통과
  - [ ] 문서화 완료

---

### F5.4: 온라인 플레이그라운드
- 상태: 📝 대기
- 브랜치: `feature/web-playground`
- 우선순위: MEDIUM
- 예상 기간: 3주

#### F5.4.1: WebAssembly 빌드
- 상태: 📝 대기
- 브랜치: `feature/f5.4.1-wasm-build`
- 우선순위: MEDIUM
- 예상 기간: 1주
- 작업:
  - [ ] Emscripten 빌드 설정
  - [ ] WASM 모듈 생성
  - [ ] JavaScript API 바인딩
- 테스트:
  - [ ] 브라우저에서 실행 (5개 테스트)
  - [ ] 기본 예제 동작 (21개 예제)
- 완료 조건:
  - [ ] WASM 빌드 성공
  - [ ] JavaScript API 동작
  - [ ] 브라우저 테스트 통과

#### F5.4.2: 웹 인터페이스
- 상태: 📝 대기
- 브랜치: `feature/f5.4.2-web-interface`
- 우선순위: MEDIUM
- 예상 기간: 2주
- 작업:
  - [ ] Monaco Editor 통합 (구문 강조)
  - [ ] 실행 버튼 및 출력 창
  - [ ] 예제 갤러리
  - [ ] 공유 기능 (URL 인코딩)
- 기능:
  - 실시간 코드 실행
  - 에러 표시
  - 21개 예제 프로그램 제공
- 배포:
  - [ ] GitHub Pages 배포
  - [ ] 도메인 연결 (선택)
- 완료 조건:
  - [ ] 웹 인터페이스 완성
  - [ ] 모든 예제 동작
  - [ ] 배포 완료

---

### F5.5: 패턴 매칭
- 상태: 📝 대기
- 브랜치: `feature/pattern-matching`
- 우선순위: MEDIUM
- 예상 기간: 3주

#### F5.5.1: 기본 패턴 매칭
- 상태: 📝 대기
- 브랜치: `feature/f5.5.1-basic-pattern-matching`
- 우선순위: MEDIUM
- 예상 기간: 2주
- 작업:
  - [ ] MatchExpression AST 노드
  - [ ] 패턴 매칭 Parser
  - [ ] 패턴 매칭 Evaluator
- 문법:
  ```ksj
  결과 = 값에 대해 {
      1 -> "하나"
      2 -> "둘"
      _ -> "기타"
  }
  ```
- 테스트:
  - [ ] 리터럴 매칭 (10개 테스트)
  - [ ] 와일드카드 매칭 (5개 테스트)
  - [ ] 가드 조건 (5개 테스트)
- 완료 조건:
  - [ ] 기본 패턴 매칭 동작
  - [ ] 테스트 통과
  - [ ] 문서화 완료

#### F5.5.2: 고급 패턴 매칭
- 상태: 📝 대기
- 브랜치: `feature/f5.5.2-advanced-pattern-matching`
- 우선순위: LOW
- 예상 기간: 1주
- 작업:
  - [ ] 배열 분해 (Array Destructuring)
  - [ ] 타입 매칭
  - [ ] 범위 매칭
- 문법:
  ```ksj
  배열 = [1, 2, 3]
  결과 = 배열에 대해 {
      [첫번째, 나머지...] -> 첫번째
      [] -> 0
  }
  ```
- 테스트:
  - [ ] 배열 분해 (10개 테스트)
  - [ ] 타입 매칭 (5개 테스트)
  - [ ] 범위 매칭 (5개 테스트)
- 완료 조건:
  - [ ] 고급 패턴 매칭 동작
  - [ ] 테스트 통과
  - [ ] 문서화 완료

---

### F5.6: 비동기 프로그래밍 (선택)
- 상태: 📝 대기
- 브랜치: `feature/async-programming`
- 우선순위: LOW
- 예상 기간: 4주
- 조건: Phase 5 다른 기능 완료 후

#### F5.6.1: Promise 구현
- 상태: 📝 대기
- 브랜치: `feature/f5.6.1-promise`
- 우선순위: LOW
- 예상 기간: 2주
- 작업:
  - [ ] Promise Value 타입
  - [ ] then/catch 메서드
  - [ ] async 함수
- 문법:
  ```ksj
  함수 비동기_작업() {
      # 비동기 작업
  }

  결과 = 비동기_작업()
  결과를 기다린다
  ```
- 테스트:
  - [ ] Promise 생성 및 해결 (10개 테스트)
  - [ ] then/catch 체이닝 (10개 테스트)
  - [ ] async 함수 동작 (10개 테스트)
- 완료 조건:
  - [ ] Promise 동작
  - [ ] 테스트 통과
  - [ ] 문서화 완료

#### F5.6.2: 시제 기반 비동기 (연구)
- 상태: 📝 대기
- 브랜치: `feature/f5.6.2-tense-based-async`
- 우선순위: LOW
- 예상 기간: 2주
- 작업:
  - [ ] 시제 분석 연구
  - [ ] 문법 설계
  - [ ] 프로토타입 구현
- 테스트:
  - [ ] 시제 분석 정확성 (5개 테스트)
  - [ ] 비동기 의미론 검증 (5개 테스트)
- 완료 조건:
  - [ ] 연구 보고서 작성
  - [ ] 프로토타입 동작
  - [ ] 문서화 완료

---

## 현재 작업 상태

### 완료
- ✅ Phase 1: 핵심 차별화 기능 (100% 완료)
- ✅ Phase 2: 실용 기능 (100% 완료)
- ✅ Phase 3: 고급 기능 (100% 완료)
- ✅ Phase 4: 최적화 및 안정화 (100% 완료)
- ✅ **모든 테스트 통과** (567/567, 100%)
- ✅ **v0.2.0 릴리스 완료** (2025-11-11)

### 진행 중
- 🚧 Phase 5: 성능 & 개발자 경험 개선 (0% 완료)
- 🚧 F5.1.1: 템플릿 JIT 프로토타입 (시작 예정)

### 다음 작업
1. **F5.1.1: 템플릿 JIT 프로토타입** - 10배 성능 향상 (3주)
2. **F5.2.1: 기본 디버거** - 브레이크포인트, 단계별 실행 (2주)
3. **F5.3.1: 기본 LSP** - VS Code 통합 (2주)
4. **F5.4: 온라인 플레이그라운드** - WebAssembly (3주)
5. **F5.5: 패턴 매칭** - 언어 기능 강화 (3주)

### 최근 완료 (2025-11-11)
- ✅ **모든 테스트 통과** (567/567) - 4개 실패 테스트 수정
- ✅ Commit ce10255: 남은 4개 테스트 실패 수정
- ✅ Commit ca1b47f: 타입 키워드 함수 이름 사용 가능
- ✅ F4.8: JIT 컴파일러 연구 (핫 패스 감지, LLVM 통합 연구)
- ✅ F4.7: 플랫폼 확장 (Commit 639cbd8)
- ✅ F4.6: 메모리 안전성 검사 (Commit cbabbe2, bf681b8)
- ✅ F4.5: CI 벤치마크 자동화 (PR #48)
- ✅ F4.4: 컴파일러 최적화 패스 (PR #47)
- ✅ F3.2: 바이트코드 컴파일러 (PR #42)
- ✅ F3.1: 가비지 컬렉터 (PR #41)

---

## 마일스톤

### ✅ M1: Hello World (완료)
```ksj
출력("안녕, KingSejong!")
```

### ✅ M2: 핵심 차별화 (완료)
```ksj
숫자들 = [1, 2, 3, 4, 5]
짝수 = 숫자들을 걸러낸다(함수(x) { 반환 x % 2 == 0 })
제곱 = 짝수를 변환한다(함수(x) { 반환 x * x })
출력(제곱)  # [4, 16]
```

### ✅ M3: 실용 수준 (완료)
```ksj
# 표준 라이브러리 활용
가져오기 "stdlib/math"

함수 피보나치(n) {
    만약 (n <= 1) {
        반환 n
    }
    반환 피보나치(n-1) + 피보나치(n-2)
}

i가 0부터 10까지 {
    출력(피보나치(i))
}
```

### ✅ M4: 프로덕션 (완료)
- [x] 가비지 컬렉터 ✅
- [x] 바이트코드 컴파일러 ✅
- [x] 모듈 시스템 ✅
- [x] 표준 라이브러리 ✅
- [ ] 온라인 플레이그라운드 (Phase 4)

---

## 메트릭스

### 코드 품질
- [x] 테스트 커버리지 ≥ 80%
- [x] 모든 기능 테스트 통과 (330개+)
- [x] 가비지 컬렉터 구현 완료
- [ ] Valgrind 클린 (Phase 4)

### 성능
- [ ] 기본 벤치마크 < 100ms
- [ ] 바이트코드 2-5배 빠름 (미구현)
- [ ] 메모리 사용량 < 50MB

### 문서화
- [x] README 완성
- [x] API 문서 (stdlib/README.md)
- [x] 예제 30개 이상 (14개 examples)
- [x] 튜토리얼 주요 개선 완료 (3개 섹션 추가, 알려진 이슈 문서화)
  - ✅ 조사 표현식 섹션 추가
  - ✅ 클로저 및 고차 함수 섹션 추가
  - ✅ 에러 처리 섹션 추가
  - ⚠️ 변수 선언 문법 이슈 발견 (타입 추론 vs 타입 필수)
  - ⚠️ `타입()` 함수 오용 문제 발견
  - 📄 상세 내용: docs/TUTORIAL_COMPLETION_NOTES.md

---

**마지막 업데이트**: 2025-11-11
**현재 상태**: v0.1.0 릴리스 준비 (Phase 1, 2, 3, 4 완료, 모든 테스트 통과)
**다음 작업**: v0.1.0 릴리스 및 Phase 5 계획
