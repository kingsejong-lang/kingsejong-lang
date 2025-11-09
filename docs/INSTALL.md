# KingSejong 설치 가이드

> 모든 주요 플랫폼을 위한 상세한 설치 가이드

**작성일**: 2025-11-10
**대상**: macOS, Linux (Ubuntu, Debian, Fedora, CentOS), Windows

---

## 📋 목차

1. [사전 요구사항](#사전-요구사항)
2. [macOS 설치](#macos-설치)
3. [Linux 설치](#linux-설치)
4. [Windows 설치](#windows-설치)
5. [설치 확인](#설치-확인)
6. [문제 해결](#문제-해결)

---

## 사전 요구사항

모든 플랫폼에서 다음이 필요합니다:

### 필수 구성 요소

- **C++23 지원 컴파일러**:
  - GCC 12 이상
  - Clang 15 이상
  - MSVC 2022 이상 (Visual Studio 17.0+)
- **CMake 3.20 이상**
- **Git**

### 선택 사항

- **GoogleTest** (자동 다운로드됨)
- **Python 3.8+** (벤치마크 실행용)

---

## macOS 설치

### 1. Xcode Command Line Tools 설치

```bash
xcode-select --install
```

### 2. Homebrew 설치 (선택사항)

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 3. CMake 설치

**Homebrew 사용:**
```bash
brew install cmake
```

**또는 공식 다운로드:**
- [cmake.org](https://cmake.org/download/)에서 다운로드

### 4. KingSejong 빌드

```bash
# 저장소 클론
git clone https://github.com/0xmhha/kingsejonglang.git
cd kingsejonglang

# 빌드 디렉토리 생성
mkdir build && cd build

# CMake 설정
cmake ..

# 빌드 (병렬 빌드 -j4)
make -j4

# 테스트 실행
ctest --output-on-failure
```

### 5. 실행 파일 경로

빌드 후 실행 파일은 다음 위치에 생성됩니다:
```
build/bin/kingsejong
```

### 6. 환경 변수 설정 (선택사항)

```bash
# ~/.zshrc 또는 ~/.bash_profile에 추가
export PATH="/path/to/kingsejonglang/build/bin:$PATH"

# 적용
source ~/.zshrc  # zsh
# 또는
source ~/.bash_profile  # bash
```

---

## Linux 설치

### Ubuntu / Debian 계열

#### 1. 시스템 업데이트

```bash
sudo apt update
sudo apt upgrade -y
```

#### 2. 필수 패키지 설치

```bash
sudo apt install -y \
    build-essential \
    cmake \
    git \
    pkg-config
```

#### 3. 최신 GCC 설치 (Ubuntu 20.04 이하인 경우)

```bash
# GCC 12 설치
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install -y gcc-12 g++-12

# 기본 컴파일러로 설정
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 120
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 120
```

#### 4. KingSejong 빌드

```bash
# 저장소 클론
git clone https://github.com/0xmhha/kingsejonglang.git
cd kingsejonglang

# 빌드
mkdir build && cd build
cmake ..
make -j$(nproc)

# 테스트
ctest --output-on-failure
```

### Fedora / CentOS / RHEL 계열

#### 1. 필수 패키지 설치

**Fedora:**
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    git \
    make
```

**CentOS 8+:**
```bash
sudo dnf install -y \
    gcc-toolset-12 \
    cmake \
    git \
    make

# GCC 12 활성화
scl enable gcc-toolset-12 bash
```

#### 2. KingSejong 빌드

```bash
# 저장소 클론
git clone https://github.com/0xmhha/kingsejonglang.git
cd kingsejonglang

# 빌드
mkdir build && cd build
cmake ..
make -j$(nproc)

# 테스트
ctest --output-on-failure
```

### Arch Linux

```bash
# 필수 패키지 설치
sudo pacman -S --needed base-devel cmake git

# 빌드
git clone https://github.com/0xmhha/kingsejonglang.git
cd kingsejonglang
mkdir build && cd build
cmake ..
make -j$(nproc)
```

---

## Windows 설치

### 방법 1: Visual Studio 2022 사용 (권장)

#### 1. Visual Studio 2022 설치

- [Visual Studio 2022](https://visualstudio.microsoft.com/ko/downloads/)  다운로드
- **워크로드** 선택:
  - "C++를 사용한 데스크톱 개발"
- **개별 구성 요소** 확인:
  - MSVC v143 - VS 2022 C++ x64/x86 빌드 도구
  - CMake용 C++ 도구
  - C++23 지원

#### 2. Git 설치

- [Git for Windows](https://git-scm.com/download/win) 다운로드 및 설치

#### 3. CMake 설치 (Visual Studio에 포함되지 않은 경우)

- [cmake.org](https://cmake.org/download/)에서 Windows 인스톨러 다운로드
- 설치 시 "Add CMake to the system PATH" 선택

#### 4. KingSejong 빌드

**PowerShell 또는 CMD 사용:**

```powershell
# 저장소 클론
git clone https://github.com/0xmhha/kingsejonglang.git
cd kingsejonglang

# 빌드 디렉토리 생성
mkdir build
cd build

# CMake 설정
cmake .. -G "Visual Studio 17 2022" -A x64

# 빌드
cmake --build . --config Release

# 테스트
ctest --output-on-failure -C Release
```

#### 5. 실행

```powershell
.\bin\Release\kingsejong.exe ..\examples\hello.ksj
```

### 방법 2: MinGW-w64 사용

#### 1. MSYS2 설치

- [MSYS2](https://www.msys2.org/) 다운로드 및 설치

#### 2. 필수 패키지 설치

**MSYS2 터미널에서:**

```bash
# 패키지 업데이트
pacman -Syu

# 개발 도구 설치
pacman -S --needed \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-make \
    git
```

#### 3. KingSejong 빌드

```bash
# 저장소 클론
git clone https://github.com/0xmhha/kingsejonglang.git
cd kingsejonglang

# 빌드
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4

# 테스트
ctest --output-on-failure
```

---

## 설치 확인

설치가 완료되면 다음을 테스트하세요:

### 1. 버전 확인

```bash
# macOS/Linux
./bin/kingsejong --version

# Windows
.\bin\Release\kingsejong.exe --version
```

**예상 출력:**
```
KingSejong v0.1.0
```

### 2. REPL 실행

```bash
# macOS/Linux
./bin/kingsejong

# Windows
.\bin\Release\kingsejong.exe
```

**예상 출력:**
```
KingSejong REPL v0.1.0
명령어: .exit 종료 | .help 도움말 | .clear 초기화 | .vars 변수목록

>>>
```

간단한 코드 테스트:
```
>>> 출력("안녕하세요!")
안녕하세요!

>>> x = 10 + 20
>>> 출력(x)
30

>>> .exit
```

### 3. 예제 실행

```bash
# macOS/Linux
./bin/kingsejong ../examples/hello.ksj

# Windows
.\bin\Release\kingsejong.exe ..\examples\hello.ksj
```

**예상 출력:**
```
안녕, KingSejong!
```

### 4. 테스트 스위트 실행

```bash
ctest --output-on-failure
```

**예상 출력:**
```
100% tests passed, 0 tests failed out of 330+
```

---

## 문제 해결

### 일반적인 문제

#### 1. CMake 버전 오류

**오류:**
```
CMake Error: CMake 3.20 or higher is required
```

**해결:**
- 최신 CMake 설치: https://cmake.org/download/

#### 2. C++23 컴파일러 없음

**오류:**
```
The compiler does not support C++23
```

**해결 (Linux):**
```bash
# GCC 12+ 설치
sudo apt install gcc-12 g++-12  # Ubuntu/Debian
sudo dnf install gcc-toolset-12  # Fedora/CentOS
```

**해결 (macOS):**
```bash
# Xcode Command Line Tools 업데이트
xcode-select --install
```

**해결 (Windows):**
- Visual Studio 2022 최신 버전 설치

#### 3. GoogleTest 다운로드 실패

**오류:**
```
Failed to download GoogleTest
```

**해결:**
```bash
# 수동으로 GoogleTest 설치 후 빌드 재시도
cd build
cmake .. -DBUILD_TESTING=OFF  # 테스트 비활성화
make -j4
```

#### 4. 메모리 부족 (빌드 실패)

**오류:**
```
c++: fatal error: Killed signal terminated program cc1plus
```

**해결:**
```bash
# 병렬 빌드 수 줄이기
make -j1  # 또는 -j2
```

#### 5. Windows에서 경로 오류

**오류:**
```
Cannot find file: examples/hello.ksj
```

**해결:**
```powershell
# 백슬래시 사용 또는 상대 경로 확인
.\bin\Release\kingsejong.exe ..\examples\hello.ksj
```

### 플랫폼별 문제

#### macOS

**문제: Xcode Command Line Tools 설치되지 않음**
```bash
xcode-select --install
```

**문제: Permission denied**
```bash
sudo make install  # /usr/local에 설치 시
```

#### Linux

**문제: 헤더 파일 없음**
```bash
sudo apt install build-essential  # Ubuntu/Debian
sudo dnf groupinstall "Development Tools"  # Fedora
```

**문제: libstdc++ 버전 불일치**
```bash
# 최신 libstdc++ 설치
sudo apt install libstdc++6
```

#### Windows

**문제: MSVC 찾을 수 없음**
```powershell
# Visual Studio Developer Command Prompt 사용
# 또는 환경 변수 설정
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
```

**문제: DLL 없음**
- Release 빌드 사용 (정적 링크)
- 또는 필요한 DLL을 실행 파일과 같은 디렉토리에 복사

---

## 추가 리소스

- **공식 문서**: [docs/](.)
- **튜토리얼**: [TUTORIAL.md](TUTORIAL.md)
- **예제**: [../examples/](../examples/)
- **문제 보고**: [GitHub Issues](https://github.com/0xmhha/kingsejonglang/issues)

---

## 다음 단계

설치가 완료되면:

1. **[튜토리얼](TUTORIAL.md)** 시작
2. **[예제 프로그램](../examples/)** 실행
3. **[언어 레퍼런스](LANGUAGE_REFERENCE.md)** 읽기
4. **나만의 프로그램** 작성하기!

즐거운 코딩 되세요! 🎉
