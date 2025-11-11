# KingSejong 디버거 설계 문서

> **작성일**: 2025-11-11
> **버전**: v0.3.0 (Phase 5 - F5.2)
> **상태**: 설계 중

---

## 1. 개요

KingSejong 언어의 대화형 디버거를 구현합니다. GDB 스타일의 명령어 인터페이스를 제공하며, REPL과 통합되어 직관적인 디버깅 경험을 제공합니다.

### 1.1 목표

- ✅ **브레이크포인트**: 특정 줄에서 실행 일시 정지
- ✅ **단계별 실행**: step, next, continue 명령어
- ✅ **변수 검사**: 지역/전역 변수 출력
- ✅ **콜 스택**: 함수 호출 체인 추적
- ✅ **조건부 브레이크포인트**: 조건식 기반 중단
- ✅ **와치포인트**: 변수 변경 감지

### 1.2 코드 품질 기준 준수

본 설계는 `.claude/system_prompt_additions.md`의 모든 기준을 준수합니다:

- **RAII**: 모든 리소스를 스마트 포인터로 관리
- **예외 처리**: abort()/exit() 사용 금지, 적절한 예외 처리
- **TDD**: 테스트 먼저 작성, 구현 후
- **메모리 안전성**: AddressSanitizer/Valgrind 검증
- **문서화**: Doxygen 스타일 주석

---

## 2. 아키텍처

### 2.1 클래스 다이어그램

```
┌─────────────────────────────────────────────┐
│              Debugger                       │
├─────────────────────────────────────────────┤
│ - breakpoints_: BreakpointManager          │
│ - watchpoints_: WatchpointManager          │
│ - callStack_: CallStack                    │
│ - state_: DebuggerState                    │
│ - evaluator_: shared_ptr<Evaluator>        │
├─────────────────────────────────────────────┤
│ + run(program: Program*): void             │
│ + setBreakpoint(file, line): bool          │
│ + step(): void                             │
│ + next(): void                             │
│ + continue(): void                         │
│ + print(variable): Value                   │
│ + backtrace(): vector<StackFrame>         │
└─────────────────────────────────────────────┘
           │
           │ uses
           ├──────────────┬──────────────┬──────────────┐
           │              │              │              │
           ▼              ▼              ▼              ▼
┌──────────────────┐ ┌──────────────┐ ┌─────────────┐ ┌────────────────┐
│BreakpointManager│ │CallStack     │ │DebuggerState│ │WatchpointMgr   │
├──────────────────┤ ├──────────────┤ ├─────────────┤ ├────────────────┤
│- breakpoints_:  │ │- frames_:    │ │- running_   │ │- watchpoints_: │
│  map<Location,  │ │  vector<     │ │- paused_    │ │  map<string,   │
│  Breakpoint>    │ │  StackFrame> │ │- currentLoc_│ │  Watchpoint>   │
├──────────────────┤ ├──────────────┤ ├─────────────┤ ├────────────────┤
│+ add()          │ │+ push()      │ │+ pause()    │ │+ add()         │
│+ remove()       │ │+ pop()       │ │+ resume()   │ │+ check()       │
│+ shouldBreak()  │ │+ current()   │ │+ isPaused() │ │+ remove()      │
└──────────────────┘ └──────────────┘ └─────────────┘ └────────────────┘
```

### 2.2 주요 컴포넌트

#### 2.2.1 Debugger (메인 클래스)

