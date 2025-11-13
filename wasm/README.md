# KingSejong WebAssembly 빌드 가이드

KingSejong 언어를 WebAssembly로 빌드하여 웹 브라우저에서 실행할 수 있습니다.

## 📋 요구사항

- **Emscripten SDK** (버전 3.1.0 이상 권장)
- CMake 3.20 이상
- Python 3.6 이상

## 🚀 Emscripten 설치

### 1. emsdk 다운로드 및 설치

```bash
# emsdk 저장소 클론
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# 최신 버전 설치
./emsdk install latest

# 최신 버전 활성화
./emsdk activate latest

# 환경 변수 설정
source ./emsdk_env.sh
```

### 2. 설치 확인

```bash
emcc --version
```

출력 예시:
```
emcc (Emscripten gcc/clang-like replacement + linker emulating GNU ld) 3.1.50
```

## 🔨 빌드 방법

### 빠른 빌드

```bash
# 프로젝트 루트에서
./scripts/build-wasm.sh
```

### 옵션

- `--clean`: 빌드 디렉토리 삭제 후 재빌드
- `--debug`: 디버그 빌드 (기본값: Release)

```bash
# 클린 빌드
./scripts/build-wasm.sh --clean

# 디버그 빌드
./scripts/build-wasm.sh --debug
```

### 수동 빌드

```bash
# 빌드 디렉토리 생성
mkdir build-wasm
cd build-wasm

# CMake 설정 (Emscripten 툴체인 사용)
emcmake cmake .. -DEMSCRIPTEN=ON

# 빌드
emmake make kingsejong_wasm -j8
```

## 📦 빌드 결과

빌드가 성공하면 다음 파일이 생성됩니다:

```
build-wasm/wasm/
├── kingsejong.js       # JavaScript 래퍼
└── kingsejong.wasm     # WebAssembly 바이너리
```

이 파일들은 자동으로 `web/public/` 디렉토리로 복사됩니다.

## 🧪 테스트

### 간단한 테스트

빌드 후 간단한 HTML 파일로 테스트할 수 있습니다:

```bash
# 로컬 서버 실행 (Python)
cd wasm
python3 -m http.server 8000
```

브라우저에서 `http://localhost:8000/test.html` 접속

### JavaScript API 사용 예제

```javascript
// WASM 모듈 로드
createKingSejong().then(module => {
    // 인터프리터 생성
    const interpreter = new module.KingSejongInterpreter();

    // 버전 확인
    console.log(interpreter.version());
    // => "KingSejong v0.3.2 (WebAssembly)"

    // 코드 실행
    const result = interpreter.evaluate(`
        변수 x = 10 + 20
        x를 출력한다
    `);
    console.log(result);  // => "30"

    // 환경 초기화
    interpreter.reset();
});
```

## 📚 API 레퍼런스

### `KingSejongInterpreter` 클래스

#### `constructor()`
인터프리터를 생성하고 환경을 초기화합니다.

#### `evaluate(code: string): string`
KingSejong 코드를 평가하고 결과를 반환합니다.

- **매개변수**:
  - `code`: 실행할 KingSejong 소스 코드
- **반환값**: 실행 결과 문자열
  - 에러 발생 시: `"ERROR: "` 접두사와 함께 에러 메시지 반환
  - 반환값 없음: 빈 문자열 `""`

#### `reset(): void`
인터프리터 환경을 초기화합니다. 모든 변수가 삭제되고 내장 함수만 남습니다.

#### `version(): string`
KingSejong 버전 정보를 반환합니다.

## 🔧 빌드 설정

### Emscripten 플래그

`CMakeLists.txt`에 다음 Emscripten 플래그가 설정되어 있습니다:

- `-s WASM=1`: WebAssembly 생성
- `-s MODULARIZE=1`: ES6 모듈로 내보내기
- `-s EXPORT_NAME='createKingSejong'`: 모듈 생성 함수 이름
- `-s ALLOW_MEMORY_GROWTH=1`: 동적 메모리 증가 허용
- `-s ENVIRONMENT=web`: 웹 환경 전용
- `-s FILESYSTEM=0`: 파일 시스템 비활성화 (크기 최적화)
- `--bind`: embind 사용 (C++/JavaScript 바인딩)
- `-O3`: 최적화 레벨 3

## ❓ 문제 해결

### "emcc를 찾을 수 없습니다"

Emscripten 환경 변수가 설정되지 않았습니다.

```bash
# emsdk 디렉토리에서
source ./emsdk_env.sh
```

매 터미널 세션마다 실행해야 합니다. 자동 설정을 원하면 `.bashrc` 또는 `.zshrc`에 추가:

```bash
# ~/.bashrc 또는 ~/.zshrc
source /path/to/emsdk/emsdk_env.sh
```

### "메모리 부족" 에러

`-s ALLOW_MEMORY_GROWTH=1` 플래그가 활성화되어 있으므로 메모리는 자동으로 증가합니다. 만약 문제가 계속되면 초기 메모리를 늘려보세요:

```cmake
LINK_FLAGS "-s INITIAL_MEMORY=67108864"  # 64MB
```

### CORS 에러

로컬에서 `file://` 프로토콜로 HTML을 열면 CORS 에러가 발생할 수 있습니다. 로컬 서버를 실행하세요:

```bash
# Python 3
python3 -m http.server 8000

# Node.js (http-server 패키지)
npx http-server -p 8000
```

## 📝 다음 단계

1. ✅ WebAssembly 빌드 설정 완료
2. 🔄 웹 플레이그라운드 인터페이스 개발
3. 🔄 Monaco Editor 통합
4. 🔄 예제 갤러리 구현
5. 🔄 GitHub Pages 배포

---

**작성일**: 2025-11-14
**버전**: KingSejong v0.3.2
**작성자**: KingSejong Team
