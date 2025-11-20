# KingSejong 프로젝트 개발 가이드

KingSejong 프로그래밍 언어 프로젝트 개발 및 기여를 위한 가이드입니다.

---

## 프로젝트 현황

### 현재 상태 (v0.5.0)

**프로젝트명**: KingSejong
**파일 확장자**: `.ksj`
**저장소**: https://github.com/0xmhha/kingsejonglang
**현재 버전**: v0.5.0
**Phase 완료**: Phase 0-8 (✅)
**테스트**: 1,503개 (100% 통과)

### 구현 완료된 주요 기능

#### Phase 1: 핵심 차별화 기능 ✅
- ✅ 조사 기반 메서드 체인 (JosaExpression)
- ✅ 자연스러운 범위 표현 (RangeExpression: `1부터 10까지`)
- ✅ 명확한 타입 시스템 (정수, 실수, 문자열, 참거짓)
- ✅ 직관적 반복문 (`10번 반복한다`, `i가 1부터 10까지`)

#### Phase 2-7: 고급 기능 ✅
- ✅ 클래스 및 상속 (ClassStatement)
- ✅ Async/Await (AsyncFunctionLiteral, AwaitExpression)
- ✅ Try/Catch/Finally/Throw
- ✅ 패턴 매칭 (MatchExpression)
- ✅ 모듈 시스템 (ModuleLoader)
- ✅ 바이트코드 VM (56개 OpCode)
- ✅ JIT Compiler Tier 1 (asmjit 사용)
- ✅ 세대별 GC (Young/Old Generation)
- ✅ LSP (Language Server Protocol)
- ✅ 디버거 (Debugger)
- ✅ ksjpm 패키지 관리자
- ✅ Linter & Formatter
- ✅ VS Code 확장

#### Phase 8: 코드 품질 및 문서화 ✅
- ✅ Doxygen 설정
- ✅ USER_GUIDE.md
- ✅ regression_test.py
- ✅ 1,503개 테스트 (100% 통과)

### 표준 라이브러리 (14개 모듈, 182개 함수)

✅ 구현 완료:
- `array.ksj` (18개 함수)
- `collections.ksj` (50개 함수) - Set, Map, Queue, Stack, Deque
- `crypto.ksj` (42개 함수)
- `db.ksj` (15개 함수)
- `http.ksj` (20개 함수)
- `io.ksj` (25개 함수)
- `json.ksj` (17개 함수)
- `math.ksj` (18개 함수)
- `os.ksj` (39개 함수)
- `regex.ksj` (30개 함수)
- `string.ksj` (9개 함수)
- `test.ksj` (5개 함수)
- `time.ksj` (14개 함수)
- `utils.ksj` (30개 함수)

---

## 저장소 클론 및 빌드

### 1. 저장소 클론
```bash
git clone https://github.com/0xmhha/kingsejonglang.git
cd kingsejonglang
```

### 2. 빌드
```bash
# 빌드 디렉토리 생성
mkdir build && cd build

# CMake 설정
cmake ..

# 빌드 (병렬 빌드)
make -j4

# 실행
./bin/kingsejong --version
./bin/kingsejong ../examples/hello.ksj
```

### 3. 테스트 실행
```bash
# 모든 테스트 실행
ctest --output-on-failure

# 회귀 테스트 실행
python3 ../scripts/regression_test.py
```

---

## 프로젝트 구조

```
kingsejonglang/
├── LICENSE                        ✅ MIT License
├── README.md                      ✅ 프로젝트 소개
├── CMakeLists.txt                 ✅ 빌드 설정
├── .gitignore                     ✅ Git 제외
│
├── src/
│   ├── main.cpp                   ✅ 진입점
│   ├── lexer/                     ✅ 어휘 분석
│   ├── parser/                    ✅ 구문 분석
│   ├── ast/                       ✅ 추상 구문 트리
│   ├── evaluator/                 ✅ 실행 엔진
│   ├── types/                     ✅ 타입 시스템
│   ├── error/                     ✅ 에러 처리
│   ├── module/                    ✅ 모듈 시스템
│   ├── repl/                      ✅ 대화형 실행
│   ├── lsp/                       ✅ Language Server Protocol
│   ├── debugger/                  ✅ 디버거
│   ├── jit/                       ✅ JIT 컴파일러
│   └── linter/                    ✅ 정적 분석기
│
├── stdlib/                        ✅ 표준 라이브러리 (14개 모듈, 182개 함수)
│   ├── array.ksj
│   ├── collections.ksj
│   ├── crypto.ksj
│   ├── db.ksj
│   ├── http.ksj
│   ├── io.ksj
│   ├── json.ksj
│   ├── math.ksj
│   ├── os.ksj
│   ├── regex.ksj
│   ├── string.ksj
│   ├── test.ksj
│   ├── time.ksj
│   └── utils.ksj
│
├── tests/                         ✅ 테스트 (1,503개, 100% 통과)
│   ├── lexer/
│   ├── parser/
│   ├── evaluator/
│   ├── lsp/                       ✅ LSP 테스트 (130개)
│   ├── debugger/                  ✅ 디버거 테스트 (179개)
│   └── ...
│
├── examples/                      ✅ 예제 프로그램 (21개)
│   ├── hello.ksj
│   ├── calculator.ksj
│   ├── prime_finder.ksj
│   ├── text_rpg.ksj
│   └── ...
│
├── vscode-extension/              ✅ VS Code 확장
│   ├── src/extension.ts
│   ├── syntaxes/
│   └── package.json
│
├── docs/                          ✅ 문서
│   ├── TUTORIAL.md
│   ├── LANGUAGE_REFERENCE.md
│   ├── LIBRARY_GUIDE.md
│   ├── USER_GUIDE.md
│   └── ROADMAP.md
│
└── .github/
    └── workflows/
        └── ci.yml                 ✅ CI/CD
```

