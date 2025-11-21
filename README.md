# KingSejong Programming Language

<div align="center">

**한글로 프로그래밍하는 자연스러운 언어**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++23](https://img.shields.io/badge/C++-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![CI](https://github.com/0xmhha/kingsejonglang/workflows/CI/badge.svg)](https://github.com/0xmhha/kingsejonglang/actions)
[![Tests](https://img.shields.io/badge/tests-1506%20passed-success)](tests/)
[![Version](https://img.shields.io/badge/version-v0.5.0-blue)](https://github.com/0xmhha/kingsejonglang/releases/tag/v0.5.0)
[![VS Code](https://img.shields.io/badge/VS%20Code-Extension-blue)](vscode-extension/)

[빠른 시작](#빠른-시작) |
[주요 특징](#주요-특징) |
[튜토리얼](docs/TUTORIAL.md) |
[문서](docs/LANGUAGE_REFERENCE.md) |
[예제](examples/)

</div>

---

## 소개

**KingSejong**은 한국어의 문법적 특성을 활용한 혁신적인 프로그래밍 언어입니다.
한글 **조사 기반 메서드 체이닝**, **함수형 프로그래밍**, **모듈 시스템**을 통해
직관적이고 표현력 있는 코드 작성을 지원합니다.

```ksj
# 소수 찾기 - 함수형 프로그래밍의 아름다움
가져오기 "stdlib/math"
가져오기 "stdlib/array"

숫자들 = 범위(1, 50)
소수들 = 숫자들을 걸러낸다(소수인가)
출력(소수들)  # [2, 3, 5, 7, 11, 13, ...]
```

---

## 빠른 시작

### 설치

KingSejong은 macOS, Linux, Windows를 지원합니다.

#### 사전 요구사항

- **C++23 지원 컴파일러** (GCC 12+, Clang 15+, MSVC 2022+)
- **CMake 3.20+**
- **Git**

#### macOS

```bash
# Homebrew로 의존성 설치
brew install cmake

# 저장소 클론 및 빌드
git clone https://github.com/0xmhha/kingsejonglang.git
cd kingsejonglang
mkdir build && cd build
cmake ..
make -j4

# 실행
./bin/kingsejong ../examples/hello.ksj
```

#### Linux (Ubuntu/Debian)

```bash
# 의존성 설치
sudo apt update
sudo apt install -y build-essential cmake git

# 저장소 클론 및 빌드
git clone https://github.com/0xmhha/kingsejonglang.git
cd kingsejonglang
mkdir build && cd build
cmake ..
make -j4

# 실행
./bin/kingsejong ../examples/hello.ksj
```

#### Linux (Fedora/CentOS)

```bash
# 의존성 설치
sudo dnf install -y gcc-c++ cmake git

# 저장소 클론 및 빌드
git clone https://github.com/0xmhha/kingsejonglang.git
cd kingsejonglang
mkdir build && cd build
cmake ..
make -j4

# 실행
./bin/kingsejong ../examples/hello.ksj
```

#### Windows

**Visual Studio 2022 사용:**

```powershell
# 저장소 클론
git clone https://github.com/0xmhha/kingsejonglang.git
cd kingsejonglang

# 빌드 디렉토리 생성
mkdir build
cd build

# CMake 설정 (Visual Studio 2022)
cmake .. -G "Visual Studio 17 2022"

# 빌드
cmake --build . --config Release

# 실행
.\bin\Release\kingsejong.exe ..\examples\hello.ksj
```

> **자세한 설치 가이드**: [docs/INSTALL.md](docs/INSTALL.md)

### Hello, World!

**hello.ksj**:

```ksj
출력("안녕, KingSejong!")
```

실행:

```bash
./kingsejong hello.ksj
```

출력:

```
안녕, KingSejong!
```

### 대화형 실행 (REPL)

```bash
$ ./kingsejong

>>> 출력("안녕하세요!")
안녕하세요!

>>> x = 10
>>> y = 20
>>> 출력(x + y)
30

>>> .exit
```

---

## 주요 특징

### 1. 조사 기반 메서드 체이닝

한국어의 조사(`을/를`, `이/가`, `의` 등)를 활용한 자연스러운 메서드 호출:

```ksj
# 영어: array.sort().reverse()
# KingSejong:
배열을 정렬한다
정렬된배열을 역순으로_나열한다
```

**받침 자동 인식:**

```ksj
배열을 정렬한다    # 받침 있음 → '을'
데이터를 처리한다  # 받침 없음 → '를'
```

### 2. 함수형 프로그래밍

고차 함수와 함수형 메서드 지원:

```ksj
# Filter + Map + Reduce
숫자들 = [1, 2, 3, 4, 5, 6]

짝수들 = 숫자들을 걸러낸다(함수(x) { 반환 x % 2 == 0 })
제곱들 = 짝수들을 변환한다(함수(x) { 반환 x * x })
합계 = 제곱들을 축약한다(0, 함수(누적, 현재) { 반환 누적 + 현재 })

출력(합계)  # 56 (2² + 4² + 6² = 4 + 16 + 36)
```

**지원하는 함수형 메서드:**

- `걸러낸다(함수)` - filter
- `변환한다(함수)` - map
- `축약한다(초기값, 함수)` - reduce
- `찾다(함수)` - find

### 3. 모듈 시스템

표준 라이브러리와 사용자 정의 모듈:

```ksj
# 표준 라이브러리 사용
가져오기 "stdlib/math"
가져오기 "stdlib/array"

# 수학 함수
출력(거듭제곱(2, 10))    # 1024
출력(계승(5))            # 120
출력(제곱근(16))         # 4.0

# 배열 유틸리티
숫자들 = 범위(1, 10)
중복제거 = 중복_제거([1, 2, 2, 3, 3])
```

**표준 라이브러리 (stdlib) - 총 251개 함수:**

- `math.ksj` - 18개 수학 함수 (절댓값, 거듭제곱, 팩토리얼, GCD, 소수 판별 등)
- `string.ksj` - 9개 문자열 처리 함수 (반복, 결합, 채우기 등)
- `array.ksj` - 18개 배열 유틸리티 (범위, 중복 제거, zip 등)
- `io.ksj` - 25개 파일 I/O 함수 (파일 읽기/쓰기, 디렉토리 관리, 경로 처리 등)
- `json.ksj` - 17개 JSON 처리 함수 (파싱, 문자열화, 파일 I/O, object 조작 등)
- `time.ksj` - 14개 시간/날짜 함수 (타임스탬프, 포맷팅, 날짜 연산, 슬립 등)
- `utils.ksj` - 30개 유틸리티 함수
- `regex.ksj` - 30개 정규표현식 함수
- `crypto.ksj` - 42개 암호화 함수
- `os.ksj` - 39개 OS & 파일 시스템 함수
- `http.ksj` - 20개 HTTP 클라이언트 함수
- `db.ksj` - 15개 데이터베이스 함수 (SQLite)
- `collections.ksj` - 50개 자료구조 함수 (Set, Map, Queue, Stack, Deque)
- `test.ksj` - 5개 테스트 프레임워크 함수 (assert 계열)

### 4. 자연스러운 범위 표현

수학적으로 직관적인 범위 표현:

```ksj
i가 1부터 10까지 {      # [1, 10] - 10 포함
    출력(i)
}

i가 0부터 5 미만 {      # [0, 5) - 5 미포함
    출력(i)
}

i가 1 초과 10 이하 {    # (1, 10] - 1 미포함, 10 포함
    출력(i)
}
```

### 5. 직관적인 타입 시스템

한글로 타입을 명시:

```ksj
정수 나이 = 25
실수 키 = 175.5
문자열 이름 = "홍길동"
논리 합격 = 참

# 타입 추론도 가능
점수 = 95          # 자동으로 정수로 추론
메시지 = "안녕"    # 자동으로 문자열로 추론
```

### 6. 1급 함수와 클로저

함수를 값처럼 다루기:

```ksj
# 함수 리터럴
더하기 = 함수(a, b) {
    반환 a + b
}

# 클로저
카운터_생성 = 함수() {
    개수 = 0
    반환 함수() {
        개수 = 개수 + 1
        반환 개수
    }
}

카운터 = 카운터_생성()
출력(카운터())  # 1
출력(카운터())  # 2

# 재귀
피보나치 = 함수(n) {
    만약 (n <= 1) { 반환 n }
    반환 피보나치(n-1) + 피보나치(n-2)
}
```

### 7. 편리한 반복문

다양한 반복 구문:

```ksj
# N번 반복
5번 반복한다 {
    출력("Hello!")
}

# 범위 반복
i가 1부터 10까지 {
    출력(i)
}

# 변수 사용
횟수 = 10
횟수번 반복한다 {
    출력("반복")
}
```

### 8. 가비지 컬렉터

자동 메모리 관리로 메모리 누수 방지:

```cpp
// Mark & Sweep 알고리즘
// 순환 참조 자동 감지
// 개발자는 메모리 관리 불필요
```

**특징:**

- Mark & Sweep 알고리즘
- 순환 참조 감지 및 해제
- 루트 집합 관리
- 자동/수동 GC 모드
- 메모리 통계 제공

### 9. 바이트코드 컴파일러

고성능 실행을 위한 바이트코드 VM:

```cpp
// AST → Bytecode 컴파일
// 스택 기반 VM 실행
// Tree-walking 대비 성능 향상
```

**구성:**

- 60+ OpCode 명령어 세트
- 스택 기반 가상 머신
- 상수 풀 및 이름 테이블
- 디스어셈블러 (디버깅)
- 런타임 에러 처리

---

## 문서

| 문서                                              | 설명                           | 대상           |
| ------------------------------------------------- | ------------------------------ | -------------- |
| [튜토리얼](docs/TUTORIAL.md)                      | 단계별 학습 가이드 (10개 챕터) | 초보자         |
| [언어 레퍼런스](docs/LANGUAGE_REFERENCE.md)       | 완전한 문법 가이드             | 모든 사용자    |
| [라이브러리 가이드](docs/LIBRARY_GUIDE.md)        | 모듈 개발 가이드               | 개발자         |
| [표준 라이브러리 레퍼런스](docs/STDLIB_REFERENCE.md) | 251개 stdlib 함수 완전 레퍼런스 | 모든 사용자    |
| [디버거 가이드](docs/DEBUGGER_GUIDE.md)           | 디버거 사용법 및 실전 예제     | 모든 사용자    |
| [Linter 규칙 가이드](docs/LINTER_RULES.md)        | 13개 Linter 규칙 설명 및 설정  | 개발자         |
| [플레이그라운드 튜토리얼](docs/PLAYGROUND_TUTORIAL.md) | 웹 플레이그라운드 개발 가이드 | 개발자         |
| [GC 설계 문서](docs/GC_DESIGN.md)                 | 가비지 컬렉터 설계 및 구현     | 고급 개발자    |
| [메모리 풀링 분석](docs/MEMORY_POOLING_DESIGN.md) | ObjectPool 설계 및 벤치마크    | 고급 개발자    |
| [VS Code Extension](vscode-extension/README.md)   | Extension 설치 및 사용법       | VS Code 사용자 |
| [작업 목록](docs/TODOLIST.md)                     | 프로젝트 진행 상황             | 기여자         |

---

## 예제 프로그램

**21개의 예제 프로그램** 제공:

### 기초 예제

- `hello.ksj` - Hello World
- `calculator.ksj` - 산술 연산
- `type_demo.ksj` - 타입 시스템

### 제어 구조

- `conditional.ksj` - 조건문
- `loop.ksj` - 반복문
- `nested_loops.ksj` - 중첩 반복문

### 함수

- `function.ksj` - 1급 함수
- `closure.ksj` - 클로저
- `fibonacci.ksj`, `recursion.ksj` - 재귀

### 배열

- `array_methods.ksj` - 배열 메서드
- `functional_array.ksj` - 함수형 메서드

### 표준 라이브러리

- `stdlib_math.ksj` - 수학 함수 활용
- `stdlib_string.ksj` - 문자열 처리
- `stdlib_array.ksj` - 배열 유틸리티

### 고급 예제

- `prime_finder.ksj` - 소수 찾기 (에라토스테네스의 체)
- `sorting_demo.ksj` - 정렬 알고리즘 (버블 정렬)
- `statistics.ksj` - 통계 분석 (분산, 표준편차, 이상치)
- `text_rpg.ksj` - 텍스트 RPG 게임 (던전 탐험)

```bash
# 예제 실행
./kingsejong examples/prime_finder.ksj
./kingsejong examples/text_rpg.ksj
./kingsejong examples/statistics.ksj
```

---

## 빌드 및 설치

### 요구사항

- **C++23** 호환 컴파일러
  - GCC 13+
  - Clang 16+
  - MSVC 2022+
- **CMake** 3.20+
- **GoogleTest** (테스트용, 자동 다운로드)

### 빌드

```bash
# 1. 저장소 클론
git clone https://github.com/0xmhha/kingsejonglang.git
cd kingsejonglang

# 2. 빌드 디렉토리 생성
mkdir build && cd build

# 3. CMake 설정
cmake ..

# 4. 빌드 (병렬 빌드)
make -j4

# 5. 테스트 실행
ctest --output-on-failure
```

### 실행

#### REPL 모드

```bash
./kingsejong
```

**REPL 명령어:**

- `.exit`, `.quit` - 종료
- `.help` - 도움말
- `.clear` - 변수 초기화
- `.vars` - 변수 목록
- `Ctrl+D` - 종료

#### 파일 실행

```bash
./kingsejong script.ksj
```

#### LSP 서버 모드

```bash
./kingsejong --lsp
```

LSP 서버는 stdin/stdout을 통해 JSON-RPC 2.0 메시지를 주고받습니다.

### VS Code Extension 사용

```bash
# Extension 디렉토리로 이동
cd vscode-extension

# 의존성 설치
npm install

# TypeScript 컴파일
npm run compile

# VS Code에서 실행
# 1. vscode-extension 폴더를 VS Code로 열기
# 2. F5를 눌러 Extension Development Host 실행
# 3. .ksj 파일을 열어서 테스트
```

**제공하는 기능:**

**기본 기능:**

- 구문 강조 (Syntax Highlighting)
- 자동 완성 (Auto Completion) - 34개 키워드, 변수명
- 실시간 진단 (Diagnostics) - 구문 오류 표시
- 자동 괄호 닫기
- 들여쓰기 자동 조정

**고급 기능:**

- 정의로 이동 (Go to Definition) - F12
- 호버 정보 (Hover Information) - 타입/시그니처 표시
- 참조 찾기 (Find References) - Shift+F12
- 심볼 이름 변경 (Rename) - F2
- 스코프 인식 - 전역/로컬 변수 구분
- 한글 문자 지원 - UTF-16/UTF-8 변환

---

## 왜 KingSejong인가?

### 교육적 가치

- **한글로 프로그래밍** - 영어 장벽 없이 학습
- **직관적 문법** - 자연스러운 한국어 표현
- **풍부한 예제** - 21개 실전 프로그램
- **완전한 문서** - 초보자부터 고급까지

### 기술적 혁신

- **조사 기반 메서드 체이닝** - 세계 최초
- **함수형 프로그래밍** - filter, map, reduce
- **모듈 시스템** - 재사용 가능한 라이브러리
- **C++23 구현** - 모던 C++ 활용

### 실용성

- **표준 라이브러리** - 251개 함수 (14개 모듈)
- **디버거** - 브레이크포인트, 단계 실행, 변수 검사
- **Linter** - 13개 코드 품질 규칙 (.ksjlintrc 설정)
- **온라인 플레이그라운드** - 즉시 실행 (WebAssembly)
- **REPL** - 빠른 프로토타이핑
- **에러 메시지 한글화** - 친절한 피드백
- **100% 테스트 통과** - 안정성 보장 (1,506개)

---

## 학습 경로

1. **[튜토리얼 시작](docs/TUTORIAL.md)** - Hello World부터 차근차근
2. **[예제 실행](examples/)** - 21개 예제로 실습
3. **[레퍼런스 참고](docs/LANGUAGE_REFERENCE.md)** - 문법 깊이 이해
4. **[모듈 만들기](docs/LIBRARY_GUIDE.md)** - 나만의 라이브러리 작성

---

## 기여하기

기여를 환영합니다!

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'feat: Add AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

자세한 가이드: [CONTRIBUTING.md](docs/CONTRIBUTING.md)

---

## 프로젝트 구조

```
kingsejonglang/
├── src/                    # 소스 코드
│   ├── lexer/             # 어휘 분석 (Token, JosaRecognizer)
│   ├── parser/            # 구문 분석 (Pratt Parsing)
│   ├── ast/               # 추상 구문 트리
│   ├── evaluator/         # 평가기 (Value, Environment)
│   ├── types/             # 타입 시스템
│   ├── error/             # 에러 처리
│   ├── memory/            # 메모리 관리
│   │   ├── ObjectPool.h         # 객체 풀 템플릿
│   │   └── ValuePool.h          # Value 풀 싱글톤
│   ├── module/            # 모듈 시스템
│   ├── repl/              # 대화형 실행
│   ├── lsp/               # Language Server Protocol
│   │   ├── JsonRpc.cpp         # JSON-RPC 2.0 통신
│   │   ├── DocumentManager.cpp # 문서 관리
│   │   ├── LanguageServer.cpp  # LSP 서버 (정의 이동, 호버, 참조 찾기, 이름 변경)
│   │   ├── CompletionProvider.cpp  # 자동 완성
│   │   ├── DiagnosticsProvider.cpp # 실시간 진단
│   │   ├── SymbolTable.cpp     # 심볼 저장 및 조회
│   │   ├── SymbolCollector.cpp # AST 기반 심볼 수집
│   │   └── LspUtils.cpp        # UTF-16/UTF-8 변환 유틸리티
│   ├── debugger/          # 디버거
│   │   ├── Debugger.cpp        # 디버거 엔진
│   │   ├── BreakpointManager.cpp # 브레이크포인트 관리
│   │   ├── WatchpointManager.cpp # 와치포인트 관리
│   │   ├── CallStack.cpp       # 콜스택 관리
│   │   ├── CommandParser.cpp   # 디버거 명령 파싱
│   │   └── SourceCodeViewer.cpp # 소스 코드 표시
│   ├── jit/               # JIT 컴파일러
│   │   ├── JITCompilerT1.cpp    # Tier-1 JIT 컴파일러
│   │   └── HotPathDetector.cpp  # 핫스팟 감지
│   ├── linter/            # 정적 분석기
│   │   ├── Linter.cpp           # Linter 엔진
│   │   └── rules/               # 13개 규칙
│   └── main.cpp           # 진입점 (REPL/파일실행/LSP)
├── vscode-extension/      # VS Code Extension
│   ├── src/
│   │   └── extension.ts   # LSP 클라이언트
│   ├── syntaxes/
│   │   └── kingsejong.tmLanguage.json  # 구문 강조
│   ├── package.json       # Extension 메타데이터
│   └── README.md          # Extension 가이드
├── stdlib/                # 표준 라이브러리 (251개 함수)
│   ├── math.ksj          # 수학 함수 (18개)
│   ├── string.ksj        # 문자열 처리 (9개)
│   ├── array.ksj         # 배열 유틸리티 (18개)
│   ├── collections.ksj   # 자료구조 (50개)
│   ├── http.ksj          # HTTP 클라이언트 (20개)
│   ├── db.ksj            # 데이터베이스 (15개)
│   ├── test.ksj          # 테스트 프레임워크 (5개)
│   └── ... (14개 모듈)
├── tests/                 # 테스트 코드 (1,506개)
│   ├── lsp/              # LSP 테스트 (130개)
│   │   ├── JsonRpcTest.cpp
│   │   ├── DocumentManagerTest.cpp
│   │   ├── LanguageServerTest.cpp
│   │   ├── CompletionProviderTest.cpp
│   │   ├── DiagnosticsProviderTest.cpp
│   │   ├── SymbolTableTest.cpp
│   │   ├── SymbolCollectorTest.cpp
│   │   ├── LspUtilsTest.cpp
│   │   ├── SymbolTableIntegrationTest.cpp
│   │   ├── GoToDefinitionTest.cpp
│   │   ├── HoverTest.cpp
│   │   ├── FindReferencesTest.cpp
│   │   └── RenameTest.cpp
│   ├── debugger/         # 디버거 테스트 (179개)
│   │   ├── BreakpointManagerTest.cpp
│   │   ├── WatchpointManagerTest.cpp
│   │   ├── DebuggerTest.cpp
│   │   ├── CallStackTest.cpp
│   │   ├── CommandParserTest.cpp
│   │   ├── SourceCodeViewerTest.cpp
│   │   └── DebuggerReplTest.cpp
│   ├── memory/           # 메모리 관리 테스트 (16개)
│   │   ├── ObjectPoolTest.cpp
│   │   └── ValuePoolTest.cpp
├── examples/              # 예제 프로그램 (21개)
├── docs/                  # 문서
│   ├── TUTORIAL.md               # 초보자 튜토리얼
│   ├── LANGUAGE_REFERENCE.md     # 언어 레퍼런스
│   ├── LIBRARY_GUIDE.md          # 라이브러리 가이드
│   ├── STDLIB_REFERENCE.md       # 표준 라이브러리 레퍼런스 (251개 함수)
│   ├── DEBUGGER_GUIDE.md         # 디버거 사용 가이드
│   ├── LINTER_RULES.md           # Linter 규칙 가이드 (13개)
│   ├── PLAYGROUND_TUTORIAL.md    # 플레이그라운드 개발 튜토리얼
│   ├── GC_DESIGN.md              # GC 설계 문서
│   ├── MEMORY_POOLING_DESIGN.md  # 메모리 풀링 분석
│   ├── TODOLIST.md               # 작업 목록
│   └── ROADMAP.md                # 개발 로드맵
├── .ksjlintrc            # Linter 설정 파일 (13개 규칙)
├── CMakeLists.txt        # 빌드 설정
├── LICENSE               # MIT 라이센스
└── README.md             # 이 파일
```

---

## 라이센스

이 프로젝트는 **MIT 라이센스** 하에 배포됩니다.
자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.

---

## 🙏 감사의 말

이 프로젝트는 한국어의 아름다움과 프로그래밍의 논리성을 결합하려는 시도입니다.

**세종대왕**의 창제 정신을 이어받아,
**누구나 쉽게 프로그래밍**할 수 있는 언어를 만들고자 합니다.

---

<div align="center">

[GitHub](https://github.com/0xmhha/kingsejonglang) •
[이슈 제기](https://github.com/0xmhha/kingsejonglang/issues) •
[튜토리얼](docs/TUTORIAL.md)

</div>
