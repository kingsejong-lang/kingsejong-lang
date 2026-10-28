# KingSejong Language Support for VS Code

한글 기반 프로그래밍 언어 KingSejong을 위한 VS Code Extension입니다.

## 기능

### ✅ 구문 강조 (Syntax Highlighting)
- 키워드: `정수`, `실수`, `문자열`, `논리`, `함수`, `만약`, `아니면`, `반환`, `반복한다` 등
- 조사: `을`, `를`, `이`, `가`, `은`, `는` 등
- 리터럴: 숫자, 문자열, 부울 값
- 연산자 및 괄호

### ✅ 자동 완성 (Auto Completion)
- 키워드 자동 완성
- 변수명 자동 완성
- 함수 파라미터 힌트

### ✅ 실시간 진단 (Diagnostics)
- 구문 오류 감지
- 타입 오류 표시
- 실시간 에러 밑줄 표시

### ✅ 편집 기능
- 자동 괄호 닫기
- 들여쓰기 자동 조정
- 주석 토글 (향후 지원 예정)

### ✅ 코드 포맷팅 (NEW!)
- `kingsejong fmt` 명령어를 통한 자동 코드 포맷팅
- 편집기 설정과 프로젝트의 `.ksjfmt.json` 을 통한 포맷팅 설정
- `Format Document` (Shift+Alt+F) 단축키 지원
- 저장 시 자동 포맷팅 지원

## 설치

### 개발 환경에서 실행

1. KingSejong 언어 서버 빌드:
```bash
cd ..
mkdir -p build && cd build
cmake ..
make -j8
```

2. Extension 의존성 설치 (Node.js 26 LTS 권장, 최소 22.12. 저장소 루트의 `.nvmrc` 가 CI 와 같은 버전을 가리킨다):
```bash
cd vscode-extension
npm install
```

3. Extension 번들(`dist/extension.js`):
```bash
npm run bundle
```

4. VS Code에서 실행:
- VS Code에서 `vscode-extension` 디렉토리 열기
- F5를 눌러 Extension Development Host 실행
- `.ksj` 파일 열어서 테스트

### `.vsix` 로 설치

```bash
cd vscode-extension
npm ci
npm test                    # 포맷터 호출 테스트 (../build/bin/kingsejong 이 필요하다)
npm run package             # kingsejong-<버전>.vsix 생성
code --install-extension kingsejong-*.vsix
```

`.vsix` 에는 `kingsejong` 실행 파일이 들어 있지 않습니다. 확장은 `server/kingsejong`, `../build/bin/kingsejong`, `PATH` 순서로 찾으므로 `kingsejong` 을 빌드해 `PATH` 에 두세요. CI 는 main 의 커밋마다 `.vsix` 를 아티팩트(`kingsejong-vscode`)로 남깁니다.

## 사용법

### KingSejong 파일 생성

1. 새 파일 생성: `test.ksj`
2. 다음 코드 입력:

```kingsejong
정수 x = 10
정수 y = 20

함수 더하기(a, b) {
    반환 a + b
}

정수 결과 = 더하기(x, y)
출력(결과)
```

3. 구문 강조, 자동 완성, 에러 검사 등이 자동으로 작동합니다.

## 설정

### `settings.json`에서 설정 가능한 옵션:

```json
{
  "kingsejong.trace.server": "verbose",
  "kingsejong.formatter.enable": true,
  "kingsejong.formatter.indentSize": 4,
  "kingsejong.formatter.useSpaces": true,
  "kingsejong.formatter.maxLineLength": 100
}
```

#### LSP 설정
- `kingsejong.trace.server`: LSP 서버 통신 로그 레벨 (`off`, `messages`, `verbose`)

#### Formatter 설정
- `kingsejong.formatter.enable`: 포맷터 활성화 (기본값: `true`)
- `kingsejong.formatter.indentSize`: 들여쓰기 크기 (기본값: `4`)
- `kingsejong.formatter.useSpaces`: 탭 대신 스페이스 사용 (기본값: `true`)
- `kingsejong.formatter.maxLineLength`: 최대 줄 길이 (기본값: `100`)

파일 끝 줄바꿈과 줄 끝 공백은 VS Code 의 `files.insertFinalNewline`, `files.trimTrailingWhitespace` 를 쓰세요.

### `.ksjfmt.json` 파일

프로젝트 루트(작업 공간 폴더)에 `.ksjfmt.json` 을 두면 위 편집기 설정보다 우선합니다. `kingsejong fmt` 명령이 읽는 파일과 같으므로 저장소에 넣어 팀이 같은 설정을 쓸 수 있습니다:

```json
{
  "indentSize": 2,
  "useSpaces": true,
  "maxLineLength": 120,
  "spaceAroundOperators": true,
  "spaceAfterComma": true,
  "spaceBeforeBrace": true
}
```

## 문제 해결

### Extension이 활성화되지 않는 경우

1. LSP 서버가 빌드되었는지 확인:
```bash
ls -la ../build/bin/kingsejong
```

2. Output 패널에서 "KingSejong Language Server" 로그 확인

3. Extension 재시작: `Developer: Reload Window`

### 자동 완성이 작동하지 않는 경우

- 파일 확장자가 `.ksj`인지 확인
- 언어 모드가 "KingSejong"으로 설정되었는지 확인 (우측 하단)

## 개발

### 디렉토리 구조

```
vscode-extension/
├── package.json              # Extension 메타데이터
├── tsconfig.json             # TypeScript 설정
├── language-configuration.json  # 언어 편집 설정
├── src/
│   └── extension.ts          # Extension 진입점
└── syntaxes/
    └── kingsejong.tmLanguage.json  # 구문 강조 정의
```

### 빌드 및 디버깅

```bash
# 번들 (dist/extension.js)
npm run bundle

# Watch 모드 (자동 재번들)
npm run watch

# 린트
npm run lint
```

### 테스트

F5를 눌러 Extension Development Host를 실행하고:

1. `test.ksj` 파일 생성
2. 구문 강조 확인
3. 의도적으로 구문 오류 입력 (예: `정수 = 10`)
4. 빨간 밑줄과 에러 메시지 확인
5. 변수명 입력 시 자동 완성 확인

## 라이선스

MIT License

## 기여

버그 리포트 및 기능 제안은 GitHub Issues에서 환영합니다!
