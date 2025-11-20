# KingSejong 언어 구현 상태 및 계획

> KingSejong Programming Language Implementation Status & Roadmap

**프로젝트**: KingSejong (`.ksj`)
**현재 버전**: v0.5.0 (Phase 8 완료)
**저장소**: https://github.com/0xmhha/kingsejonglang
**테스트**: 1,503개 (100% 통과)
**마지막 업데이트**: 2025-11-21

---

## 목차

1. [✅ 완료된 Phase (0-8)](#-완료된-phase-0-8)
2. [📝 계획 중인 Phase (9 이후)](#-계획-중인-phase-9-이후)
3. [구현 상세 내역](#구현-상세-내역)

---

## 개발 원칙 (✅ 적용 완료)

### Clean Room 설계 ✅
- ✅ 독립적으로 재설계된 코드베이스
- ✅ 알고리즘과 아이디어 독립 구현
- ✅ MIT License 적용
- ✅ 라이센스 문제 완전 해결

### 핵심 우선순위 (✅ 달성)
1. ✅ **차별화 먼저**: 조사 체인, 범위 표현 등 혁신적 기능 구현 완료
2. ✅ **교육 중심**: 명확한 에러 메시지, 한글 문서화
3. ✅ **실용성 확보**: 실제 사용 가능한 수준 (LSP, 디버거, VS Code)
4. ✅ **고성능**: Bytecode VM + JIT Compiler Tier 1

---

## ✅ 완료된 Phase (0-8)

### Phase 0: 프로젝트 초기화 ✅

**목표**: 새 프로젝트 구조 및 개발 환경 구축
**상태**: ✅ 완료

---

#### 0.1 GitHub 저장소 생성 ✅

**작업 내용**:
- [x] 저장소 생성: `kingsejonglang`
- [x] MIT License 적용
- [x] README.md 작성
- [x] .gitignore 설정
- [x] 브랜치 전략 수립 (main 브랜치)

**결과**:
```
kingsejonglang/
├── LICENSE (MIT) ✅
├── README.md ✅
├── .gitignore ✅
└── docs/
    ├── LANGUAGE_DESIGN_PHILOSOPHY.md ✅
    ├── FEATURES.md ✅
    ├── IMPLEMENTATION.md ✅
    ├── PROJECT_OVERVIEW.md ✅
    ├── STARTER_GUIDE.md ✅
    └── CONTRIBUTING.md ✅
```

#### 0.2-0.5 프로젝트 기반 구축 ✅

**완료된 작업**:
- [x] CMake 빌드 시스템 (C++23)
- [x] GoogleTest 통합 (1,503개 테스트)
- [x] CI/CD 설정 (GitHub Actions)
- [x] 크로스 플랫폼 지원 (macOS, Linux, Windows)

**현재 디렉토리 구조**:
```
sejong/
├── src/
│   ├── lexer/          # 어휘 분석 (조사 인식)
│   ├── parser/         # 구문 분석
│   ├── ast/            # AST 노드
│   ├── evaluator/      # 실행 엔진
│   ├── types/          # 타입 시스템
│   ├── stdlib/         # 표준 라이브러리 (C++)
│   ├── repl/           # REPL
│   └── main.cpp        # 진입점
├── stdlib/             # 표준 라이브러리 (.sj 파일)
│   ├── 입출력.sj
│   ├── 수학.sj
│   └── 문자열.sj
├── tests/              # 테스트
│   ├── lexer/
│   ├── parser/
│   └── evaluator/
├── examples/           # 예제 코드
│   ├── hello.sj
│   ├── calculator.sj
│   └── ...
├── CMakeLists.txt      # 빌드 설정
└── docs/               # 문서
```

---

### 0.3 빌드 시스템 (1일)

**CMakeLists.txt**:
```cmake
cmake_minimum_required(VERSION 3.21)
project(Sejong VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 소스 파일
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 실행 파일
add_executable(sejong ${SOURCES})

# 테스트 (GoogleTest)
enable_testing()
add_subdirectory(tests)

# 설치
install(TARGETS sejong DESTINATION bin)
install(DIRECTORY stdlib DESTINATION share/sejong)
```

---

### 0.4 테스트 프레임워크 (2일)

**GoogleTest 통합**:
```bash
# tests/CMakeLists.txt
find_package(GTest REQUIRED)

add_executable(sejong_tests
    lexer_test.cpp
    parser_test.cpp
    evaluator_test.cpp
)

target_link_libraries(sejong_tests GTest::GTest GTest::Main)
```

**첫 테스트**:
```cpp
// tests/lexer_test.cpp
TEST(LexerTest, BasicTokenization) {
    Lexer lexer("이름 = \"철수\"");
    Token token1 = lexer.nextToken();
    EXPECT_EQ(token1.type, TokenType::IDENTIFIER);
    EXPECT_EQ(token1.literal, "이름");
}
```

---

### 0.5 CI/CD 설정 (1일)

**GitHub Actions**:
```yaml
# .github/workflows/ci.yml
name: CI

on: [push, pull_request]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]

    steps:
    - uses: actions/checkout@v2
    - name: Configure CMake
      run: cmake -B build
    - name: Build
      run: cmake --build build
    - name: Test
      run: ctest --test-dir build --output-on-failure
```

---

## Phase 1: 핵심 차별화 기능 (3개월)

### 목표
Sejong만의 혁신적 기능 구현으로 차별화

---

### Week 1-2: 조사 인식 시스템

#### 1.1 Lexer - 조사 토큰화 (5일)

**작업**:
- ☐ 조사 사전 구축
- ☐ 조사 인식 알고리즘
- ☐ 받침 유무에 따른 조사 처리 (`을/를`, `이/가`)

**구현**:
```cpp
// src/lexer/Josa.h
enum class JosaType {
    EUL_REUL,   // 을/를
    I_GA,       // 이/가
    EUN_NEUN,   // 은/는
    UI,         // 의
    RO,         // 로/으로
    ESO,        // 에서
    E           // 에
};

class JosaRecognizer {
public:
    static bool isJosa(const std::string& str);
    static JosaType getJosaType(const std::string& str);
    static std::string selectJosa(const std::string& noun, JosaType type);
};

// 예: selectJosa("철수", JosaType::EUL_REUL) → "를"
//     selectJosa("책", JosaType::EUL_REUL) → "을"
```

**테스트**:
```cpp
TEST(JosaTest, Recognition) {
    EXPECT_TRUE(JosaRecognizer::isJosa("을"));
    EXPECT_TRUE(JosaRecognizer::isJosa("를"));
    EXPECT_EQ(JosaRecognizer::getJosaType("을"), JosaType::EUL_REUL);
}

TEST(JosaTest, Selection) {
    EXPECT_EQ(JosaRecognizer::selectJosa("철수", JosaType::EUL_REUL), "를");
    EXPECT_EQ(JosaRecognizer::selectJosa("책", JosaType::EUL_REUL), "을");
}
```

---

#### 1.2 Parser - 조사 체인 파싱 (5일)

**작업**:
- ☐ 조사 표현식 AST 노드
- ☐ 조사 체인 파싱 로직
- ☐ 우선순위 처리

**AST 노드**:
```cpp
// src/ast/JosaExpression.h
class JosaExpression : public Expression {
public:
    std::unique_ptr<Expression> object;
    JosaType josaType;
    std::unique_ptr<Expression> method;

    // 예: 배열을 정렬한다
    //     object = 배열
    //     josaType = EUL_REUL
    //     method = 정렬한다
};
```

**Parser**:
```cpp
// src/parser/Parser.cpp
std::unique_ptr<Expression> Parser::parseJosaChain() {
    auto expr = parsePrimary();

    while (currentTokenIsJosa()) {
        Token josa = consume();
        auto method = parseMethodCall();
        expr = std::make_unique<JosaExpression>(
            std::move(expr),
            josa.josaType,
            std::move(method)
        );
    }

    return expr;
}
```

---

#### 1.3 Evaluator - 조사 의미 해석 (5일)

**작업**:
- ☐ 조사별 동작 정의
- ☐ 메서드 호출 처리
- ☐ 체인 평가

**구현**:
```cpp
// src/evaluator/Evaluator.cpp
Value Evaluator::evalJosaExpression(JosaExpression* expr) {
    Value object = eval(expr->object.get());

    switch (expr->josaType) {
        case JosaType::EUL_REUL:
            // 목적어 - 메서드 호출
            return callMethod(object, expr->method.get());

        case JosaType::UI:
            // 소유격 - 속성 접근
            return getProperty(object, expr->method.get());

        case JosaType::RO:
            // 수단/방법 - 변환
            return transform(object, expr->method.get());

        // ...
    }
}
```

---

### Week 3-4: 자연스러운 범위 표현

#### 1.4 범위 문법 설계 (3일)

**작업**:
- ☐ 범위 키워드 정의 (`부터`, `까지`, `미만`, `초과`, `이하`)
- ☐ AST 노드 설계

**AST**:
```cpp
// src/ast/RangeExpression.h
enum class RangeBoundType {
    INCLUSIVE,   // 포함 (까지, 이하)
    EXCLUSIVE    // 미포함 (미만, 초과)
};

class RangeExpression : public Expression {
public:
    std::unique_ptr<Expression> start;
    std::unique_ptr<Expression> end;
    RangeBoundType startType;  // 시작 포함/미포함
    RangeBoundType endType;    // 끝 포함/미포함
};
```

---

#### 1.5 범위 파싱 (4일)

**작업**:
- ☐ `부터`, `까지`, `미만` 등 키워드 파싱
- ☐ 범위 표현식 생성

**Parser**:
```cpp
std::unique_ptr<RangeExpression> Parser::parseRange() {
    auto start = parseExpression();

    // "부터"
    if (!match(TokenType::BUTEO)) {
        error("범위는 '부터'로 시작해야 합니다");
    }

    auto startType = RangeBoundType::INCLUSIVE;
    if (match(TokenType::CHOGA)) {  // "초과"
        startType = RangeBoundType::EXCLUSIVE;
    }

    auto end = parseExpression();

    auto endType = RangeBoundType::EXCLUSIVE;
    if (match(TokenType::KKAJI)) {  // "까지"
        endType = RangeBoundType::INCLUSIVE;
    } else if (match(TokenType::MIMAN)) {  // "미만"
        endType = RangeBoundType::EXCLUSIVE;
    } else if (match(TokenType::IHA)) {  // "이하"
        endType = RangeBoundType::INCLUSIVE;
    }

    return std::make_unique<RangeExpression>(
        std::move(start),
        std::move(end),
        startType,
        endType
    );
}
```

---

#### 1.6 범위 평가 (3일)

**작업**:
- ☐ 범위 객체 생성
- ☐ 반복문 통합
- ☐ 배열 슬라이싱

**Evaluator**:
```cpp
Value Evaluator::evalRange(RangeExpression* expr) {
    int start = evalAsInteger(expr->start.get());
    int end = evalAsInteger(expr->end.get());

    // 범위 타입에 따라 조정
    if (expr->startType == RangeBoundType::EXCLUSIVE) {
        start++;
    }
    if (expr->endType == RangeBoundType::INCLUSIVE) {
        end++;
    }

    return Value::createRange(start, end);
}
```

---

### Week 5-6: 명확한 타입 시스템

#### 1.7 타입 정의 (4일)

**작업**:
- ☐ 기본 타입 (`정수`, `실수`, `문자열`, `참거짓`, `없음`)
- ☐ 타입 선언 문법
- ☐ 타입 검사

**타입 시스템**:
```cpp
// src/types/Type.h
enum class TypeKind {
    INTEGER,    // 정수
    FLOAT,      // 실수
    STRING,     // 문자열
    BOOLEAN,    // 참거짓
    NULL_TYPE,  // 없음
    ARRAY,      // 배열
    OBJECT      // 객체
};

class Type {
public:
    TypeKind kind;
    std::string name;  // "정수", "실수" 등

    static Type* getBuiltinType(const std::string& koreanName);
};
```

---

#### 1.8 타입 선언 파싱 (3일)

**Parser**:
```cpp
// "정수 나이 = 15" 파싱
std::unique_ptr<VarDeclaration> Parser::parseVarDeclaration() {
    Type* type = nullptr;

    if (isTypeKeyword(currentToken)) {
        type = Type::getBuiltinType(currentToken.literal);
        advance();
    }

    auto name = expectIdentifier();
    expect(TokenType::ASSIGN);
    auto value = parseExpression();

    return std::make_unique<VarDeclaration>(type, name, std::move(value));
}
```

---

#### 1.9 타입 검사 및 변환 (3일)

**Evaluator**:
```cpp
Value Evaluator::evalBinaryOp(BinaryExpression* expr) {
    Value left = eval(expr->left.get());
    Value right = eval(expr->right.get());

    // 타입 검사
    if (left.type != right.type) {
        if (expr->op == TokenType::PLUS &&
            (left.type == TypeKind::STRING ||
             right.type == TypeKind::STRING)) {
            // 문자열 연결은 허용
        } else {
            throw TypeError(
                "타입 오류: " + left.typeName() +
                "와 " + right.typeName() +
                "은 " + expr->op + " 연산을 할 수 없습니다"
            );
        }
    }

    // 연산 수행
    // ...
}
```

---

### Week 7-8: 직관적 반복문

#### 1.10 반복문 문법 (5일)

**작업**:
- ☐ `10번 반복한다` 파싱
- ☐ `1부터 10까지 반복한다` 파싱
- ☐ `각각에 대해` 파싱

**AST**:
```cpp
// "10번 반복한다"
class RepeatStatement : public Statement {
public:
    std::unique_ptr<Expression> count;
    std::unique_ptr<BlockStatement> body;
};

// "i가 1부터 10까지 반복한다"
class RangeForStatement : public Statement {
public:
    std::string variable;
    std::unique_ptr<RangeExpression> range;
    std::unique_ptr<BlockStatement> body;
};

// "배열 각각에 대해 항목으로"
class ForEachStatement : public Statement {
public:
    std::unique_ptr<Expression> collection;
    std::string itemName;
    std::unique_ptr<BlockStatement> body;
};
```

---

#### 1.11 반복문 평가 (5일)

**Evaluator**:
```cpp
Value Evaluator::evalRepeatStatement(RepeatStatement* stmt) {
    int count = evalAsInteger(stmt->count.get());

    for (int i = 0; i < count; i++) {
        eval(stmt->body.get());

        // break/continue 처리
        if (breakFlag) {
            breakFlag = false;
            break;
        }
        if (continueFlag) {
            continueFlag = false;
            continue;
        }
    }

    return Value::createNull();
}
```

---

### Week 9-12: 기본 기능 완성

#### 1.12 나머지 기본 기능 (4주)

**작업**:
- ☐ 함수 정의/호출
- ☐ 조건문 (if/else)
- ☐ 기본 연산자
- ☐ 변수 환경 (스코프)
- ☐ 표준 입출력 (`출력`, `입력`)

**테스트**:
```cpp
TEST(EvaluatorTest, BasicProgram) {
    std::string code = R"(
        정수 나이 = 15

        만약 (나이 >= 18) {
            출력("성인")
        } 아니면 {
            출력("미성년자")
        }

        함수 인사(이름) {
            반환 "안녕, " + 이름
        }

        메시지 = 인사("철수")
        출력(메시지)
    )";

    Evaluator eval;
    eval.run(code);
}
```

---

## Phase 2: 실용 기능 (3개월)

### Week 13-16: 배열 및 컬렉션

#### 2.1 배열 구현 (4주)

**기능**:
- ☐ 배열 리터럴
- ☐ 인덱싱/슬라이싱
- ☐ 배열 메서드 (조사 활용)
- ☐ 고차 함수 (필터, 맵, 리듀스)

**예제**:
```sejong
숫자들 = [1, 2, 3, 4, 5]

# 조사 활용
숫자들에서 짝수만_걸러낸다
숫자들을 각각_제곱한다
숫자들의 합계를_구한다
```

---

### Week 17-20: 시제 기반 비동기

#### 2.2 비동기 시스템 (4주)

**기능**:
- ☐ `~을것이다` (Promise 생성)
- ☐ `~때까지_기다린다` (await)
- ☐ 에러 처리

**구현**:
```cpp
// 시제 분석
enum class Tense {
    PRESENT,  // ~ㄴ다
    FUTURE    // ~ㄹ것이다
};

Value Evaluator::evalFunctionCall(CallExpression* expr) {
    if (expr->tense == Tense::FUTURE) {
        // Promise 생성
        return createPromise(expr);
    } else {
        // 동기 실행
        return callFunction(expr);
    }
}
```

---

### Week 21-24: 에러 처리 및 메시지

#### 2.3 에러 시스템 (4주)

**기능**:
- ☐ 구조화된 에러 클래스
- ☐ 스택 트레이스
- ☐ 한글 에러 메시지
- ☐ 해결 방법 제안

**에러 메시지 개선**:
```cpp
class RuntimeError {
public:
    std::string message;
    Location location;
    std::vector<std::string> suggestions;

    std::string format() {
        std::stringstream ss;
        ss << "오류: " << message << "\n\n";
        ss << "위치: " << location.filename
           << " (" << location.line << "번째 줄)\n";

        // 코드 표시
        ss << showCodeContext();

        // 제안
        if (!suggestions.empty()) {
            ss << "\n제안:\n";
            for (auto& s : suggestions) {
                ss << "  " << s << "\n";
            }
        }

        return ss.str();
    }
};
```

---

## Phase 3: 고급 기능 (6개월)

### Month 7-8: 모듈 시스템

#### 3.1 모듈 설계 (2개월)

**기능**:
- ☐ `가져오기` 키워드
- ☐ 모듈 검색 경로
- ☐ 순환 참조 방지

---

### Month 9-10: 바이트코드 컴파일러

#### 3.2 바이트코드 VM (2개월)

**기능**:
- ☐ OpCode 설계
- ☐ AST → Bytecode 컴파일러
- ☐ 스택 기반 VM
- ☐ 성능 벤치마크 (2-5배 향상 목표)

---

### Month 11-12: 생태계 구축

#### 3.3 개발 도구 (2개월)

**기능**:
- ☐ 디버거
- ☐ 온라인 플레이그라운드 (WebAssembly)
- ☐ VSCode 확장
- ☐ 문서 사이트

---

## 개발 가이드

### 코딩 스타일

```cpp
// 네이밍
class JosaExpression;    // PascalCase
void parseJosaChain();   // camelCase
josa_type_;              // snake_case (private)

// 스마트 포인터 사용
std::unique_ptr<Expression> expr;
std::shared_ptr<Environment> env;

// 테스트 필수
TEST(FeatureName, TestCase) {
    // Arrange
    // Act
    // Assert
}
```

### Git 워크플로

```bash
# 기능 브랜치
git checkout -b feature/josa-chain

# 커밋 메시지
feat: 조사 기반 메서드 체인 구현
fix: 범위 표현 버그 수정
docs: 사용자 가이드 업데이트
test: 타입 시스템 테스트 추가
```

---

## ✅ 달성된 마일스톤

### ✅ M1: 첫 실행 (완료)
```ksj
출력("안녕, KingSejong!")
```

### ✅ M2: 핵심 차별화 (완료)
```ksj
숫자들 = [1, 2, 3, 4, 5]
결과 = 숫자들에서 짝수만_걸러내고 각각을_제곱한다
출력(결과)
```

### ✅ M3: 실용 수준 (완료)
```ksj
비동기_함수 데이터_가져오기(url) {
    응답 = API호출(url)를_기다린다
    데이터 = 응답_파싱()을_기다린다
    반환 데이터
}
```

### 🚧 M4: 완성 (진행 중)
- 🚧 온라인 플레이그라운드 (WebAssembly)
- ✅ 21개 예제
- 📝 교육 과정 통합

---

## 📝 계획 중인 Phase (9 이후)

### Phase 9: 리팩토링 및 최적화

**목표**: 코드 품질 향상 및 성능 최적화

**작업 내용**:
- [ ] 긴 함수 리팩토링 (VM::execute, Evaluator 메서드들)
- [ ] Incremental GC 구현 (현재: Stop-the-World GC)
- [ ] 메모리 풀링 전략 (Allocator/Pool 재사용)
- [ ] stdlib/http, stdlib/db 완성 (현재 일부 구현)
- [ ] 성능 프로파일링 및 핫스팟 최적화

**예상 기간**: 2-3개월

---

## 🎯 v1.0.0 릴리스 목표

**Production Readiness**: 8.5/10 → 9.5/10

**필수 작업**:
- [ ] Phase 9 완료 (리팩토링 및 최적화)
- [ ] 최종 안정화 테스트
- [ ] 릴리스 노트 작성
- [ ] 프로덕션 배포 가이드

**선택 작업**:
- [ ] 온라인 플레이그라운드 (WebAssembly)
- [ ] 교육 기관 파일럿 테스트
- [ ] 커뮤니티 구축

---

## 📊 현재 상태 요약 (v0.5.0)

| 카테고리           | 진행률  | 상태 |
| ------------------ | ------- | ---- |
| **핵심 언어 기능** | 100%    | ✅   |
| **표준 라이브러리**| 95%     | ✅   |
| **개발 도구**      | 100%    | ✅   |
| **테스트**         | 100%    | ✅   |
| **문서화**         | 100%    | ✅   |
| **성능 최적화**    | 80%     | 🚧   |

**구현 완료된 주요 기능**:
- ✅ 조사 기반 메서드 체인
- ✅ 자연스러운 범위 표현
- ✅ 명확한 타입 시스템
- ✅ 클래스 및 상속
- ✅ Async/Await
- ✅ Try/Catch/Finally
- ✅ 패턴 매칭
- ✅ 모듈 시스템
- ✅ Bytecode VM (56개 OpCode)
- ✅ JIT Compiler Tier 1
- ✅ 세대별 GC
- ✅ LSP (Language Server Protocol)
- ✅ 디버거
- ✅ Linter & Formatter
- ✅ VS Code 확장
- ✅ ksjpm 패키지 관리자

**표준 라이브러리**:
- ✅ 14개 모듈
- ✅ 182개 함수
- ✅ array, collections, crypto, db, http, io, json, math, os, regex, string, test, time, utils

**테스트**:
- ✅ 1,503개 테스트
- ✅ 100% 통과
- ✅ regression_test.py 자동화

---

**프로젝트 상태**: Phase 8 완료, Phase 9 준비 중
**저장소**: https://github.com/0xmhha/kingsejonglang
**다음 작업**: Phase 9 시작 - 긴 함수 리팩토링 및 Incremental GC 구현
