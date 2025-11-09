#!/bin/bash

##############################################################################
# KingSejong 언어 설치 스크립트 (macOS/Linux)
#
# 사용법:
#   ./install.sh              # 기본 설치 (./build/bin/kingsejong)
#   ./install.sh --system     # 시스템 설치 (/usr/local/bin/kingsejong)
#   ./install.sh --help       # 도움말
#
# 작성일: 2025-11-10
##############################################################################

set -e  # 오류 발생 시 즉시 종료

# 색상 정의
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 로그 함수
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 도움말
show_help() {
    cat << EOF
KingSejong 언어 설치 스크립트

사용법:
  $0 [옵션]

옵션:
  --system        시스템 전역 설치 (/usr/local/bin)
  --prefix PATH   사용자 지정 경로에 설치
  --jobs N        병렬 빌드 작업 수 (기본: 4)
  --no-tests      테스트 실행 건너뛰기
  --clean         기존 빌드 삭제 후 재빌드
  --help          이 도움말 표시

예제:
  $0                          # 로컬 빌드
  $0 --system                 # 시스템 설치
  $0 --prefix ~/.local        # 홈 디렉토리에 설치
  $0 --jobs 8 --no-tests      # 빠른 빌드

EOF
}

# 옵션 파싱
SYSTEM_INSTALL=false
CUSTOM_PREFIX=""
JOBS=4
RUN_TESTS=true
CLEAN_BUILD=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --system)
            SYSTEM_INSTALL=true
            shift
            ;;
        --prefix)
            CUSTOM_PREFIX="$2"
            shift 2
            ;;
        --jobs)
            JOBS="$2"
            shift 2
            ;;
        --no-tests)
            RUN_TESTS=false
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            log_error "알 수 없는 옵션: $1"
            show_help
            exit 1
            ;;
    esac
done

# 배너
echo ""
echo "================================="
echo "  KingSejong 언어 설치 스크립트  "
echo "================================="
echo ""

# 1. 플랫폼 감지
log_info "플랫폼 감지 중..."
OS=$(uname -s)
ARCH=$(uname -m)

case "$OS" in
    Darwin*)
        PLATFORM="macOS"
        ;;
    Linux*)
        PLATFORM="Linux"
        ;;
    *)
        log_error "지원하지 않는 플랫폼: $OS"
        exit 1
        ;;
esac

log_info "플랫폼: $PLATFORM ($ARCH)"

# 2. 필수 도구 확인
log_info "필수 도구 확인 중..."

check_command() {
    if ! command -v $1 &> /dev/null; then
        log_error "$1을(를) 찾을 수 없습니다."
        log_info "$2"
        exit 1
    fi
    log_success "$1 확인됨: $(command -v $1)"
}

check_command "git" "git을 설치하세요: https://git-scm.com/"
check_command "cmake" "cmake를 설치하세요: https://cmake.org/"

# C++ 컴파일러 확인
if command -v g++ &> /dev/null; then
    CXX_COMPILER="g++"
elif command -v clang++ &> /dev/null; then
    CXX_COMPILER="clang++"
else
    log_error "C++ 컴파일러를 찾을 수 없습니다."
    if [[ "$PLATFORM" == "macOS" ]]; then
        log_info "Xcode Command Line Tools를 설치하세요: xcode-select --install"
    else
        log_info "GCC를 설치하세요: sudo apt install build-essential"
    fi
    exit 1
fi

log_success "C++ 컴파일러 확인됨: $CXX_COMPILER"

# 3. CMake 버전 확인
CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
CMAKE_REQUIRED="3.20"

log_info "CMake 버전: $CMAKE_VERSION (요구사항: $CMAKE_REQUIRED+)"

# 4. 기존 빌드 정리 (--clean 옵션)
if [[ "$CLEAN_BUILD" == true ]]; then
    if [[ -d "build" ]]; then
        log_warning "기존 빌드 디렉토리 삭제 중..."
        rm -rf build
        log_success "빌드 디렉토리 삭제 완료"
    fi
fi

# 5. 빌드 디렉토리 생성
log_info "빌드 디렉토리 준비 중..."
mkdir -p build
cd build

# 6. CMake 설정
log_info "CMake 설정 중..."