---

## 개발 로드맵

### ✅ 완료된 Phase (0-8)

**Phase 0-1**: 프로젝트 초기화 및 핵심 기능 ✅
- [x] GitHub 저장소 생성
- [x] 기본 파일 구조
- [x] CMake 빌드 시스템
- [x] CI/CD 설정
- [x] Lexer/Parser 구현
- [x] 조사 기반 메서드 체인
- [x] 자연스러운 범위 표현
- [x] 명확한 타입 시스템
- [x] 직관적 반복문

**Phase 2-7**: 고급 기능 ✅
- [x] 클래스 및 상속
- [x] Async/Await
- [x] Try/Catch/Finally
- [x] 패턴 매칭
- [x] 모듈 시스템
- [x] 바이트코드 VM
- [x] JIT Compiler Tier 1
- [x] 세대별 GC
- [x] LSP
- [x] 디버거
- [x] Linter & Formatter
- [x] VS Code 확장

**Phase 8**: 코드 품질 및 문서화 ✅
- [x] Doxygen 설정
- [x] USER_GUIDE.md
- [x] regression_test.py
- [x] 1,503개 테스트 (100% 통과)

### 📝 다음 Phase (9 이후)

**Phase 9**: 리팩토링 및 최적화
- [ ] 긴 함수 리팩토링 (VM, Evaluator)
- [ ] Incremental GC 구현
- [ ] 메모리 풀링 전략
- [ ] stdlib/http, stdlib/db 완성

**장기 계획**:
- [ ] LLVM 백엔드 (선택)
- [ ] 멀티스레딩 지원
- [ ] 세미콜론 필수화 (v2.0.0)

상세한 로드맵은 `ROADMAP.md`를 참조하세요.

---

## 참고 문서

KingSejong 프로젝트의 주요 문서들:

### 사용자 문서
- **docs/TUTORIAL.md**: 초보자를 위한 단계별 튜토리얼
- **docs/LANGUAGE_REFERENCE.md**: 완전한 언어 레퍼런스
- **docs/USER_GUIDE.md**: 사용자 가이드 (설치, 실행, 디버깅)
- **docs/LIBRARY_GUIDE.md**: 모듈 개발 가이드
- **stdlib/README.md**: 표준 라이브러리 함수 목록

### 개발자 문서
- **docs/ROADMAP.md**: 개발 로드맵 및 진행 상황
- **docs/CONTRIBUTING.md**: 기여 가이드
- **docs/design/LANGUAGE_DESIGN_PHILOSOPHY.md**: 언어 설계 철학
- **docs/design/PROJECT_OVERVIEW.md**: 프로젝트 개요
- **docs/design/FEATURES.md**: 기능 명세
- **docs/design/IMPLEMENTATION.md**: 구현 계획

### 기술 문서
- **docs/GC_DESIGN.md**: 가비지 컬렉터 설계
- **vscode-extension/README.md**: VS Code 확장 가이드

---

## 개발 환경 요구 사항

### 필수
- **CMake**: 3.21 이상
- **컴파일러**: C++23 지원
  - GCC 11+ 또는
  - Clang 14+ 또는
  - MSVC 2022+

### 권장
- **GoogleTest**: 테스트 프레임워크
  - macOS: `brew install googletest`
  - Ubuntu: `sudo apt install libgtest-dev`
  - Windows: `vcpkg install gtest`

---

## 빠른 시작

KingSejong 언어를 사용해보세요:

```bash
cd kingsejonglang/build

# 버전 확인
./bin/kingsejong --version

# 예상 출력:
# KingSejong Programming Language v0.5.0
# Copyright (c) 2025 KingSejong Programming Language Contributors
# Licensed under MIT License

# REPL 모드
./bin/kingsejong

# 파일 실행
./bin/kingsejong ../examples/hello.ksj

# LSP 서버 모드
./bin/kingsejong --lsp
```

---

## 기여하기

KingSejong 프로젝트에 기여하는 방법:

1. **Fork the repository**
2. **Create your feature branch** (`git checkout -b feature/AmazingFeature`)
3. **Commit your changes** (`git commit -m 'feat: Add AmazingFeature'`)
4. **Push to the branch** (`git push origin feature/AmazingFeature`)
5. **Open a Pull Request**

자세한 가이드: [docs/CONTRIBUTING.md](/Users/kevin/work/github/kingsejong-lang/kingsejonglang/docs/CONTRIBUTING.md)

---

## 커뮤니티

- **Issues**: 버그 리포트, 기능 제안
- **Discussions**: 질문, 아이디어 공유
- **GitHub**: https://github.com/0xmhha/kingsejonglang

---

## 라이센스

MIT License - 교육용 프로젝트에 최적화된 오픈소스 라이센스

---

## 질문이나 문제가 있으신가요?

1. `docs/USER_GUIDE.md`에서 사용자 가이드 확인
2. `docs/TUTORIAL.md`에서 튜토리얼 확인
3. GitHub Issues를 통해 문의

---

**"백성을 가르치는 바른 코드"** - KingSejong Programming Language

프로젝트 기여를 환영합니다!
