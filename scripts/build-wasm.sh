#!/bin/bash

# ============================================================================
# KingSejong WebAssembly 빌드 스크립트
# ============================================================================
# 이 스크립트는 Emscripten을 사용하여 KingSejong을 WebAssembly로 빌드합니다.
#
# 사용법:
#   ./scripts/build-wasm.sh [옵션]
#
# 옵션:
#   --clean    빌드 디렉토리를 삭제하고 처음부터 빌드
#   --debug    디버그 빌드 (기본값: Release)
#
# 요구사항:
#   - Emscripten SDK (emsdk) 설치 필요
#   - emsdk 활성화: source /path/to/emsdk/emsdk_env.sh
# ============================================================================

set -e  # 에러 발생 시 즉시 종료

# 색상 정의
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 프로젝트 루트 디렉토리
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build-wasm"

# 기본 빌드 타입
BUILD_TYPE="Release"
CLEAN=false

# ============================================================================
# 인자 파싱
# ============================================================================
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN=true
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        *)
            echo -e "${RED}알 수 없는 옵션: $1${NC}"
            echo "사용법: $0 [--clean] [--debug]"
            exit 1
            ;;
    esac
done

# ============================================================================
# Emscripten 환경 체크
# ============================================================================
echo -e "${BLUE}=== Emscripten 환경 체크 ===${NC}"

if ! command -v emcc &> /dev/null; then
    echo -e "${RED}오류: emcc를 찾을 수 없습니다.${NC}"
    echo ""
    echo "Emscripten SDK를 설치하고 활성화해주세요:"
    echo ""
    echo "  # emsdk 설치 (최초 1회)"
    echo "  git clone https://github.com/emscripten-core/emsdk.git"
    echo "  cd emsdk"
    echo "  ./emsdk install latest"
    echo "  ./emsdk activate latest"
    echo ""
    echo "  # 환경 변수 설정 (매 터미널 세션마다)"
    echo "  source ./emsdk_env.sh"
    echo ""
    exit 1
fi

EMCC_VERSION=$(emcc --version | head -n 1)
echo -e "${GREEN}✓ Emscripten 발견: $EMCC_VERSION${NC}"

# ============================================================================
# 빌드 디렉토리 준비
# ============================================================================
if [ "$CLEAN" = true ] && [ -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}빌드 디렉토리 삭제 중...${NC}"
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# ============================================================================
# CMake 설정
# ============================================================================
echo ""
echo -e "${BLUE}=== CMake 설정 ===${NC}"
echo "빌드 타입: $BUILD_TYPE"
echo "출력 디렉토리: $BUILD_DIR/wasm"
echo ""

emcmake cmake "$PROJECT_ROOT" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DEMSCRIPTEN=ON

# ============================================================================
# 빌드
# ============================================================================
echo ""
echo -e "${BLUE}=== 빌드 시작 ===${NC}"

emmake make kingsejong_wasm -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# ============================================================================
# 빌드 결과 확인
# ============================================================================
echo ""
echo -e "${BLUE}=== 빌드 완료 ===${NC}"

WASM_OUTPUT="$BUILD_DIR/wasm"

if [ -f "$WASM_OUTPUT/kingsejong.js" ] && [ -f "$WASM_OUTPUT/kingsejong.wasm" ]; then
    echo -e "${GREEN}✓ WebAssembly 모듈 생성 완료!${NC}"
    echo ""
    echo "출력 파일:"
    echo "  - $WASM_OUTPUT/kingsejong.js"
    echo "  - $WASM_OUTPUT/kingsejong.wasm"
    echo ""

    # 파일 크기 표시
    JS_SIZE=$(du -h "$WASM_OUTPUT/kingsejong.js" | cut -f1)
    WASM_SIZE=$(du -h "$WASM_OUTPUT/kingsejong.wasm" | cut -f1)

    echo "파일 크기:"
    echo "  - JavaScript: $JS_SIZE"
    echo "  - WebAssembly: $WASM_SIZE"
    echo ""

    # 웹 디렉토리로 복사
    WEB_DIR="$PROJECT_ROOT/web/public"
    if [ -d "$WEB_DIR" ]; then
        echo -e "${YELLOW}웹 디렉토리로 복사 중...${NC}"
        cp "$WASM_OUTPUT/kingsejong.js" "$WEB_DIR/"
        cp "$WASM_OUTPUT/kingsejong.wasm" "$WEB_DIR/"
        echo -e "${GREEN}✓ $WEB_DIR 로 복사 완료${NC}"
    fi

else
    echo -e "${RED}오류: WebAssembly 모듈을 생성하지 못했습니다.${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}=== 빌드 성공! ===${NC}"
