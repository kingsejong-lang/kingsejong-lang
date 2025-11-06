# Sejong 프로젝트 시작 가이드

새로운 Sejong 프로그래밍 언어 프로젝트를 위한 스타터 패키지가 준비되었습니다.

---

## 생성된 파일들

`sejong-starter/` 디렉토리에 다음 파일들이 생성되었습니다:

### 핵심 파일
- **LICENSE**: MIT License
- **README.md**: 프로젝트 소개 및 사용법 (한글)
- **CMakeLists.txt**: C++23 빌드 설정
- **.gitignore**: Git 제외 파일 목록

### 소스 코드
- **src/main.cpp**: 메인 진입점 (스켈레톤 구조)
  - 파일 실행 모드
  - REPL 모드
  - 버전/도움말 출력

### 예제 코드
- **examples/hello.sj**: Hello World
- **examples/basic.sj**: 기본 문법 (변수, 조건문, 반복문, 함수)

### 문서
- **docs/CONTRIBUTING.md**: 기여 가이드
- **docs/PROJECT_SETUP.md**: 프로젝트 설정 상세 가이드

### CI/CD
- **.github/workflows/ci.yml**: GitHub Actions 설정
  - Ubuntu, macOS, Windows 크로스 플랫폼 빌드
  - 자동 테스트 실행

---

## 새 GitHub 저장소 생성 방법

### 1. GitHub에서 새 저장소 생성
```
Repository name: sejong (또는 sejong-lang)
Description: 한글의 언어적 특성을 활용한 교육용 프로그래밍 언어
Public: ✓
Initialize: 체크 해제
```

### 2. 로컬에서 Git 초기화 및 푸시
```bash
cd sejong-starter

# Git 초기화
git init

# 원격 저장소 추가 (YOUR_USERNAME을 본인 계정으로 변경)
git remote add origin https://github.com/YOUR_USERNAME/sejong.git

# 모든 파일 추가
git add .

# 첫 커밋
git commit -m "Initial commit: Sejong 프로젝트 시작

- MIT License 추가
- CMake 빌드 시스템 설정
- 기본 프로젝트 구조
- Hello World 예제
- CI/CD 설정"

# 메인 브랜치 설정 및 푸시
git branch -M main
git push -u origin main
```

### 3. 빌드 및 테스트
```bash
# 빌드
mkdir build && cd build
cmake ..
make

# 실행
./sejong --version
./sejong ../examples/hello.sj
```

---

## 프로젝트 구조

```
sejong-starter/
├── LICENSE                        ✓ MIT License
├── README.md                      ✓ 프로젝트 소개
├── CMakeLists.txt                 ✓ 빌드 설정
├── .gitignore                     ✓ Git 제외
│
├── src/
│   ├── main.cpp                   ✓ 진입점
│   ├── lexer/                     TODO
│   ├── parser/                    TODO
│   ├── ast/                       TODO
│   ├── evaluator/                 TODO
│   ├── types/                     TODO
│   ├── stdlib/                    TODO
│   └── repl/                      TODO
│
├── stdlib/                        TODO
├── tests/                         TODO
│
├── examples/
│   ├── hello.sj                   ✓ Hello World
│   └── basic.sj                   ✓ 기본 문법
│
├── docs/
│   ├── CONTRIBUTING.md            ✓ 기여 가이드
│   └── PROJECT_SETUP.md           ✓ 설정 가이드
│
└── .github/
    └── workflows/
        └── ci.yml                 ✓ CI/CD
```

---

## 다음 단계

### Phase 0: 프로젝트 초기화 (1주)

**Week 1 (현재 완료)**:
- [x] GitHub 저장소 생성
- [x] 기본 파일 구조
- [x] CMake 빌드 시스템
- [x] CI/CD 설정

**Week 2 (다음 작업)**:
- [ ] Lexer 기본 구조 구현
  - Token 타입 정의
  - UTF-8 한글 처리
  - 기본 토큰화
- [ ] 첫 번째 테스트 작성

### Phase 1: 핵심 차별화 기능 (3개월)
- [ ] 조사 기반 메서드 체인
- [ ] 자연스러운 범위 표현
- [ ] 명확한 타입 시스템
- [ ] 직관적 반복문

상세한 구현 계획은 `SEJONG_IMPLEMENTATION.md`를 참조하세요.

---

## 참고 문서

현재 디렉토리에 다음 설계 문서들이 있습니다:

- **LANGUAGE_DESIGN_PHILOSOPHY.md**: 언어 설계 철학 (핵심!)
- **SEJONG_FEATURES.md**: 기능 명세
- **SEJONG_IMPLEMENTATION.md**: 구현 계획
- **SEJONG_PROJECT_OVERVIEW.md**: 프로젝트 개요

이 문서들을 새 저장소의 `docs/` 디렉토리에 복사하는 것을 권장합니다:

```bash
cp LANGUAGE_DESIGN_PHILOSOPHY.md sejong-starter/docs/LANGUAGE_DESIGN.md
cp SEJONG_FEATURES.md sejong-starter/docs/FEATURES.md
cp SEJONG_IMPLEMENTATION.md sejong-starter/docs/IMPLEMENTATION.md
```

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

## 빠른 확인

생성된 파일이 제대로 작동하는지 확인:

```bash
cd sejong-starter

# 빌드
mkdir build && cd build
cmake ..
make

# 버전 확인
./sejong --version

# 예상 출력:
# Sejong Programming Language v1.0.0
# Copyright (c) 2025 Sejong Programming Language Contributors
# Licensed under MIT License

# 도움말
./sejong --help

# 파일 실행 (아직 구현 전이므로 "구현 예정..." 메시지)
./sejong ../examples/hello.sj
```

---

## 커뮤니티

저장소 생성 후 권장 설정:

1. **Issues 활성화**: 버그 리포트, 기능 제안
2. **Discussions 활성화**: 질문, 아이디어 공유
3. **Wiki 활성화**: 문서화

---

## 라이센스

MIT License - 교육용 프로젝트에 최적화된 오픈소스 라이센스

---

## 질문이나 문제가 있으신가요?

1. `docs/PROJECT_SETUP.md`에서 상세한 설정 가이드 확인
2. `SEJONG_IMPLEMENTATION.md`에서 구현 계획 확인
3. GitHub Issues를 통해 문의

---

**"백성을 가르치는 바른 코드"** - Sejong Programming Language

새 프로젝트 시작을 축하합니다!
