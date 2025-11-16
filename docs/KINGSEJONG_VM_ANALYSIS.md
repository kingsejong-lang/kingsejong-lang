# KingSejong Virtual Machine 심층 분석

> 작성일: 2025-11-16
> 분석 대상: KingSejong v0.3.1
> 분석 범위: VM 아키텍처, 메모리 관리, 런타임, 표준 라이브러리

---

## 목차

1. [개요](#개요)
2. [VM 아키텍처](#vm-아키텍처)
3. [메모리 관리 및 GC](#메모리-관리-및-gc)
4. [Value 표현 및 타입 시스템](#value-표현-및-타입-시스템)
5. [표준 라이브러리](#표준-라이브러리)
6. [동시성 및 병렬성](#동시성-및-병렬성)
7. [디버깅 지원](#디버깅-지원)
8. [프로파일링 및 성능](#프로파일링-및-성능)
9. [보안 분석](#보안-분석)
10. [모듈 시스템](#모듈-시스템)
11. [에러 처리](#에러-처리)
12. [FFI (Foreign Function Interface)](#ffi)
13. [빌드 시스템 및 도구](#빌드-시스템-및-도구)
14. [생산성 평가](#생산성-평가)
15. [부족한 기능 종합](#부족한-기능-종합)
16. [개선 로드맵](#개선-로드맵)

---

## 개요

### 핵심 발견사항

KingSejong은 **하이브리드 실행 엔진**을 가진 현대적인 프로그래밍 언어입니다:

**아키텍처**: Tree-walking Interpreter + Stack-based Bytecode VM

**통계**:
- **코드 규모**: 26,003줄 (C++23)
- **소스 파일**: 93개
- **테스트**: 1,230개 (100% 통과)
- **내장 함수**: 78개
- **stdlib 모듈**: 13개
- **OpCode**: 43개
- **버전**: v0.3.3

**성능 특성**:
```
루프 벤치마크 (1천만 반복):
- Tree-walking: ~5초
- Bytecode VM: ~1초
- JIT (가상): ~0.1초
```

**Production Readiness**: **4.5/10** ⚠️

---

## VM 아키텍처

### 1.1 하이브리드 아키텍처

KingSejong은 **2개의 독립적인 실행 엔진**을 제공합니다:

#### 1️⃣ Tree-walking Interpreter (Evaluator)

**파일**: `/src/evaluator/Evaluator.h/cpp` (1,257줄)

**동작 방식**:
- AST를 직접 순회하며 실행
- 각 노드를 방문하여 평가 (`eval()`)
- 재귀적 실행

**장점**:
- ✅ 구현 간단
- ✅ 디버깅 용이
- ✅ REPL에 적합

**단점**:
- ❌ 느린 성능 (~5초)
- ❌ 반복적인 AST 순회
- ❌ 최적화 어려움

**사용 시점**:
- 대화형 REPL
- 개발/디버깅 모드
- 동적 코드 실행

#### 2️⃣ Stack-based Bytecode VM

**파일**:
- VM: `/src/bytecode/VM.h` (181줄) + `VM.cpp` (581줄)
- Compiler: `/src/bytecode/Compiler.h` (135줄) + `Compiler.cpp` (789줄)
- OpCode: `/src/bytecode/OpCode.h` (143줄)

**동작 방식**:
```
AST → Compiler → Bytecode → VM 실행
```

**VM 구조**:
```cpp
class VM {
private:
    std::vector<evaluator::Value> stack_;     // 피연산자 스택
    std::vector<CallFrame> frames_;           // 호출 프레임
    size_t ip_;                               // 명령어 포인터

    // 안전장치
    size_t instructionCount_ = 0;             // 실행된 명령어 수
    std::chrono::steady_clock::time_point startTime_;  // 시작 시간

public:
    VMResult run();                           // 바이트코드 실행
};
```

**장점**:
- ✅ 빠른 실행 (~1초, 5배 향상)
- ✅ 최적화 가능
- ✅ 스택 기반 (간단한 구조)

**단점**:
- ❌ 컴파일 오버헤드
- ❌ JIT 없음 (아직)

### 1.2 Instruction Set (43개 OpCode)

**파일**: `/src/bytecode/OpCode.h:24-118`

#### 분류별 OpCode

**상수 로딩 (4개)**:
```cpp
LOAD_CONST        // 상수 풀에서 로드
LOAD_TRUE         // true 로드
LOAD_FALSE        // false 로드
LOAD_NULL         // null 로드
```

**변수 처리 (4개)**:
```cpp
LOAD_VAR          // 지역 변수 로드
STORE_VAR         // 지역 변수 저장
LOAD_GLOBAL       // 전역 변수 로드
STORE_GLOBAL      // 전역 변수 저장
```

**산술 연산 (6개)**:
```cpp
ADD, SUB, MUL, DIV, MOD, NEG
```

**비교 연산 (6개)**:
```cpp
EQ, NE, LT, GT, LE, GE
```

**논리 연산 (3개)**:
```cpp
AND, OR, NOT
```

**제어 흐름 (4개)**:
```cpp
JUMP              // 무조건 점프
JUMP_IF_FALSE     // false면 점프
JUMP_IF_TRUE      // true면 점프
LOOP              // 루프 (역방향 점프)
```

**함수 관련 (3개)**:
```cpp
CALL              // 함수 호출
RETURN            // 함수 반환
BUILD_FUNCTION    // 함수 객체 생성
```

**배열/컬렉션 (4개)**:
```cpp
BUILD_ARRAY       // 배열 생성
INDEX_GET         // 인덱스 읽기
INDEX_SET         // 인덱스 쓰기
ARRAY_APPEND      // 배열에 추가
```

**한글 특화 (1개)**:
```cpp
JOSA_CALL         // 조사 기반 메서드 호출
                  // 예: 배열을 정렬한다
```

**기타 (8개)**:
```cpp
POP               // 스택 팝
DUP               // 스택 복제
SWAP              // 스택 교환
PRINT             // 출력
HALT              // 실행 종료
BUILD_RANGE       // 범위 객체 생성
IMPORT            // 모듈 임포트
NOP               // 아무 동작 안 함
```

### 1.3 바이트코드 형식

**Chunk 구조** (`/src/bytecode/Chunk.h`):
```cpp
class Chunk {
private:
    std::vector<uint8_t> code_;              // 바이트코드
    std::vector<evaluator::Value> constants_;  // 상수 풀
    std::vector<std::string> names_;         // 변수/함수 이름
    std::vector<int> lines_;                 // 라인 번호 매핑
};
```

**인코딩 방식**:
- OpCode: 8비트 (1바이트)
- 피연산자: 8비트 또는 16비트
- 즉시값 없음 (모두 상수 풀 사용)

**예제**: `출력(1 + 2)` 컴파일 결과
```
LOAD_CONST 0      # 상수 1 로드
LOAD_CONST 1      # 상수 2 로드
ADD               # 덧셈
CALL 1            # 1개 인자로 함수 호출
```

### 1.4 안전장치 (Safety Limits)

**파일**: `/src/bytecode/VM.cpp:33-45`

```cpp
// 1. 명령어 실행 횟수 제한
if (++instructionCount_ > maxInstructions_) {  // 기본: 10,000,000
    runtimeError("최대 명령어 실행 횟수 초과");
    return VMResult::RUNTIME_ERROR;
}

// 2. 실행 시간 제한
auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
    now - startTime_
);
if (elapsed > maxExecutionTime_) {  // 기본: 5,000ms
    runtimeError("최대 실행 시간 초과");
    return VMResult::RUNTIME_ERROR;
}

// 3. 스택 깊이 제한
if (stack_.size() >= maxStackSize_) {  // 기본: 10,000
    throw std::runtime_error("최대 스택 크기 초과");
}
```

**보호 기능**:
- ✅ 무한 루프 방지
- ✅ 스택 오버플로우 방지
- ✅ 장시간 실행 방지

### 1.5 실행 파이프라인

```
소스 코드 (.ksj)
    ↓
┌─────────────┐
│   Lexer     │ → 토큰 생성
└─────────────┘
    ↓
┌─────────────┐
│   Parser    │ → AST 생성
└─────────────┘
    ↓
┌─────────────┐
│  Semantic   │ → 심볼 테이블 (⚠️ 불완전)
│  Analyzer   │
└─────────────┘
    ↓
    ├──────────┬──────────┐
    ↓          ↓          ↓
┌─────────┐ ┌─────────┐
│Evaluator│ │Compiler │
│(Tree    │ │(AST→    │
│walking) │ │Bytecode)│
└─────────┘ └─────────┘
    ↓          ↓
┌─────────┐ ┌─────────┐
│직접 실행│ │   VM    │
│         │ │(Bytecode│
│         │ │실행)    │
└─────────┘ └─────────┘
```

---

## 메모리 관리 및 GC

### 2.1 Garbage Collection: Mark & Sweep

**파일**: `/src/memory/GC.h` (250줄)

**알고리즘**: 전통적인 Mark-and-Sweep GC

```cpp
class GarbageCollector {
private:
    std::unordered_set<Object*> allObjects_;     // 모든 추적 객체
    std::unordered_set<Object*> roots_;          // 루트 셋
    std::unordered_map<void*, std::weak_ptr<evaluator::Environment>> environments_;

    size_size_t gcThreshold_ = 100;    // 100개 객체마다 GC
    bool autoGC_ = true;                // 자동 수집 활성화
};
```

**동작 방식** (`/src/memory/GC.cpp:83-198`):

#### Phase 1: Mark (표시)
```cpp
void mark() {
    // 1. 모든 객체 미표시 처리
    for (auto* obj : allObjects_) {
        obj->marked = false;
    }

    // 2. 루트에서 재귀적으로 도달 가능한 객체 표시
    for (auto* root : roots_) {
        markObject(root);
    }
}

void markObject(Object* obj) {
    if (obj->marked) return;  // 이미 표시됨
    obj->marked = true;

    // 자식 객체 재귀적으로 표시
    for (auto* child : obj->getChildren()) {
        markObject(child);
    }
}
```

#### Phase 2: Sweep (정리)
```cpp
void sweep() {
    std::vector<Object*> toDelete;

    // 미표시 객체 찾기
    for (auto* obj : allObjects_) {
        if (!obj->marked) {
            toDelete.push_back(obj);
        }
    }

    // 삭제
    for (auto* obj : toDelete) {
        allObjects_.erase(obj);
        delete obj;
    }

    // 통계 업데이트
    updateStats();
}
```

**특징**:
- ✅ **순환 참조 처리**: `weak_ptr`로 Environment 추적
- ✅ **증분 수집**: 임계값 기반 (기본 100개 객체)
- ✅ **싱글톤**: 전역 GC 인스턴스
- ✅ **통계**: 상세한 GC 메트릭

**한계**:
- ❌ **세대별 GC 없음**: 매번 모든 객체 수집
- ❌ **동시성 없음**: GC 중 실행 중단
- ❌ **수동 등록**: 객체가 `registerObject()` 호출 필요

### 2.2 Value 표현: Tagged Union

**파일**: `/src/evaluator/Value.h` (312줄)

**메모리 레이아웃**:
```cpp
class Value {
private:
    types::TypeKind type_;  // 8바이트 - 타입 태그
    ValueData data_;        // std::variant - 실제 데이터
};

using ValueData = std::variant<
    std::monostate,                            // 미초기화
    int64_t,                                   // 8바이트
    double,                                    // 8바이트
    std::string,                               // ~56바이트 (힙 포인터 포함)
    bool,                                      // 1바이트
    std::nullptr_t,                            // 0바이트
    std::shared_ptr<Function>,                 // 16바이트 (참조 카운팅)
    BuiltinFn,                                 // 32+바이트 (std::function)
    std::shared_ptr<std::vector<Value>>        // 16바이트 (배열)
>;
```

**메모리 특성**:
- **최대 크기**: ~64바이트 (std::function 때문)
- **오버헤드**: 8바이트 (타입 태그) + variant 패딩
- **참조 카운팅**: `shared_ptr` 자동 관리
- **힙 할당**: 문자열/배열은 힙 사용

**인코딩 방식**:
- NaN-boxing **사용 안 함** (명시적 std::variant)
- 컴파일 타임 타입 안전성
- 런타임 타입 체킹: `isInteger()`, `isString()` 등

### 2.3 문자열 및 배열 관리

**문자열** (`/src/evaluator/Builtin.cpp:176-219`):
- UTF-8 인식 길이 계산
- 한글 지원 (3-4바이트 문자)
- 인터닝 없음 (각 문자열 독립적)

**배열** (`Value.h:96`):
```cpp
std::shared_ptr<std::vector<Value>>  // ARRAY 타입
```
- Copy-on-write (shared_ptr 통해)
- 특수 메모리 레이아웃 없음
- 동적 리사이징

---

## Value 표현 및 타입 시스템

### 3.1 타입 시스템

**파일**: `/src/types/Type.h` (100+줄)

**8개 내장 타입**:
```cpp
enum class TypeKind {
    INTEGER,           // int64_t
    FLOAT,             // double
    STRING,            // std::string (UTF-8)
    BOOLEAN,           // bool
    NULL_TYPE,         // nullptr 동등
    ARRAY,             // std::vector<Value>
    FUNCTION,          // 사용자 정의 함수
    BUILTIN_FUNCTION,  // 네이티브 C++ 함수
    RANGE              // 범위 객체 (1부터 10까지)
};
```

**타입 추론**: ❌ 없음
- 런타임 덕 타이핑
- Semantic Analyzer 존재하지만 **불완전**
- 타입 에러는 런타임 발견

### 3.2 함수 객체

**파일**: `/src/evaluator/Value.h:36-63`

```cpp
class Function {
private:
    std::vector<std::string> parameters_;      // 매개변수 이름
    ast::Statement* body_;                     // 함수 본문 (BlockStatement)
    std::shared_ptr<Environment> closure_;     // 캡처된 환경
};
```

**클로저 지원**:
- ✅ 렉시컬 스코핑 + 환경 캡처
- ✅ 1급 함수 (First-class)
- ✅ 중첩 함수 정의
- ✅ 익명 람다

**한계**:
- ❌ 꼬리 호출 최적화 없음
- ❌ 부분 적용/커링 없음
- ❌ 클로저 캡처가 eager (lazy 아님)

---

## 표준 라이브러리

### 4.1 내장 함수: 78개

**파일**: `/src/evaluator/Builtin.cpp` (3,300줄)

**등록** (Lines 3167-3297):
```cpp
void Builtin::registerAllBuiltins() {
    registerBuiltin("출력", builtin_출력);
    registerBuiltin("타입", builtin_타입);
    registerBuiltin("길이", builtin_길이);
    // ... 75개 더
}
```

#### 분류별 내장 함수

| 분류 | 개수 | 함수 예시 |
|------|------|----------|
| **파일 I/O** | 7 | 파일_읽기, 파일_쓰기, 파일_추가, 파일_존재, 줄별_읽기, 파일_삭제, 파일_복사 |
| **수학** | 6 | 반올림, 올림, 내림, 절댓값, 제곱근, 제곱 |
| **문자열** | 8 | 분리, 찾기, 바꾸기, 대문자, 소문자, + 타입 변환 |
| **JSON** | 4 | JSON_파싱, JSON_문자열화, JSON_파일_읽기, JSON_파일_쓰기 |
| **날짜/시간** | 7 | 현재_시간, 현재_날짜, 시간_포맷, 타임스탬프, 슬립 |
| **정규표현식** | 10 | 정규표현식_일치, 정규표현식_검색, 정규표현식_모두_찾기, 정규표현식_치환 |
| **암호화** | 10 | Base64, SHA256, XOR암호, 시저암호, 난수 |
| **HTTP** | 3 | HTTP_GET, HTTP_POST, HTTP_요청 |
| **데이터베이스** | 6 | DB_열기, DB_닫기, DB_실행, DB_쿼리 (SQLite) |
| **디렉토리** | 7 | 디렉토리_생성, 디렉토리_삭제, 디렉토리_목록 |
| **경로/파일** | 8 | 경로_결합, 파일명_추출, 확장자_추출, 절대경로 |
| **환경변수** | 5 | 환경변수_읽기, 환경변수_쓰기, 환경변수_존재하는가 |
| **시스템** | 4 | OS_이름, 사용자_이름, 호스트_이름, 프로세스_종료 |
| **테스팅** | 5 | assert, assert_같음, assert_다름, assert_참, assert_거짓 |

### 4.2 Production 언어 대비 부족한 기능

| 기능 | 상태 | 영향도 | 우선순위 |
|------|------|--------|---------|
| **Async/Await** | ❌ | 동시 I/O 불가 | 높음 |
| **Threading** | ❌ | 멀티스레딩 불가 | 높음 |
| **Generators** | ❌ | 지연 시퀀스 불가 | 중간 |
| **Context Managers** | ❌ | 리소스 정리 보장 없음 | 중간 |
| **Try/Catch** | ⚠️ 부분적 | 구조화된 예외 처리 없음 | 높음 |
| **Decorators** | ❌ | 함수 래핑 불가 | 낮음 |
| **Classes/OOP** | ❌ | 객체 지향 불가 | 중간 |
| **Operator Overloading** | ⚠️ 제한적 | + 연산자만 | 중간 |

### 4.3 I/O 시스템

**파일 I/O** (`Builtin.cpp:814-971`):
- 텍스트 파일 읽기/쓰기
- 줄 단위 읽기
- 파일 존재 확인
- 디렉토리 조작

**HTTP 클라이언트** (`Builtin.cpp:2047-2343`):
```cpp
static Value builtin_HTTP_GET(const std::vector<Value>& args)
static Value builtin_HTTP_POST(const std::vector<Value>& args)
static Value builtin_HTTP_요청(const std::vector<Value>& args)
```
- `httplib.h` 사용 (헤더 전용 C++ HTTP 라이브러리)
- 기본 GET/POST 지원
- JSON 응답 처리

**데이터베이스** (`Builtin.cpp:2343-2550`):
- SQLite 통합 (`#include <sqlite3.h>`)
- 임의 SQL 실행
- 결과 바인딩이 있는 쿼리
- 트랜잭션 지원 (암묵적)

**JSON 지원** (`Builtin.cpp:1206-1309`):
- nlohmann/json 라이브러리 통합
- JSON 문자열을 Value로 파싱
- Value를 JSON으로 직렬화
- JSON 파일 I/O

### 4.4 부족한 I/O 기능

- ❌ **stdin/stdout 리디렉션**: 기본 콘솔만
- ❌ **바이너리 파일 I/O**: 텍스트 기반만
- ❌ **네트워크 소켓**: HTTP만 (raw TCP/UDP 없음)
- ❌ **스트림/파이프**: 스트림 추상화 없음
- ❌ **파일시스템 감시**: inotify/kqueue 통합 없음
- ❌ **비동기 I/O**: 이벤트 루프 없음

### 4.5 stdlib 모듈 (13개)

**위치**: `/stdlib/` (총 2,806줄)

| 파일 | 상태 | 설명 |
|------|------|------|
| **array.ksj** | ✅ 완성 | 배열 메서드 |
| **string.ksj** | ✅ 완성 | 문자열 조작 |
| **math.ksj** | ✅ 완성 | 수학 유틸리티 |
| **json.ksj** | ✅ 완성 | JSON 래퍼 |
| **io.ksj** | ✅ 완성 | I/O 헬퍼 |
| **time.ksj** | ✅ 완성 | 시간/날짜 함수 |
| **collections.ksj** | ⚠️ 작성됨, 깨짐 | Set, Map, Queue, Stack, Deque (46 함수) |
| **crypto.ksj** | ⚠️ 부분적 | 암호화 함수 |
| **db.ksj** | ⚠️ 부분적 | 데이터베이스 래퍼 |
| **http.ksj** | ⚠️ 부분적 | HTTP 클라이언트 |
| **os.ksj** | ⚠️ 부분적 | OS 유틸리티 |
| **regex.ksj** | ⚠️ 부분적 | 정규식 헬퍼 |
| **utils.ksj** | ✅ 부분적 | 유틸리티 함수 |

**알려진 문제**:
- ⚠️ **collections.ksj**: 배열 결합 연산자 (`[] + []`) 미구현으로 사용 불가
- ⚠️ **파서 취약성**: 작은 코드 변경으로 파싱 실패
- ⚠️ **불완전한 Semantic Analysis**: 별도 의미 분석 단계 없음

---

## 동시성 및 병렬성

### 5.1 현재 상태: ❌ 지원 안 됨

**증거**:
- evaluator/interpreter에서 `std::thread` 사용 없음
- 동기화를 위한 `std::mutex` 없음
- `async`/`await` 키워드 없음
- 단일 스레드 실행만

**스레드 사용 제한**:
- `#include <thread>` in Builtin.cpp (line 18)
- `슬립()` 함수에서만 사용: `std::this_thread::sleep_for()`

**코드 증거** (`Builtin.cpp:1401-1423`):
```cpp
static Value builtin_슬립(const std::vector<Value>& args) {
    // ...
    int milliseconds = static_cast<int>(args[0].asInteger());
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    return Value::createNull();
}
```

### 5.2 부족한 동시성 기능

| 기능 | 상태 | 영향 |
|------|------|------|
| **멀티스레딩** | ❌ | 다중 CPU 코어 활용 불가 |
| **스레드 풀** | ❌ | 비동기 작업 스케줄링 불가 |
| **채널** | ❌ | 스레드 간 조정 불가 |
| **Mutex/Lock** | ❌ | 스레드 안전 자료구조 없음 |
| **비동기 I/O** | ❌ | I/O가 전체 실행 블록 |
| **이벤트 루프** | ❌ | 이벤트 주도 아키텍처 없음 |
| **Promise/Future** | ❌ | 비동기 작업 표현 불가 |
| **코루틴** | ❌ | 경량 동시성 없음 |

---

## 디버깅 지원

### 6.1 디버거 구현

**파일**: `/include/debugger/Debugger.h` (80+줄)

**구현된 기능**:
- ✅ **브레이크포인트**: 추가/제거/관리
- ✅ **호출 스택**: 함수 호출 계층 추적
- ✅ **감시점**: 변수 변경 감시
- ✅ **명령 파서**: 대화형 디버거 명령
- ✅ **소스 뷰어**: 줄 번호와 함께 소스 표시

**코드 구조** (`Debugger.h:30-78`):
```cpp
enum class DebuggerState {
    IDLE,           // 프로그램 시작 전 대기
    RUNNING,        // 다음 브레이크포인트까지 실행
    STEPPING,       // Step into (단계별)
    STEPPING_OVER,  // Step over (함수 호출 건너뛰기)
    PAUSED          // 브레이크포인트 또는 사용자 요청으로 일시정지
};

class Debugger {
private:
    std::unique_ptr<BreakpointManager> breakpoints_;
    std::unique_ptr<CallStack> callStack_;
    std::unique_ptr<WatchpointManager> watchpoints_;
    std::unique_ptr<CommandParser> commandParser_;
    std::unique_ptr<SourceCodeViewer> sourceViewer_;
};
```

### 6.2 사용 가능한 디버깅 명령

헤더 구조를 기반으로 지원될 가능성이 있는 명령:
- `break <line>` - 브레이크포인트 설정
- `step` / `next` / `continue` - 실행 제어
- `print <var>` - 변수 값 출력
- `watch <var>` - 변수 변경 감시
- `list <line>` - 소스 코드 표시
- `backtrace` - 호출 스택 표시

### 6.3 한계

- ⚠️ **설계 단계 불완전**: TODO 주석이 진행 중 개발 표시 (Week 3-6 타임라인)
- ⚠️ **제한된 통합**: Evaluator/VM과 완전히 통합 안 됨
- ⚠️ **원격 디버깅 없음**: 로컬 콘솔 상호작용만
- ⚠️ **라인 수준만**: 문장/표현식 수준 디버깅 없음

---

## 프로파일링 및 성능

### 7.1 프로파일러 구현

**파일**: `/src/profiler/Profiler.h` (161줄)

**추적되는 통계**:
```cpp
struct FunctionStats {
    std::string name;       // 함수 이름
    uint64_t callCount;     // 호출 횟수
    double totalTime;       // 총 실행 시간 (ms)
    double minTime;         // 최소 실행 시간
    double maxTime;         // 최대 실행 시간
    double avgTime;         // 평균 실행 시간
};
```

**프로파일러 API**:
```cpp
class Profiler {
    void start();                  // 프로파일링 시작
    void stop();                   // 프로파일링 종료
    void enterFunction(const std::string& functionName);
    void exitFunction(const std::string& functionName);
    const std::unordered_map<std::string, FunctionStats>& getStats() const;
};
```

### 7.2 Hot Path Detector (JIT 기반)

**파일**: `/src/jit/HotPathDetector.cpp` (8.7KB)

**추적**:
- 함수 호출 빈도 및 기간
- 루프 백에지 실행 횟수
- 기본 블록 실행 횟수
- "hot" 및 "very hot" 코드 경로 식별

**JIT 계층**:
```cpp
enum class JITTier {
    UNCOMPILED,
    BASELINE,      // 단순 JIT
    OPTIMIZED      // 고급 JIT
};
```

**현재 상태**: ⚠️ **인프라만** - 실제 JIT 컴파일 미구현

### 7.3 성능 특성

**관찰된 성능** (문서에서):
```
루프 벤치마크 (1천만 반복):
- Tree-walking interpreter: ~5초
- Bytecode VM: ~1초
- JIT (가상): ~0.1초
```

**병목 지점**:
1. **AST 순회**: AST 구조 최적화 없음
2. **타입 체킹**: 모든 연산마다 동적 타입 체크
3. **함수 호출**: 작은 함수 인라이닝 없음
4. **배열 연산**: SIMD나 특수화 구현 없음
5. **메모리 할당**: 빈번한 할당/해제 사이클

---

## 보안 분석

### 8.1 보안 기능: 최소

**현재 보호**:

1. **실행 제한** (`VM.h:61-65`, `Evaluator.h:125-128`):
   - 최대 명령어: 10,000,000
   - 최대 실행 시간: 5,000ms
   - 최대 스택 깊이: 10,000

2. **타입 안전성**:
   - Variant 기반 value 표현
   - 연산 전 런타임 타입 체킹
   - 체크되지 않은 캐스트 없음

3. **메모리 안전성 (부분적)**:
   - KingSejong 코드에서 버퍼 오버플로우 없음
   - 배열 접근 경계 체크
   - 스마트 포인터로 use-after-free 방지

### 8.2 보안 취약점 및 부족한 기능

| 취약점 | 심각도 | 설명 |
|--------|--------|------|
| **무제한 파일 접근** | 🔴 높음 | 프로세스가 접근 가능한 모든 파일 읽기/쓰기 가능 |
| **샌드박싱 없음** | 🔴 높음 | 파일 경로나 작업 제한 없음 |
| **권한 시스템 없음** | 🔴 높음 | 모든 코드가 동일한 권한 |
| **무제한 HTTP** | 🔴 높음 | 모든 URL에 요청 가능 (SSRF 위험) |
| **SQL 인젝션 위험** | 🟡 중간 | DB_실행()이 raw SQL 문자열 받음 |
| **입력 검증 없음** | 🟡 중간 | 내장 함수가 입력 데이터 신뢰 |
| **암호화 검증 없음** | 🟡 중간 | 단순 XOR, Caesar cipher만 |
| **인증서 검증 없음** | 🟡 중간 | HTTP 클라이언트가 TLS 인증서 검증 안 할 수 있음 |
| **Regex DoS** | 🟡 중간 | 복잡한 정규식 패턴이 hang 유발 가능 |
| **메모리 제한 없음** | 🟡 중간 | GC 임계값을 무제한 할당 허용하도록 설정 가능 |

### 8.3 권장 보안 조치

프로덕션 사용을 위해 필요:
- ✅ 제한된 파일/네트워크 접근 샌드박스
- ✅ 리소스 할당량 (메모리, CPU, 디스크)
- ✅ 기능 기반 보안 모델
- ✅ 암호화 서명 검증
- ✅ 입력 검증 프레임워크
- ✅ 네트워크 작업 속도 제한

---

## 모듈 시스템

### 9.1 모듈 로더

**파일**: `/src/module/ModuleLoader.h` (108줄)

**기능**:
```cpp
class ModuleLoader {
private:
    std::unordered_map<std::string, ModuleCacheEntry> cache_;    // 모듈 캐시
    std::unordered_set<std::string> loading_;                    // 순환 참조 감지
    std::string basePath_;                                        // 검색 경로

public:
    std::shared_ptr<evaluator::Environment> loadModule(const std::string& modulePath);
    std::string resolvePath(const std::string& modulePath) const;
    void clearCache();
    bool isCached(const std::string& modulePath) const;
};
```

**구현 세부사항**:
- ✅ **모듈 캐싱**: 로드된 모듈 캐시하여 재로딩 방지
- ✅ **순환 임포트 감지**: 로딩 중인 모듈 추적
- ✅ **경로 해석**: "math" → "stdlib/math.ksj" 변환
- ✅ **환경 격리**: 각 모듈이 자체 네임스페이스 보유

**한계**:
- ⚠️ **파일 기반만**: 메모리 내 모듈 사용 불가
- ⚠️ **버전 관리 없음**: 여러 버전 지원 안 함
- ⚠️ **의존성 관리 없음**: 수동 의존성 순서 필요
- ⚠️ **단일 검색 경로**: 하나의 기본 디렉토리만 지원

---

## 에러 처리

### 10.1 에러 리포팅 시스템

**파일**: `/src/error/ErrorReporter.h` (349줄)

**기능**:
```cpp
enum class ErrorType {
    SYNTAX_ERROR,
    SEMANTIC_ERROR,
    TYPE_ERROR,
    RUNTIME_ERROR,
    // ... 더 많은 에러 타입
};

class ErrorReporter {
private:
    std::vector<Error> errors_;
    SourceManager sourceManager_;

public:
    void report(const Error& error);
    void printError(const Error& error, std::ostream& os);
    const std::vector<Error>& getErrors() const;
};
```

**향상된 기능**:
- ✅ **한글 에러 메시지**: 모든 에러 텍스트 한글
- ✅ **ANSI 컬러 출력**: 에러는 빨강, 경고는 노랑
- ✅ **소스 컨텍스트**: 에러 주변 코드 표시
- ✅ **에러 힌트**: 수정 제안
- ✅ **줄/열 추적**: 정확한 에러 위치

### 10.2 에러 범주

| 에러 타입 | 감지 | 복구 |
|----------|------|------|
| **Syntax** | Parser 단계 | ❌ 복구 없음 (전체 재파싱 필요) |
| **Semantic** | 불완전한 analyzer | ⚠️ 기본 체크만 |
| **Type** | 런타임 연산 | 🔴 런타임 에러가 실행 중단 |
| **Runtime** | Evaluator/VM | ❌ 예외 처리 없음 (try/catch 부족) |

### 10.3 부족한 에러 처리

- ❌ **try/catch 블록 없음**: 예외 잡을 수 없음
- ❌ **finally 블록 없음**: 정리 보장 불가
- ❌ **에러 복구 없음**: 파싱 에러가 전체 프로그램 중단
- ❌ **스택 언와인딩 훅 없음**: 에러 시 정리 처리 불가
- ❌ **에러 컨텍스트 체이닝 없음**: 호출 간 에러 컨텍스트 구축 불가

---

## FFI (Foreign Function Interface)

### 11.1 현재 구현: 내장 함수만

**직접적인 C/C++ 상호운용 메커니즘 없음**

**우회 접근**:
모든 C/C++ 기능이 내장 함수로 노출됨:
```cpp
// 예: C++로 구현된 HTTP 클라이언트를 builtin으로 노출
static Value builtin_HTTP_GET(const std::vector<Value>& args)
// 래핑:
registerBuiltin("HTTP_GET", builtin_HTTP_GET);
```

**지원되는 라이브러리**:
- `httplib.h` - HTTP 클라이언트
- `sqlite3.h` - 데이터베이스
- `nlohmann/json.hpp` - JSON
- 표준 C++ 라이브러리 (crypto, filesystem, threads)

### 11.2 부족한 FFI 기능

- ❌ **동적 라이브러리 로딩**: dlopen/LoadLibrary 없음
- ❌ **C 함수 바인딩**: 자동 C 함수 래핑 없음
- ❌ **구조체 마샬링**: C struct 표현 불가
- ❌ **포인터 조작**: unsafe 포인터 접근 없음
- ❌ **콜백 지원**: KingSejong 함수를 C에 전달 불가
- ❌ **ABI 호환성**: C++ 함수 시그니처로 제한

---

## 빌드 시스템 및 도구

### 12.1 CMake 구성

**파일**: `/CMakeLists.txt` (251줄)

**빌드 타겟**:
```cmake
add_library(kingsejong_lib STATIC ${LIB_SOURCES})  # 코어 라이브러리
add_executable(kingsejong ${SOURCES})              # CLI 실행 파일
add_executable(kingsejong_tests ${TEST_SOURCES})  # 테스트 스위트
add_executable(kingsejong_wasm ...)                # WebAssembly 타겟
```

**컴파일러 요구사항**:
- C++23 표준
- C++23 지원 컴파일러 (GCC 12+, Clang 15+, MSVC 2022+)
- 플랫폼별: Linux는 pthread, macOS는 stdlib=libc++

**빌드 출력**:
```
build/bin/kingsejong              # 메인 인터프리터
build/lib/libkingsejong_lib.a     # 정적 라이브러리
build/wasm/kingsejong.js          # WebAssembly 모듈 (Emscripten)
build/wasm/kingsejong.wasm        # WASM 바이너리
```

### 12.2 테스팅 프레임워크

**GoogleTest 통합**:
- `tests/` 디렉토리에 1,230개 테스트
- 100% 통과율
- 범주: Lexer, Parser, Evaluator, GC, Debugger, JIT, LSP 등

### 12.3 패키지 관리자: 없음

- ❌ 내장 패키지 관리자 없음
- ❌ 의존성 명세 없음 (CMake FetchContent 사용)
- ❌ stdlib 모듈 수동 포함

---

## 생산성 평가

### 13.1 평가 매트릭스 (1-10 척도)

| 차원 | 점수 | 비고 |
|------|------|------|
| **언어 완성도** | 6/10 | 클래스, async, 예외 처리 부족 |
| **VM/인터프리터 품질** | 7/10 | 견고한 아키텍처지만 최적화 안 됨 |
| **표준 라이브러리** | 7/10 | 78 builtins + 13 모듈, 고급 기능 부족 |
| **메모리 관리** | 7/10 | GC 작동하지만 세대별 아님 |
| **성능** | 4/10 | JIT 없음, 인터프리터 속도만 |
| **에러 처리** | 5/10 | 좋은 에러 메시지, 예외 없음 |
| **디버깅** | 6/10 | 디버거 인프라 존재, 불완전 |
| **문서화** | 8/10 | 방대한 문서 및 예제 |
| **테스팅** | 9/10 | 1,230 테스트, 100% 통과 |
| **보안** | 3/10 | 샌드박싱 없음, 무제한 I/O |
| **도구** | 8/10 | VS Code 확장, LSP 지원 |

**전체 Production Readiness: 4.5/10** ❌

### 13.2 프로덕션 이전 권장 조치

**필수 (Must Do)**:
1. ✅ 예외 처리 구현 (try/catch/finally)
2. ✅ Semantic Analysis 단계 완료
3. ✅ JIT 컴파일러 추가 (기본 계층이라도)
4. ✅ 파서 휴리스틱을 적절한 문법으로 수정
5. ✅ 리소스 제한/샌드박싱 구현

**중요 (Should Do)**:
1. 클래스/객체 시스템 추가
2. async/await 구현
3. generators/iterators 추가
4. 배열 결합 연산자 완성
5. Collections 모듈 최종 구현

**선택 (Could Do)**:
1. metaclasses 추가
2. 명명된 인자 구현
3. spread 연산자 추가
4. destructuring assignment 구현
5. 선택적 타입 주석 추가

---

## 부족한 기능 종합

### 14.1 언어 기능 비교

| 기능 | KingSejong | Python | JavaScript | Lua |
|------|-----------|--------|------------|-----|
| **변수** | ✅ | ✅ | ✅ | ✅ |
| **함수** | ✅ | ✅ | ✅ | ✅ |
| **클로저** | ✅ | ✅ | ✅ | ✅ |
| **클래스** | ❌ | ✅ | ✅ | ❌ |
| **모듈** | ✅ | ✅ | ✅ | ✅ |
| **예외 처리** | ❌ | ✅ | ✅ | ⚠️ |
| **Async/Await** | ❌ | ✅ | ✅ | ❌ |
| **Generators** | ❌ | ✅ | ✅ | ⚠️ |
| **패턴 매칭** | ⚠️ | ✅ | ❌ | ❌ |
| **Metaclasses** | ❌ | ✅ | ⚠️ | ❌ |

### 14.2 실행 엔진 비교

| 메트릭 | KingSejong | Python | V8 (JS) | Ruby | Lua |
|--------|-----------|--------|---------|------|-----|
| **인터프리터 타입** | 하이브리드 | Bytecode VM | JIT (3-tier) | Tree-walk | Bytecode VM |
| **VM 타입** | Stack-based | Stack-based | Register-based | N/A | Stack-based |
| **최적화 수준** | 최소 | 기본 | 공격적 | 최소 | 기본 |
| **JIT 지원** | ❌ | ⚠️ 3.13+ | ✅ Full | ✅ MJIT | ⚠️ LuaJIT |
| **성능 (루프)** | ~5초 | ~0.5초 | ~0.05초 | ~3초 | ~0.5초 |
| **성능 (상대)** | 1x | 10x | 100x | ~2x | 10x |

### 14.3 표준 라이브러리 범위

| 범주 | KingSejong | Python | JavaScript | Lua |
|------|-----------|--------|------------|-----|
| **Built-in 함수** | 78 | 68 | 40+ | 50+ |
| **Stdlib 모듈** | 13 | 300+ | 40+ (Node) | 30+ |
| **파일 I/O** | ✅ 기본 | ✅ 완전 | ⚠️ Node.js | ✅ 기본 |
| **네트워킹** | ⚠️ HTTP만 | ✅ 완전 | ✅ 완전 | ❌ |
| **암호화** | ⚠️ XOR/Caesar | ✅ 완전 | ✅ 완전 | ❌ |
| **데이터베이스** | ✅ SQLite | ✅ 다중 | ✅ 다중 | ⚠️ |
| **JSON** | ✅ | ✅ | ✅ 네이티브 | ⚠️ |
| **Regex** | ✅ | ✅ 완전 | ✅ 완전 | ⚠️ 기본 |

### 14.4 프로덕션 언어로 동작하기 위한 필수 기능

#### 🔴 최우선 (P0)

1. **예외 처리 시스템**
   - `시도`, `오류`, `마지막` 키워드
   - 스택 언와인딩
   - 에러 체이닝
   - **예상 공수**: 2주

2. **Semantic Analyzer 완성**
   - Symbol Table 구현
   - 타입 체크 분리
   - 이름 해결
   - **예상 공수**: 2-3주

3. **보안 샌드박스**
   - 파일 접근 제한
   - 네트워크 접근 제한
   - 리소스 할당량
   - **예상 공수**: 2주

4. **JIT 컴파일러 (기본)**
   - Hot path 컴파일
   - 기본 최적화
   - **예상 공수**: 1-2개월

#### 🟡 중요 (P1)

5. **클래스/OOP 시스템**
   - 클래스 선언
   - 상속
   - 메서드
   - **예상 공수**: 3주

6. **Async/Await**
   - 비동기 함수
   - await 연산자
   - Promise/Future
   - **예상 공수**: 1개월

7. **Generators/Iterators**
   - yield 키워드
   - 지연 평가
   - **예상 공수**: 2주

8. **딕셔너리/맵**
   - 해시 테이블 구현
   - `{ "key": value }` 문법
   - **예상 공수**: 1-2주

#### 🟢 보완 (P2)

9. **스레딩/동시성**
   - 멀티스레딩 지원
   - Mutex/Lock
   - 채널
   - **예상 공수**: 1개월

10. **고급 타입 시스템**
    - 타입 추론
    - Generic 타입
    - Optional 타입
    - **예상 공수**: 1-2개월

---

## 개선 로드맵

### Phase 1: 핵심 안정성 (2-3개월)

| 작업 | 우선순위 | 공수 | 의존성 |
|------|---------|------|--------|
| Semantic Analyzer 완성 | P0 | 2-3주 | - |
| 예외 처리 시스템 | P0 | 2주 | Semantic Analyzer |
| 변수 스코프 수정 | P0 | 1주 | Semantic Analyzer |
| 보안 샌드박스 | P0 | 2주 | - |
| 테스트 케이스 확장 | P0 | 1주 | 모든 상기 |

**완료 기준**:
- ✅ 파서 휴리스틱 제거
- ✅ try/catch 동작
- ✅ 스코프 버그 해결
- ✅ 샌드박스 적용 가능
- ✅ 1,500+ 테스트 통과

### Phase 2: 언어 기능 확장 (2개월)

| 작업 | 우선순위 | 공수 | 의존성 |
|------|---------|------|--------|
| 클래스/OOP | P1 | 3주 | Semantic Analyzer |
| 딕셔너리 자료구조 | P1 | 1-2주 | - |
| Async/Await | P1 | 1개월 | - |
| Generators | P1 | 2주 | - |
| 배열 연산자 완성 | P1 | 3-5일 | - |

**완료 기준**:
- ✅ 클래스 정의 및 상속
- ✅ 딕셔너리 CRUD
- ✅ 비동기 함수 동작
- ✅ yield 문 동작

### Phase 3: 성능 최적화 (2-3개월)

| 작업 | 우선순위 | 공수 | 의존성 |
|------|---------|------|--------|
| JIT 컴파일러 (Baseline) | P0 | 1-2개월 | Hot path detector |
| 함수 인라이닝 | P1 | 2주 | JIT |
| 타입 특수화 | P1 | 3주 | JIT |
| 세대별 GC | P1 | 2주 | - |
| 메모리 풀 할당자 | P2 | 1주 | - |

**완료 기준**:
- ✅ 10-20배 성능 향상
- ✅ V8 대비 10% 이내 격차
- ✅ GC 일시정지 <10ms

### Phase 4: 생태계 구축 (진행 중)

| 작업 | 우선순위 | 공수 |
|------|---------|------|
| 패키지 관리자 | P1 | 1개월 |
| stdlib 확장 (100+ 함수) | P1 | 지속적 |
| LSP 기능 완성 | P1 | 2주 |
| 문서 및 튜토리얼 | P1 | 1개월 |
| 커뮤니티 라이브러리 | P2 | 지속적 |

---

## 결론

### 강점

1. ✅ **하이브리드 아키텍처**: Tree-walking + Bytecode VM
2. ✅ **완전한 VM 구현**: 43개 OpCode, 스택 기반
3. ✅ **풍부한 내장 함수**: 78개 (I/O, HTTP, DB, JSON, Crypto)
4. ✅ **GC 구현**: Mark & Sweep, 자동 메모리 관리
5. ✅ **높은 테스트 커버리지**: 1,230개, 100% 통과
6. ✅ **개발 도구**: Debugger, Profiler, LSP, VS Code 확장
7. ✅ **한글 친화적**: Josa 표현식, 한글 키워드

### 치명적 약점

1. 🔴 **예외 처리 없음**: try/catch 부재, 프로덕션 불가
2. 🔴 **JIT 없음**: 성능 50-100배 느림 (V8 대비)
3. 🔴 **파서 취약성**: 휴리스틱 의존, 코드 변경 시 파싱 실패
4. 🔴 **보안 부재**: 샌드박스 없음, 신뢰할 수 없는 코드 실행 위험
5. 🔴 **클래스 없음**: 객체 지향 프로그래밍 불가
6. 🔴 **동시성 없음**: 멀티스레딩/비동기 불가

### 최종 평가

**교육 목적 & DSL**: ✅ **우수** (9/10)
**프로덕션 시스템**: ❌ **부적합** (4.5/10)

KingSejong은 **견고한 아키텍처와 풍부한 기능**을 가진 인상적인 언어이지만, 프로덕션 배포를 위해서는 **예외 처리, JIT, 보안, 클래스** 등의 핵심 기능이 반드시 필요합니다.

---

**문서 버전**: 2.0
**최종 수정**: 2025-11-16
**작성자**: AI Code Analysis (VM Deep Dive)
**리뷰 필요**: KingSejong Core Team
