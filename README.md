# KingSejong Programming Language

<div align="center">

**한글로 프로그래밍하는 자연스러운 언어**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++23](https://img.shields.io/badge/C++-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![CI](https://github.com/kingsejong-lang/kingsejong-lang/workflows/CI/badge.svg)](https://github.com/kingsejong-lang/kingsejong-lang/actions)
[![Version](https://img.shields.io/badge/version-v0.6.0-blue)](https://github.com/kingsejong-lang/kingsejong-lang/releases)
[![VS Code](https://img.shields.io/badge/VS%20Code-Extension-blue)](vscode-extension/)

[빠른 시작](#빠른-시작) |
[문서](#-문서) |
[예제](examples/) |
[기여하기](#기여하기)

</div>

---

## 소개

**KingSejong**은 한국어의 문법적 특성을 활용한 프로그래밍 언어입니다. 조사 기반 메서드 체이닝, 함수형 프로그래밍, 250개의 표준 라이브러리 함수를 제공하며, 교육부터 실전까지 활용할 수 있습니다.

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
- **표준 라이브러리**: 250개 함수 (14개 모듈 - math, string, array, json, http, db, crypto 등)
- **개발 도구**: Linter (13개 규칙, `kingsejong lint`), Formatter (`kingsejong fmt`), Debugger (`--debug`), LSP (`--lsp`) + VS Code Extension
- **보안 모드**: `--sandbox`, `--untrusted`, `--allow-path` 로 파일·네트워크 접근 제한
- **바이트코드 VM**: `kingsejong --vm 파일.ksj` 로 같은 프로그램을 바이트코드로 컴파일해 실행한다. 값의 의미와 변수 모델을 평가기와 공유해 모든 예제가 같은 결과를 내며(ctest `examples-vm`), 함수 호출이 많은 코드에서 평가기보다 빠르다. 기본 실행기는 여전히 평가기다.

---

## 빠른 시작

### 설치

```bash
# 저장소 클론
git clone https://github.com/kingsejong-lang/kingsejong-lang.git
cd kingsejong-lang

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
# 전체 테스트 (단위 테스트와 예제 비교)
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

### 설정 파일 (.ksjlint.json)

```json
{
  "rules": {
    "unused-variable": "warning",
    "no-magic-number": "off",
    "josa-agreement": "warning"
  }
}
```

### 실행

```bash
# Lint 검사 (오류 등급 이슈가 있으면 종료 코드 1)
./bin/kingsejong lint script.ksj

# 포맷 (원본을 덮어쓴다. 포매터가 모르는 구문이 있으면 파일을 건드리지 않는다)
./bin/kingsejong fmt script.ksj
```

**13개 규칙**: `unused-variable`, `dead-code`, `no-self-comparison`, `constant-condition`, `empty-block`,
`no-magic-number`, `no-shadowing`, `no-unused-parameter`, `naming-convention`, `function-complexity`,
`unused-import`, `import-order`, `josa-agreement`. 자세한 내용: [docs/LINTER_RULES.md](docs/LINTER_RULES.md)

---

## 📚 문서

온라인 문서 사이트: https://kingsejong-lang.github.io/kingsejong-lang/docs/ (아래 문서를 검색·목차와 함께 본다)

| 문서 | 설명 |
|------|------|
| [튜토리얼](docs/TUTORIAL.md) | 단계별 학습 가이드 (초보자용) |
| [사용자 가이드](docs/USER_GUIDE.md) | 언어 기능 레퍼런스 |
| [언어 레퍼런스](docs/LANGUAGE_REFERENCE.md) | 완전한 문법 가이드 |
| [표준 라이브러리](docs/STDLIB_REFERENCE.md) | 250개 함수 레퍼런스 |
| [로드맵](docs/ROADMAP.md) | 프로젝트 계획 및 버전 히스토리 |
| [Linter 규칙](docs/LINTER_RULES.md) | 13개 규칙 설명 및 설정 |
| [디버거 가이드](docs/DEBUGGER_GUIDE.md) | 디버거 사용법 |
| [VS Code Extension](vscode-extension/README.md) | Extension 가이드 |
| [설치 가이드](docs/INSTALL.md) | 상세 설치 방법 (OS별) |
| [기여 가이드](docs/CONTRIBUTING.md) | 기여 방법 및 개발 가이드 |

### 설계 문서

- [프로젝트 개요](docs/design/PROJECT_OVERVIEW.md) - 목표와 전체 구조
- [설계 철학](docs/design/LANGUAGE_DESIGN_PHILOSOPHY.md) - 한글 문법을 택한 이유
- [기능 명세](docs/design/FEATURES.md) - 언어 기능 명세
- [구현 상태](docs/design/IMPLEMENTATION.md) - 구현된 것과 남은 것
- [메모리 풀링 설계](docs/design/MEMORY_POOLING_DESIGN.md) - 값 객체 메모리 전략
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

[GitHub](https://github.com/kingsejong-lang/kingsejong-lang) •
[릴리스](https://github.com/kingsejong-lang/kingsejong-lang/releases) •
[이슈 제기](https://github.com/kingsejong-lang/kingsejong-lang/issues) •
[튜토리얼](docs/TUTORIAL.md)

</div>