```cpp
/**
 * @class Debugger
 * @brief KingSejong 언어 대화형 디버거
 *
 * GDB 스타일의 디버깅 인터페이스를 제공합니다.
 * 브레이크포인트, 단계별 실행, 변수 검사 등을 지원합니다.
 *
 * Thread Safety: NOT thread-safe
 * Memory: RAII - 모든 리소스 자동 관리
 *
 * @invariant evaluator_ != nullptr (실행 중)
 * @invariant state_.isPaused() == true (브레이크포인트 도달 시)
 */
class Debugger {
private:
    std::unique_ptr<BreakpointManager> breakpoints_;  // RAII
    std::unique_ptr<WatchpointManager> watchpoints_;  // RAII
    std::unique_ptr<CallStack> callStack_;            // RAII
    std::unique_ptr<DebuggerState> state_;            // RAII
    std::shared_ptr<Evaluator> evaluator_;            // 공유 ownership

public:
    /**
     * @brief 디버거 생성자
     * @param evaluator 평가기 (shared ownership)
     * @throws std::invalid_argument if evaluator is null
     */
    explicit Debugger(std::shared_ptr<Evaluator> evaluator);

    // 복사 금지, 이동 허용
    Debugger(const Debugger&) = delete;
    Debugger& operator=(const Debugger&) = delete;
    Debugger(Debugger&&) noexcept = default;
    Debugger& operator=(Debugger&&) noexcept = default;

    ~Debugger() = default;  // RAII로 자동 정리

    /**
     * @brief 프로그램 실행 (디버그 모드)
     * @param program AST 프로그램
     * @throws std::invalid_argument if program is null
     * @throws error::RuntimeError 실행 중 에러
     */
    void run(ast::Program* program);
};
```

#### 2.2.2 BreakpointManager

```cpp
/**
 * @class BreakpointManager
 * @brief 브레이크포인트 관리
 *
 * 파일:줄 위치 기반 브레이크포인트를 관리합니다.
 * 조건부 브레이크포인트를 지원합니다.
 *
 * Thread Safety: NOT thread-safe
 * Memory: RAII - map으로 자동 관리
 *
 * @invariant 모든 Location은 유효한 파일명과 줄 번호를 가짐
 * @invariant 조건식이 있는 경우 유효한 표현식이어야 함
 */
class BreakpointManager {
public:
    /**
     * @struct Breakpoint
     * @brief 브레이크포인트 정보
     */
    struct Breakpoint {
        SourceLocation location;                  // 위치
        std::optional<std::string> condition;     // 조건식 (optional)
        bool enabled;                             // 활성화 여부
        int hitCount;                             // 도달 횟수

        Breakpoint(const SourceLocation& loc)
            : location(loc)
            , condition(std::nullopt)
            , enabled(true)
            , hitCount(0)
        {}
    };

private:
    std::map<SourceLocation, Breakpoint> breakpoints_;  // 자동 정렬

public:
    /**
     * @brief 브레이크포인트 추가
     * @param location 위치 (파일:줄)
     * @param condition 조건식 (optional)
     * @return 추가 성공 여부
     * @throws std::invalid_argument if location is invalid
     */
    bool add(const SourceLocation& location,
             const std::optional<std::string>& condition = std::nullopt);

    /**
     * @brief 브레이크포인트 제거
     * @param location 위치
     * @return 제거 성공 여부
     */
    bool remove(const SourceLocation& location);

    /**
     * @brief 현재 위치에서 중단해야 하는지 확인
     * @param location 현재 위치
     * @param env 환경 (조건식 평가용)
     * @return 중단 여부
     */
    bool shouldBreak(const SourceLocation& location,
                     const Environment& env) const;

    /**
     * @brief 모든 브레이크포인트 조회
     * @return 브레이크포인트 목록 (const 참조)
     */
    const std::map<SourceLocation, Breakpoint>& getAll() const {
        return breakpoints_;
    }
};
```

#### 2.2.3 CallStack

