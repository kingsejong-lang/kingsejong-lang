# KingSejong 플레이그라운드 개발 튜토리얼

> **날짜**: 2025-11-21
> **버전**: v0.5.0
> **대상**: 플레이그라운드 개발자 및 기여자

---

## 목차

1. [개요](#1-개요)
2. [아키텍처](#2-아키텍처)
3. [WebAssembly 빌드](#3-webassembly-빌드)
4. [프론트엔드 구조](#4-프론트엔드-구조)
5. [Monaco Editor 통합](#5-monaco-editor-통합)
6. [예제 시스템](#6-예제-시스템)
7. [커스터마이징](#7-커스터마이징)
8. [배포 가이드](#8-배포-가이드)
9. [문제 해결](#9-문제-해결)

---

## 1. 개요

KingSejong 온라인 플레이그라운드는 사용자가 웹 브라우저에서 KingSejong 코드를 작성하고 실행할 수 있는 인터랙티브 환경입니다.

### 주요 기능

- ✅ **Monaco Editor** - VS Code와 동일한 편집 경험
- ✅ **WebAssembly** - 네이티브 속도로 코드 실행
- ✅ **실시간 출력** - 즉시 결과 확인
- ✅ **예제 갤러리** - 10개의 샘플 코드
- ✅ **Vite** - 빠른 개발 및 빌드

### 기술 스택

| 기술 | 역할 | 버전 |
|------|------|------|
| **Monaco Editor** | 코드 에디터 | 0.34+ |
| **WebAssembly** | 실행 엔진 | - |
| **Vite** | 빌드 도구 | 4.0+ |
| **Emscripten** | WASM 컴파일러 | 3.1+ |
| **Vanilla JS** | 프론트엔드 | ES6+ |

---

## 2. 아키텍처

### 2.1 전체 구조

```
┌─────────────────────────────────────────────┐
│              사용자 브라우저                  │
├─────────────────────────────────────────────┤
│  Monaco Editor          │   출력 영역        │
│  ┌─────────────────┐   │  ┌──────────────┐ │
│  │  KingSejong     │   │  │  실행 결과    │ │
│  │  코드 작성      │   │  │  에러 메시지  │ │
│  └─────────────────┘   │  └──────────────┘ │
│         │               │         ▲          │
│         ▼               │         │          │
│  ┌─────────────────────────────────┐        │
│  │  JavaScript Runtime (main.js)   │        │
│  └─────────────────────────────────┘        │
│                 │                            │
│                 ▼                            │
│  ┌─────────────────────────────────┐        │
│  │    WebAssembly Module            │        │
│  │  (KingSejong Interpreter)        │        │
│  └─────────────────────────────────┘        │
└─────────────────────────────────────────────┘
```

### 2.2 데이터 흐름

1. **사용자 입력**: Monaco Editor에서 코드 작성
2. **JavaScript → WASM**: main.js가 코드를 WASM 모듈로 전달
3. **WASM 실행**: C++ 인터프리터가 코드 파싱 및 실행
4. **결과 반환**: WASM → JavaScript 콜백으로 출력
5. **UI 업데이트**: 출력 영역에 결과 표시

### 2.3 디렉토리 구조

```
web/
├── public/                  # 정적 파일
│   ├── kingsejong.js       # WASM 래퍼 (빌드 후 생성)
│   └── kingsejong.wasm     # WASM 바이너리 (빌드 후 생성)
├── src/
│   ├── main.js             # 메인 애플리케이션 로직
│   ├── examples.js         # 예제 프로그램 모음
│   └── styles/
│       └── main.css        # 스타일시트
├── index.html              # HTML 템플릿
├── vite.config.js          # Vite 설정
├── package.json            # 의존성
└── README.md               # 사용 가이드
```

---

## 3. WebAssembly 빌드

### 3.1 Emscripten 설치

WebAssembly 빌드를 위해 Emscripten이 필요합니다:

```bash
# macOS / Linux
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

**Windows**:
```powershell
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
.\emsdk install latest
.\emsdk activate latest
.\emsdk_env.bat
```

### 3.2 WASM 빌드 스크립트

프로젝트에는 `scripts/build-wasm.sh` 스크립트가 제공됩니다:

```bash
#!/bin/bash
# scripts/build-wasm.sh

# Emscripten 환경 로드
source ~/emsdk/emsdk_env.sh

# 빌드 디렉토리 생성
mkdir -p build-wasm
cd build-wasm

# CMake 설정 (Emscripten 툴체인 사용)
emcmake cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_WASM=ON

# 빌드
emmake make -j8

# WASM 파일을 web/public/으로 복사
cp kingsejong.js kingsejong.wasm ../web/public/

echo "✅ WASM 빌드 완료!"
```

**실행**:
```bash
cd /path/to/kingsejonglang
./scripts/build-wasm.sh
```

### 3.3 CMake 설정

`CMakeLists.txt`에서 WASM 빌드 옵션 추가:

```cmake
# WASM 빌드 옵션
option(BUILD_WASM "Build for WebAssembly" OFF)

if(BUILD_WASM)
    # Emscripten 전용 설정
    set(CMAKE_EXECUTABLE_SUFFIX ".js")

    # WASM 링크 플래그
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} \
        -s WASM=1 \
        -s EXPORTED_FUNCTIONS='[\"_main\", \"_runCode\"]' \
        -s EXPORTED_RUNTIME_METHODS='[\"ccall\", \"cwrap\"]' \
        -s MODULARIZE=1 \
        -s EXPORT_NAME='KingsejongModule' \
        -s ALLOW_MEMORY_GROWTH=1")
endif()
```

### 3.4 C++ → JavaScript 인터페이스

WASM에서 JavaScript로 함수를 노출:

**src/wasm/wasm_interface.cpp**:
```cpp
#include <emscripten/emscripten.h>
#include <string>
#include "evaluator/Evaluator.h"
#include "parser/Parser.h"
#include "lexer/Lexer.h"

extern "C" {

// JavaScript에서 호출 가능한 함수
EMSCRIPTEN_KEEPALIVE
const char* runCode(const char* code) {
    try {
        std::string input(code);

        // Lexer → Parser → Evaluator
        kingsejong::Lexer lexer(input);
        auto tokens = lexer.tokenize();

        kingsejong::Parser parser(tokens);
        auto ast = parser.parse();

        kingsejong::Evaluator evaluator;
        auto result = evaluator.eval(ast);

        return result.toString().c_str();
    } catch (const std::exception& e) {
        return e.what();
    }
}

} // extern "C"
```

---

## 4. 프론트엔드 구조

### 4.1 main.js 핵심 로직

**src/main.js**:
```javascript
import * as monaco from 'monaco-editor';
import { examples } from './examples.js';

let editor;
let wasmModule;

// 1. Monaco Editor 초기화
async function initEditor() {
    editor = monaco.editor.create(document.getElementById('editor'), {
        value: examples[0].code,
        language: 'javascript', // 추후 'kingsejong'으로 변경
        theme: 'vs-dark',
        fontSize: 14,
        minimap: { enabled: false },
        automaticLayout: true,
    });

    // Ctrl+Enter로 코드 실행
    editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter, runCode);
}

// 2. WASM 모듈 로드
async function loadWasm() {
    wasmModule = await KingsejongModule({
        print: (text) => {
            appendOutput(text);
        },
        printErr: (text) => {
            appendOutput(`[ERROR] ${text}`, 'error');
        }
    });

    console.log('✅ WASM 모듈 로드 완료');
}

// 3. 코드 실행
function runCode() {
    const code = editor.getValue();

    try {
        // WASM 함수 호출
        const result = wasmModule.ccall(
            'runCode',           // C++ 함수 이름
            'string',            // 반환 타입
            ['string'],          // 매개변수 타입
            [code]               // 매개변수 값
        );

        appendOutput(result);
    } catch (error) {
        appendOutput(`실행 오류: ${error.message}`, 'error');
    }
}

// 4. 출력 영역 업데이트
function appendOutput(text, type = 'normal') {
    const output = document.getElementById('output');
    const line = document.createElement('div');
    line.className = type === 'error' ? 'error-line' : 'output-line';
    line.textContent = text;
    output.appendChild(line);
    output.scrollTop = output.scrollHeight;
}

// 5. 초기화
document.addEventListener('DOMContentLoaded', async () => {
    await loadWasm();
    await initEditor();

    // 버튼 이벤트 리스너
    document.getElementById('run-btn').addEventListener('click', runCode);
    document.getElementById('clear-btn').addEventListener('click', clearOutput);
    document.getElementById('reset-btn').addEventListener('click', resetInterpreter);
});
```

### 4.2 index.html

**index.html**:
```html
<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>KingSejong 플레이그라운드</title>
    <link rel="stylesheet" href="/src/styles/main.css">
</head>
<body>
    <div class="container">
        <!-- 헤더 -->
        <header>
            <h1>🇰🇷 KingSejong 플레이그라운드</h1>
            <p>한글로 프로그래밍하는 자연스러운 언어</p>
        </header>

        <!-- 메인 영역 -->
        <main>
            <!-- 에디터 영역 -->
            <div class="editor-section">
                <div class="toolbar">
                    <button id="run-btn" class="btn btn-primary">▶ 실행</button>
                    <button id="clear-btn" class="btn">🗑 지우기</button>
                    <button id="reset-btn" class="btn">🔄 초기화</button>
                </div>
                <div id="editor"></div>
            </div>

            <!-- 출력 영역 -->
            <div class="output-section">
                <h3>출력</h3>
                <div id="output" class="output-panel"></div>
            </div>

            <!-- 예제 갤러리 -->
            <aside class="examples-section">
                <h3>예제</h3>
                <div id="examples-list"></div>
            </aside>
        </main>
    </div>

    <!-- WASM 모듈 로드 -->
    <script src="/kingsejong.js"></script>
    <!-- 메인 스크립트 -->
    <script type="module" src="/src/main.js"></script>
</body>
</html>
```

### 4.3 스타일링

**src/styles/main.css**:
```css
:root {
    --primary-color: #4CAF50;
    --bg-primary: #1e1e1e;
    --bg-secondary: #252526;
    --text-primary: #d4d4d4;
    --text-secondary: #858585;
    --border-color: #3e3e42;
}

body {
    margin: 0;
    padding: 0;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    background: var(--bg-primary);
    color: var(--text-primary);
}

.container {
    max-width: 1400px;
    margin: 0 auto;
    padding: 20px;
}

/* 에디터 영역 */
.editor-section {
    display: flex;
    flex-direction: column;
    height: 500px;
    border: 1px solid var(--border-color);
    border-radius: 4px;
}

#editor {
    flex: 1;
}

/* 출력 영역 */
.output-panel {
    background: #0d0d0d;
    color: #00ff00;
    font-family: 'Courier New', monospace;
    padding: 10px;
    height: 300px;
    overflow-y: auto;
    border: 1px solid var(--border-color);
    border-radius: 4px;
}

.output-line {
    margin: 2px 0;
}

.error-line {
    color: #ff6b6b;
}

/* 버튼 */
.btn {
    padding: 8px 16px;
    border: none;
    border-radius: 4px;
    cursor: pointer;
    font-size: 14px;
    margin-right: 8px;
    background: var(--bg-secondary);
    color: var(--text-primary);
}

.btn-primary {
    background: var(--primary-color);
    color: white;
}

.btn:hover {
    opacity: 0.8;
}
```

---

## 5. Monaco Editor 통합

### 5.1 언어 정의 추가

KingSejong 언어를 Monaco에 등록:

```javascript
// src/main.js

// 1. 언어 등록
monaco.languages.register({ id: 'kingsejong' });

// 2. 문법 정의 (Monarch)
monaco.languages.setMonarchTokensProvider('kingsejong', {
    keywords: [
        '변수', '정수', '실수', '문자열', '논리', '상수',
        '만약', '아니면', '아니면만약',
        '반복한다', '번', '부터', '까지', '미만', '초과', '이하', '이상',
        '함수', '반환',
        '출력', '입력',
        '참', '거짓', 'null',
        '가져오기', '클래스', '생성자', '메서드'
    ],

    operators: [
        '+', '-', '*', '/', '%',
        '=', '==', '!=', '<', '>', '<=', '>=',
        '그리고', '또는', '아니다'
    ],

    tokenizer: {
        root: [
            // 키워드
            [/[가-힣]+/, {
                cases: {
                    '@keywords': 'keyword',
                    '@default': 'identifier'
                }
            }],

            // 숫자
            [/\d+\.\d+/, 'number.float'],
            [/\d+/, 'number'],

            // 문자열
            [/"([^"\\]|\\.)*$/, 'string.invalid'],
            [/"/, 'string', '@string'],

            // 주석
            [/#.*$/, 'comment'],

            // 연산자
            [/[+\-*/%=<>!]/, 'operator'],
        ],

        string: [
            [/[^\\"]+/, 'string'],
            [/\\./, 'string.escape'],
            [/"/, 'string', '@pop']
        ],
    }
});

// 3. 테마 설정
monaco.editor.defineTheme('kingsejong-dark', {
    base: 'vs-dark',
    inherit: true,
    rules: [
        { token: 'keyword', foreground: '569CD6', fontStyle: 'bold' },
        { token: 'number', foreground: 'B5CEA8' },
        { token: 'string', foreground: 'CE9178' },
        { token: 'comment', foreground: '6A9955', fontStyle: 'italic' },
        { token: 'operator', foreground: 'D4D4D4' },
    ],
    colors: {
        'editor.background': '#1e1e1e',
    }
});

// 4. 에디터 생성 시 적용
editor = monaco.editor.create(document.getElementById('editor'), {
    value: examples[0].code,
    language: 'kingsejong',  // 여기!
    theme: 'kingsejong-dark',
    fontSize: 14,
});
```

### 5.2 자동 완성

```javascript
monaco.languages.registerCompletionItemProvider('kingsejong', {
    provideCompletionItems: (model, position) => {
        const suggestions = [
            {
                label: '출력',
                kind: monaco.languages.CompletionItemKind.Function,
                insertText: '출력($1)',
                insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: '값을 출력합니다'
            },
            {
                label: '만약',
                kind: monaco.languages.CompletionItemKind.Keyword,
                insertText: '만약 (${1:조건}) {\n\t$2\n}',
                insertTextRules: monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet,
                documentation: '조건문'
            },
            // ... 더 많은 제안
        ];

        return { suggestions };
    }
});
```

---

## 6. 예제 시스템

### 6.1 예제 정의

**src/examples.js**:
```javascript
export const examples = [
    {
        title: 'Hello, World!',
        description: '첫 번째 프로그램',
        code: `출력("안녕, KingSejong!")
출력("한글로 프로그래밍해요!")`
    },

    {
        title: '변수와 연산',
        description: '변수 선언 및 산술 연산',
        code: `정수 나이 = 25
실수 키 = 175.5
문자열 이름 = "홍길동"

출력("이름:", 이름)
출력("나이:", 나이)
출력("키:", 키)

# 연산
합 = 10 + 20
곱 = 5 * 6
출력("합:", 합)
출력("곱:", 곱)`
    },

    {
        title: '조건문',
        description: 'if-else 분기',
        code: `점수 = 85

만약 (점수 >= 90) {
    출력("A 학점")
} 아니면만약 (점수 >= 80) {
    출력("B 학점")
} 아니면만약 (점수 >= 70) {
    출력("C 학점")
} 아니면 {
    출력("F 학점")
}`
    },

    {
        title: '반복문',
        description: '다양한 반복 구문',
        code: `# N번 반복
5번 반복한다 {
    출력("안녕!")
}

# 범위 반복
i가 1부터 5까지 {
    출력("i =", i)
}

# 배열 반복
숫자들 = [10, 20, 30, 40, 50]
숫자들을 각각(함수(n) {
    출력("숫자:", n)
})`
    },

    {
        title: '함수',
        description: '함수 정의 및 호출',
        code: `# 함수 정의
함수 더하기(a, b) {
    반환 a + b
}

함수 곱하기(x, y) {
    반환 x * y
}

# 함수 호출
결과1 = 더하기(10, 20)
결과2 = 곱하기(5, 6)

출력("10 + 20 =", 결과1)
출력("5 * 6 =", 결과2)`
    },

    {
        title: '팩토리얼 (재귀)',
        description: '재귀 함수 예제',
        code: `함수 팩토리얼(n) {
    만약 (n <= 1) {
        반환 1
    }
    반환 n * 팩토리얼(n - 1)
}

i가 1부터 10까지 {
    출력(i, "! =", 팩토리얼(i))
}`
    },

    {
        title: '피보나치 수열',
        description: '재귀 피보나치',
        code: `함수 피보나치(n) {
    만약 (n <= 1) {
        반환 n
    }
    반환 피보나치(n-1) + 피보나치(n-2)
}

출력("=== 피보나치 수열 ===")
i가 0부터 10 미만 {
    출력("F(", i, ") =", 피보나치(i))
}`
    },

    {
        title: '배열 메서드',
        description: '함수형 배열 메서드',
        code: `숫자들 = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

# filter - 짝수만
짝수들 = 숫자들을 걸러낸다(함수(n) {
    반환 n % 2 == 0
})
출력("짝수:", 짝수들)

# map - 제곱
제곱들 = 짝수들을 변환한다(함수(n) {
    반환 n * n
})
출력("제곱:", 제곱들)

# reduce - 합계
합계 = 제곱들을 축약한다(0, 함수(누적, 현재) {
    반환 누적 + 현재
})
출력("합계:", 합계)`
    },

    {
        title: '클로저',
        description: '클로저와 스코프',
        code: `# 카운터 생성 함수
함수 카운터_생성() {
    개수 = 0

    반환 함수() {
        개수 = 개수 + 1
        반환 개수
    }
}

# 각각 독립적인 카운터
카운터1 = 카운터_생성()
카운터2 = 카운터_생성()

출력("카운터1:", 카운터1())  # 1
출력("카운터1:", 카운터1())  # 2
출력("카운터1:", 카운터1())  # 3

출력("카운터2:", 카운터2())  # 1
출력("카운터2:", 카운터2())  # 2`
    },

    {
        title: 'stdlib 활용',
        description: '표준 라이브러리 사용',
        code: `가져오기 "stdlib/math"
가져오기 "stdlib/array"

# 수학 함수
출력("거듭제곱(2, 10) =", 거듭제곱(2, 10))
출력("계승(5) =", 계승(5))
출력("제곱근(16) =", 제곱근(16))
출력("소수인가(17) =", 소수인가(17))

# 배열 함수
숫자들 = 범위(1, 10)
출력("범위(1, 10) =", 숫자들)

중복 = [1, 2, 2, 3, 3, 4]
고유 = 중복_제거(중복)
출력("중복_제거 =", 고유)`
    },
];
```

### 6.2 예제 UI 생성

```javascript
// 예제 목록 렌더링
function renderExamples() {
    const list = document.getElementById('examples-list');

    examples.forEach((example, index) => {
        const item = document.createElement('div');
        item.className = 'example-item';
        item.innerHTML = `
            <h4>${example.title}</h4>
            <p>${example.description}</p>
        `;

        item.addEventListener('click', () => {
            editor.setValue(example.code);
            clearOutput();
        });

        list.appendChild(item);
    });
}
```

---

## 7. 커스터마이징

### 7.1 테마 변경

다크 모드 / 라이트 모드 토글:

```javascript
let isDarkMode = true;

function toggleTheme() {
    isDarkMode = !isDarkMode;

    if (isDarkMode) {
        monaco.editor.setTheme('kingsejong-dark');
        document.body.classList.add('dark');
    } else {
        monaco.editor.setTheme('vs');
        document.body.classList.remove('dark');
    }
}
```

### 7.2 폰트 설정

```javascript
editor.updateOptions({
    fontSize: 16,
    fontFamily: '"D2Coding", "Fira Code", monospace',
    fontLigatures: true,
});
```

### 7.3 키바인딩 추가

```javascript
// Ctrl+S: 코드 저장 (로컬스토리지)
editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS, () => {
    const code = editor.getValue();
    localStorage.setItem('kingsejong_code', code);
    alert('코드가 저장되었습니다!');
});

// Ctrl+L: 코드 불러오기
editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyL, () => {
    const code = localStorage.getItem('kingsejong_code');
    if (code) {
        editor.setValue(code);
    }
});
```

---

## 8. 배포 가이드

### 8.1 GitHub Pages

**1단계: 빌드**
```bash
cd web
npm run build
```

**2단계: gh-pages 브랜치 생성**
```bash
git checkout -b gh-pages
cp -r dist/* .
git add .
git commit -m "Deploy to GitHub Pages"
git push origin gh-pages
```

**3단계: GitHub 설정**
- 저장소 → Settings → Pages
- Source: `gh-pages` 브랜치 선택
- Save

**접속**: `https://username.github.io/kingsejonglang/`

### 8.2 Vercel

**1단계: Vercel CLI 설치**
```bash
npm install -g vercel
```

**2단계: 배포**
```bash
cd web
vercel
```

**설정**:
- Project name: `kingsejong-playground`
- Build command: `npm run build`
- Output directory: `dist`

### 8.3 Netlify

**1단계: netlify.toml 생성**
```toml
# web/netlify.toml
[build]
  command = "npm run build"
  publish = "dist"

[[redirects]]
  from = "/*"
  to = "/index.html"
  status = 200
```

**2단계: 배포**
```bash
npm install -g netlify-cli
netlify deploy --prod
```

---

## 9. 문제 해결

### Q1: WASM 모듈을 로드할 수 없습니다

**증상**:
```
Failed to fetch kingsejong.wasm
```

**원인**: WASM 파일이 public/ 디렉토리에 없음

**해결**:
```bash
cd ..  # 프로젝트 루트로
./scripts/build-wasm.sh
```

---

### Q2: Monaco Editor가 표시되지 않습니다

**증상**: 빈 화면만 보임

**원인**: 의존성 미설치 또는 Vite 오류

**해결**:
```bash
rm -rf node_modules package-lock.json
npm install
npm run dev
```

---

### Q3: 한글이 깨집니다

**원인**: UTF-8 인코딩 문제

**해결**:
```html
<!-- index.html -->
<meta charset="UTF-8">
```

---

### Q4: 코드 실행이 느립니다

**원인**: WASM 최적화 부족

**해결**:
```bash
# Release 모드로 빌드
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release -O3
```

---

### Q5: 여러 번 실행하면 오류가 발생합니다

**원인**: 인터프리터 상태 관리 문제

**해결**: 초기화 버튼으로 환경 리셋
```javascript
function resetInterpreter() {
    // WASM 모듈 재로드
    loadWasm();
    clearOutput();
}
```

---

## 10. 고급 기능

### 10.1 코드 공유 (URL 인코딩)

```javascript
function shareCode() {
    const code = editor.getValue();
    const encoded = btoa(encodeURIComponent(code));
    const url = `${window.location.origin}?code=${encoded}`;

    navigator.clipboard.writeText(url);
    alert('URL이 복사되었습니다!');
}

// 페이지 로드 시 URL에서 코드 복원
const params = new URLSearchParams(window.location.search);
if (params.has('code')) {
    const code = decodeURIComponent(atob(params.get('code')));
    editor.setValue(code);
}
```

### 10.2 실행 시간 측정

```javascript
function runCode() {
    const code = editor.getValue();

    const startTime = performance.now();

    try {
        const result = wasmModule.ccall('runCode', 'string', ['string'], [code]);
        appendOutput(result);
    } catch (error) {
        appendOutput(`오류: ${error.message}`, 'error');
    }

    const endTime = performance.now();
    const elapsed = (endTime - startTime).toFixed(2);

    appendOutput(`\n실행 시간: ${elapsed}ms`, 'info');
}
```

### 10.3 에러 위치 하이라이팅

```javascript
function highlightError(line, column) {
    const decorations = editor.deltaDecorations([], [
        {
            range: new monaco.Range(line, 1, line, 1),
            options: {
                isWholeLine: true,
                className: 'error-line-decoration',
                glyphMarginClassName: 'error-glyph'
            }
        }
    ]);
}
```

---

## 11. 참고 자료

### 공식 문서
- [Monaco Editor](https://microsoft.github.io/monaco-editor/)
- [Emscripten](https://emscripten.org/docs/)
- [Vite](https://vitejs.dev/)

### 관련 프로젝트
- [TypeScript Playground](https://www.typescriptlang.org/play)
- [Rust Playground](https://play.rust-lang.org/)
- [Go Playground](https://go.dev/play/)

---

**작성자**: KingSejong Team
**최종 수정**: 2025-11-21
**피드백**: https://github.com/anthropics/kingsejong-lang/issues