CMAKE_ARGS=""

if [[ "$SYSTEM_INSTALL" == true ]]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=/usr/local"
    log_info "시스템 설치 모드: /usr/local"
elif [[ -n "$CUSTOM_PREFIX" ]]; then
    CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=$CUSTOM_PREFIX"
    log_info "사용자 지정 설치 경로: $CUSTOM_PREFIX"
fi

# Release 빌드
CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release"

cmake .. $CMAKE_ARGS

if [[ $? -eq 0 ]]; then
    log_success "CMake 설정 완료"
else
    log_error "CMake 설정 실패"
    exit 1
fi

# 7. 빌드
log_info "빌드 중... (병렬 작업 수: $JOBS)"

if command -v make &> /dev/null; then
    make -j$JOBS
else
    cmake --build . --parallel $JOBS
fi

if [[ $? -eq 0 ]]; then
    log_success "빌드 완료"
else
    log_error "빌드 실패"
    exit 1
fi

# 8. 테스트 실행
if [[ "$RUN_TESTS" == true ]]; then
    log_info "테스트 실행 중..."
    if ctest --output-on-failure; then
        log_success "모든 테스트 통과"
    else
        log_warning "일부 테스트 실패 (무시하고 계속)"
    fi
else
    log_warning "테스트 건너뛰기 (--no-tests)"
fi

# 9. 설치
if [[ "$SYSTEM_INSTALL" == true ]] || [[ -n "$CUSTOM_PREFIX" ]]; then
    log_info "설치 중..."

    if [[ "$SYSTEM_INSTALL" == true ]]; then
        if command -v sudo &> /dev/null; then
            sudo make install
        else
            make install
        fi
    else
        make install
    fi

    if [[ $? -eq 0 ]]; then
        log_success "설치 완료"

        if [[ "$SYSTEM_INSTALL" == true ]]; then
            INSTALL_PATH="/usr/local/bin/kingsejong"
        else
            INSTALL_PATH="$CUSTOM_PREFIX/bin/kingsejong"
        fi

        log_info "실행 파일 위치: $INSTALL_PATH"
    else
        log_error "설치 실패"
        exit 1
    fi
else
    log_info "로컬 빌드 완료 (설치하지 않음)"
    log_info "실행 파일 위치: $(pwd)/bin/kingsejong"
fi

# 10. 설치 확인
log_info "설치 확인 중..."

cd ..

if [[ "$SYSTEM_INSTALL" == true ]] || [[ -n "$CUSTOM_PREFIX" ]]; then
    KINGSEJONG_BIN="kingsejong"
else
    KINGSEJONG_BIN="./build/bin/kingsejong"
fi

if [[ -f "$KINGSEJONG_BIN" ]] || command -v kingsejong &> /dev/null; then
    echo ""
    echo "===================================="
    log_success "KingSejong 설치 성공! 🎉"
    echo "===================================="
    echo ""

    # 버전 확인
    if command -v kingsejong &> /dev/null; then
        VERSION=$(kingsejong --version 2>/dev/null || echo "unknown")
    else
        VERSION=$($KINGSEJONG_BIN --version 2>/dev/null || echo "unknown")
    fi

    log_info "버전: $VERSION"
    echo ""

    # 사용법 안내
    echo "사용법:"
    echo ""

    if [[ "$SYSTEM_INSTALL" == true ]] || [[ -n "$CUSTOM_PREFIX" ]]; then
        echo "  kingsejong                    # REPL 시작"
        echo "  kingsejong script.ksj         # 스크립트 실행"
        echo "  kingsejong --help             # 도움말"
    else
        echo "  ./build/bin/kingsejong                    # REPL 시작"
        echo "  ./build/bin/kingsejong script.ksj         # 스크립트 실행"
        echo "  ./build/bin/kingsejong --help             # 도움말"
    fi

    echo ""
    echo "예제 실행:"
    echo "  $KINGSEJONG_BIN examples/hello.ksj"
    echo ""
    log_info "문서: docs/INSTALL.md"
    log_info "튜토리얼: docs/TUTORIAL.md"
    log_info "예제: examples/"
    echo ""
else
    log_error "설치 확인 실패: 실행 파일을 찾을 수 없습니다"
    exit 1
fi