```cpp
/**
 * @class CallStack
 * @brief 함수 호출 스택 추적
 *
 * 함수 호출 체인을 추적하여 backtrace를 제공합니다.
 *
 * Thread Safety: NOT thread-safe
 * Memory: RAII - vector로 자동 관리
 *
 * @invariant frames_는 항상 유효한 스택 프레임만 포함
 * @invariant top 프레임이 현재 실행 중인 함수
 */
class CallStack {
public:
    /**
     * @struct StackFrame
     * @brief 스택 프레임 정보
     */
    struct StackFrame {
        std::string functionName;     // 함수 이름
        SourceLocation location;      // 호출 위치
        std::shared_ptr<Environment> env;  // 환경 (변수 검사용)

        StackFrame(const std::string& name,
                   const SourceLocation& loc,
                   std::shared_ptr<Environment> environment)
            : functionName(name)
            , location(loc)
            , env(std::move(environment))
        {}
    };

private:
    std::vector<StackFrame> frames_;  // RAII

public:
    /**
     * @brief 스택 프레임 추가
     * @param frame 스택 프레임
     */
    void push(StackFrame frame) {
        frames_.push_back(std::move(frame));
    }

    /**
     * @brief 스택 프레임 제거
     * @throws std::runtime_error if stack is empty
     */
    void pop() {
        if (frames_.empty()) {
            throw std::runtime_error("CallStack::pop() - stack is empty");
        }
        frames_.pop_back();
    }

    /**
     * @brief 현재 스택 프레임 조회
     * @return 현재 프레임 참조
     * @throws std::runtime_error if stack is empty
     */
    StackFrame& current() {
        if (frames_.empty()) {
            throw std::runtime_error("CallStack::current() - stack is empty");
        }
        return frames_.back();
    }

    /**
     * @brief 모든 스택 프레임 조회 (backtrace)
     * @return 스택 프레임 목록 (const 참조)
     */
    const std::vector<StackFrame>& getAll() const {
        return frames_;
    }

    /**
     * @brief 스택 깊이
     */
    size_t depth() const {
        return frames_.size();
    }

    /**
     * @brief 스택이 비어있는지 확인
     */
    bool empty() const {
        return frames_.empty();
    }
};
```

#### 2.2.4 WatchpointManager

```cpp
/**
 * @class WatchpointManager
 * @brief 와치포인트 관리
 *
 * 변수 값 변경을 감지하여 자동으로 중단합니다.
 *
 * Thread Safety: NOT thread-safe
 * Memory: RAII - map으로 자동 관리
 */
class WatchpointManager {
public:
    /**
     * @struct Watchpoint
     * @brief 와치포인트 정보
     */
    struct Watchpoint {
        std::string variableName;  // 변수 이름
        Value lastValue;           // 마지막 값
        bool enabled;              // 활성화 여부

        Watchpoint(const std::string& name, const Value& value)
            : variableName(name)
            , lastValue(value)
            , enabled(true)
        {}
    };

private:
    std::map<std::string, Watchpoint> watchpoints_;

public:
    /**
     * @brief 와치포인트 추가
     * @param variableName 변수 이름
     * @param env 현재 환경
     * @return 추가 성공 여부
     * @throws error::NameError if variable not found
     */
    bool add(const std::string& variableName, const Environment& env);

    /**
     * @brief 변수 값 변경 확인
     * @param env 현재 환경
     * @return 변경된 변수 목록
     */
    std::vector<std::string> checkChanges(const Environment& env);

    /**
     * @brief 와치포인트 제거
     * @param variableName 변수 이름
     * @return 제거 성공 여부
     */
    bool remove(const std::string& variableName);
};
```

---

## 3. 명령어 인터페이스

### 3.1 기본 명령어

| 명령어 | 약어 | 설명 | 예시 |
|--------|------|------|------|
| `break` | `b` | 브레이크포인트 설정 | `break test.ksj:10` |
| `delete` | `d` | 브레이크포인트 삭제 | `delete 1` |
| `run` | `r` | 프로그램 실행 | `run` |
| `continue` | `c` | 계속 실행 | `continue` |
| `step` | `s` | 한 줄 실행 (함수 내부 진입) | `step` |
| `next` | `n` | 한 줄 실행 (함수 건너뜀) | `next` |
| `print` | `p` | 변수 출력 | `print x` |
| `backtrace` | `bt` | 콜 스택 출력 | `backtrace` |
| `list` | `l` | 소스 코드 표시 | `list` |
| `quit` | `q` | 디버거 종료 | `quit` |

