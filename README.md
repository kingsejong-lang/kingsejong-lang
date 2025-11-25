# KingSejong Programming Language

<div align="center">

**한글로 프로그래밍하는 자연스러운 언어**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++23](https://img.shields.io/badge/C++-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![CI](https://github.com/0xmhha/kingsejonglang/workflows/CI/badge.svg)](https://github.com/0xmhha/kingsejonglang/actions)
[![Tests](https://img.shields.io/badge/tests-1506%20passed-success)](tests/)
[![Version](https://img.shields.io/badge/version-v0.6.0--rc-blue)](https://github.com/0xmhha/kingsejonglang/releases)
[![VS Code](https://img.shields.io/badge/VS%20Code-Extension-blue)](vscode-extension/)

[빠른 시작](#빠른-시작) |
[문서](#-문서) |
[예제](examples/) |
[기여하기](#기여하기)

</div>

---

## 소개

**KingSejong**은 한국어의 문법적 특성을 활용한 프로그래밍 언어입니다. 조사 기반 메서드 체이닝, 함수형 프로그래밍, 251개의 표준 라이브러리 함수를 제공하며, 교육부터 실전까지 활용할 수 있습니다.

```ksj
# 소수 찾기 예제
가져오기 "stdlib/math"
가져오기 "stdlib/array"

숫자들 = 범위(1, 50)
소수들 = 숫자들을 걸러낸다(소수인가)
출력(소수들)  # [2, 3, 5, 7, 11, 13, ...]
```

### 주요 특징

- **조사 기반 메서드 체이닝**: 받침 자동 인식 (배열`을`, 데이터`를`)
- **함수형 프로그래밍**: filter, map, reduce 지원
- **표준 라이브러리**: 251개 함수 (14개 모듈 - math, string, array, json, http, db, crypto 등)
- **JIT 컴파일러**: Tier 1 + Tier 2 함수 인라이닝
- **개발 도구**: Linter (13개 규칙), Formatter, Debugger, VS Code Extension

---

## 빠른 시작

### 설치

```bash
# 저장소 클론
git clone https://github.com/0xmhha/kingsejonglang.git
cd kingsejonglang

# 빌드
mkdir build && cd build
cmake ..
make -j4

# 실행
./bin/kingsejong ../examples/hello.ksj
```

**요구사항**: C++23 컴파일러 (GCC 13+, Clang 16+, MSVC 2022+), CMake 3.20+

### Hello, World!

**hello.ksj**:
```ksj
출력("안녕, KingSejong!")
```

실행:
```bash
./bin/kingsejong hello.ksj
```

### REPL 모드

```bash
$ ./bin/kingsejong

>>> 출력("안녕하세요!")
안녕하세요!

>>> x = 10
>>> 출력(x * 2)
20
```

명령어: `.exit` (종료), `.help` (도움말), `.vars` (변수 목록)

---

## 빌드 및 테스트

### 빌드

```bash
mkdir build && cd build
cmake ..
make -j4
```

### 테스트 실행

```bash
# 전체 테스트 (1,506개)
ctest --output-on-failure

# 특정 테스트
./bin/kingsejong_tests --gtest_filter="LexerTest.*"
```

### 메모리 검사

```bash
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -g" ..
make && ctest
```

---

## Linter 사용

### 설정 파일 (.ksjlintrc)

```json
{
  "rules": {
    "no-unused-variables": "warn",
    "no-undefined-variables": "error",
    "no-reassignment": "off"
  }
}
```

### 실행

```bash
# Lint 검사
./bin/ksjlint script.ksj

# 자동 수정
./bin/ksjfmt script.ksj --write
```

**13개 규칙**: `no-unused-variables`, `no-undefined-variables`, `max-function-length`, `prefer-const` 등
자세한 내용: [docs/LINTER_RULES.md](docs/LINTER_RULES.md)

---

## 📚 문서

| 문서 | 설명 |
|------|------|
| [튜토리얼](docs/TUTORIAL.md) | 단계별 학습 가이드 (초보자용) |
| [사용자 가이드](docs/USER_GUIDE.md) | 언어 기능 레퍼런스 |
| [언어 레퍼런스](docs/LANGUAGE_REFERENCE.md) | 완전한 문법 가이드 |
| [표준 라이브러리](docs/STDLIB_REFERENCE.md) | 251개 함수 레퍼런스 |
| [로드맵](docs/ROADMAP.md) | 프로젝트 계획 및 버전 히스토리 |
| [Linter 규칙](docs/LINTER_RULES.md) | 13개 규칙 설명 및 설정 |
| [디버거 가이드](docs/DEBUGGER_GUIDE.md) | 디버거 사용법 |
| [VS Code Extension](vscode-extension/README.md) | Extension 가이드 |
| [설치 가이드](docs/INSTALL.md) | 상세 설치 방법 (OS별) |
| [기여 가이드](docs/CONTRIBUTING.md) | 기여 방법 및 개발 가이드 |

### 아키텍처 문서

- [아키텍처 분석](docs/ARCHITECTURE_ANALYSIS.md) - 시스템 구조
- [VM 분석](docs/KINGSEJONG_VM_ANALYSIS.md) - 가상 머신 상세
- [언어 분석](docs/KINGSEJONG_LANGUAGE_ANALYSIS.md) - 언어 스펙
- [성능 분석](docs/PERFORMANCE_ANALYSIS.md) - 벤치마크 및 최적화
- [GC 설계](docs/GC_DESIGN.md) - 가비지 컬렉터 설계
- [Phase 9 리팩토링](docs/PHASE9_REFACTORING_PLAN.md) - 코드 품질 개선

---

## 예제

**21개의 예제** 제공:

- **기초**: `hello.ksj`, `calculator.ksj`, `type_demo.ksj`
- **제어 구조**: `conditional.ksj`, `loop.ksj`, `nested_loops.ksj`
- **함수**: `function.ksj`, `closure.ksj`, `fibonacci.ksj`
- **배열**: `array_methods.ksj`, `functional_array.ksj`
- **stdlib**: `stdlib_math.ksj`, `stdlib_string.ksj`, `stdlib_array.ksj`
- **고급**: `prime_finder.ksj`, `sorting_demo.ksj`, `text_rpg.ksj`, `statistics.ksj`

```bash
# 예제 실행
./bin/kingsejong examples/prime_finder.ksj
./bin/kingsejong examples/text_rpg.ksj
```

전체 예제: [examples/](examples/)

---

## 기여하기

기여를 환영합니다!

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'feat: Add AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

자세한 가이드: [CONTRIBUTING.md](docs/CONTRIBUTING.md)

### 커밋 규칙

```
feat: 새 기능 추가
fix: 버그 수정
refactor: 코드 리팩토링
test: 테스트 추가
docs: 문서 수정
perf: 성능 개선
```

---

## 라이센스

이 프로젝트는 **MIT 라이센스** 하에 배포됩니다. [LICENSE](LICENSE) 참조.

---

## 🙏 감사의 말

**세종대왕**의 창제 정신을 이어받아, **누구나 쉽게 프로그래밍**할 수 있는 언어를 만들고자 합니다.

---

<div align="center">

[GitHub](https://github.com/0xmhha/kingsejonglang) •
[릴리스](https://github.com/0xmhha/kingsejonglang/releases) •
[이슈 제기](https://github.com/0xmhha/kingsejonglang/issues) •
[튜토리얼](docs/TUTORIAL.md)

</div>