### 3.2 고급 명령어

| 명령어 | 설명 | 예시 |
|--------|------|------|
| `condition` | 조건부 브레이크포인트 | `condition 1 x > 10` |
| `watch` | 와치포인트 설정 | `watch x` |
| `info breakpoints` | 브레이크포인트 목록 | `info breakpoints` |
| `info locals` | 지역 변수 목록 | `info locals` |
| `set` | 변수 값 변경 | `set x = 20` |

### 3.3 명령어 파서

```cpp
/**
 * @class CommandParser
 * @brief 디버거 명령어 파서
 *
 * 사용자 입력을 파싱하여 명령어 객체로 변환합니다.
 */
class CommandParser {
public:
    /**
     * @enum CommandType
     * @brief 명령어 타입
     */
    enum class CommandType {
        BREAK,
        DELETE,
        RUN,
        CONTINUE,
        STEP,
        NEXT,
        PRINT,
        BACKTRACE,
        LIST,
        QUIT,
        CONDITION,
        WATCH,
        INFO,
        SET,
        UNKNOWN
    };

    /**
     * @struct Command
     * @brief 파싱된 명령어
     */
    struct Command {
        CommandType type;
        std::vector<std::string> args;
    };

    /**
     * @brief 명령어 파싱
     * @param input 사용자 입력
     * @return 파싱된 명령어
     */
    static Command parse(const std::string& input);
};
```

---

## 4. 구현 계획 (TDD)

### 4.1 Week 3-4: F5.2.1 기본 디버거

#### Day 1-2: 브레이크포인트 시스템

**테스트 먼저 작성** (tests/debugger/BreakpointManagerTest.cpp):
```cpp
TEST(BreakpointManagerTest, ShouldAddBreakpoint) {
    BreakpointManager mgr;
    SourceLocation loc("test.ksj", 10, 1);

    EXPECT_TRUE(mgr.add(loc));
    EXPECT_EQ(mgr.getAll().size(), 1);
}

TEST(BreakpointManagerTest, ShouldRemoveBreakpoint) {
    BreakpointManager mgr;
    SourceLocation loc("test.ksj", 10, 1);

    mgr.add(loc);
    EXPECT_TRUE(mgr.remove(loc));
    EXPECT_EQ(mgr.getAll().size(), 0);
}

TEST(BreakpointManagerTest, ShouldCheckBreakCondition) {
    BreakpointManager mgr;
    SourceLocation loc("test.ksj", 10, 1);
    Environment env;

    mgr.add(loc, "x > 10");
    env.set("x", Value::createInteger(15));

    EXPECT_TRUE(mgr.shouldBreak(loc, env));
}
```

**구현** (src/debugger/BreakpointManager.cpp):
- BreakpointManager 클래스
- add(), remove(), shouldBreak() 메서드
- 조건식 평가 (Evaluator 활용)

#### Day 3-4: CallStack

**테스트** (tests/debugger/CallStackTest.cpp):
```cpp
TEST(CallStackTest, ShouldPushAndPopFrame) {
    CallStack stack;
    auto env = std::make_shared<Environment>();
    SourceLocation loc("test.ksj", 10, 1);

    stack.push(CallStack::StackFrame("main", loc, env));
    EXPECT_EQ(stack.depth(), 1);

    stack.pop();
    EXPECT_TRUE(stack.empty());
}

TEST(CallStackTest, ShouldThrowOnEmptyPop) {
    CallStack stack;
    EXPECT_THROW(stack.pop(), std::runtime_error);
}
```

**구현** (src/debugger/CallStack.cpp)

#### Day 5-7: Debugger 메인 클래스

**테스트** (tests/debugger/DebuggerTest.cpp):
```cpp
TEST(DebuggerTest, ShouldSetBreakpointAndRun) {
    auto evaluator = std::make_shared<Evaluator>();
    Debugger debugger(evaluator);

    // 간단한 프로그램 실행
    std::string code = "x = 10\ny = 20\n출력(x + y)";
    // ... 파싱 ...

    debugger.setBreakpoint(SourceLocation("test.ksj", 2, 1));
    // ... 실행 및 검증 ...
}
```

**구현** (src/debugger/Debugger.cpp)

#### Day 8-10: 명령어 인터페이스

**테스트** (tests/debugger/CommandParserTest.cpp)
**구현** (src/debugger/CommandParser.cpp)

### 4.2 Week 5-6: F5.2.2 고급 디버거

#### Day 11-13: 조건부 브레이크포인트
#### Day 14-16: 와치포인트
#### Day 17-20: 소스 코드 표시 및 통합

---

## 5. 메모리 안전성 & 테스트

### 5.1 RAII 준수

```cpp
// ✅ GOOD - 스마트 포인터 사용
class Debugger {
    std::unique_ptr<BreakpointManager> breakpoints_;  // 자동 정리
    std::shared_ptr<Evaluator> evaluator_;            // 공유 ownership
};

// ❌ BAD - 원시 포인터
class BadDebugger {
    BreakpointManager* breakpoints_;  // 누가 delete?
};
```

### 5.2 예외 안전성

```cpp
// ✅ GOOD - 적절한 예외 처리
void Debugger::run(ast::Program* program) {
    if (!program) {
        throw std::invalid_argument("Debugger::run() - program is null");
    }

    try {
        evaluator_->eval(program);
    } catch (const error::KingSejongError& e) {
        // 에러 리포트
        reporter_->report(e);
        throw;  // 재전파
    }
}

// ❌ BAD - abort() 사용
void BadDebugger::run(ast::Program* program) {
    if (!program) {
        abort();  // 절대 안됨!
    }
}
```

### 5.3 테스트 전략

```bash
# 1. 단위 테스트 (각 클래스)
./bin/kingsejong_tests --gtest_filter="BreakpointManagerTest.*"
./bin/kingsejong_tests --gtest_filter="CallStackTest.*"
./bin/kingsejong_tests --gtest_filter="DebuggerTest.*"

# 2. 통합 테스트 (전체 디버거)
./bin/kingsejong_tests --gtest_filter="DebuggerIntegrationTest.*"

# 3. 메모리 안전성 (AddressSanitizer)
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -g" ..
make && ./bin/kingsejong_tests

# 4. 메모리 누수 (Valgrind)
valgrind --leak-check=full ./bin/kingsejong_tests
```

---

## 6. 성공 기준

### 6.1 기능 요구사항
- [ ] 모든 기본 명령어 동작 (10개)
- [ ] 브레이크포인트 설정/제거
- [ ] 단계별 실행 (step, next, continue)
- [ ] 변수 검사 (print)
- [ ] 콜 스택 출력 (backtrace)
- [ ] 조건부 브레이크포인트
- [ ] 와치포인트

### 6.2 품질 요구사항
- [ ] 30개+ 단위 테스트 통과
- [ ] 10개+ 통합 테스트 통과
- [ ] AddressSanitizer 클린
- [ ] Valgrind 메모리 누수 0건
- [ ] 모든 public API Doxygen 문서화
- [ ] 사용자 가이드 작성

### 6.3 성능 요구사항
- [ ] 브레이크포인트 체크 오버헤드 < 1%
- [ ] 명령어 응답 시간 < 100ms
- [ ] 메모리 사용량 증가 < 10MB

---

## 7. 향후 계획 (Phase 6+)

### 7.1 DAP (Debug Adapter Protocol) 지원
- VS Code 디버거 통합
- 그래픽 인터페이스

### 7.2 원격 디버깅
- 네트워크를 통한 원격 디버깅
- 프로세스 attach

### 7.3 타임 트래블 디버깅
- 실행 기록 저장
- 역방향 실행 (reverse step)

---

**작성자**: KingSejong Team
**버전**: 1.0
**다음 리뷰**: 기본 디버거 구현 완료 후 (Week 4)
