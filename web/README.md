# KingSejong 온라인 플레이그라운드

KingSejong 언어를 웹 브라우저에서 실행할 수 있는 온라인 플레이그라운드입니다.

## 🚀 빠른 시작

### 1. 의존성 설치

```bash
cd web
npm install
```

### 2. WASM 모듈 빌드

플레이그라운드를 실행하기 전에 WebAssembly 모듈을 빌드해야 합니다:

```bash
# 프로젝트 루트에서
./scripts/build-wasm.sh
```

빌드된 WASM 파일(`kingsejong.js`, `kingsejong.wasm`)은 자동으로 `web/public/` 디렉토리로 복사됩니다.

### 3. 개발 서버 실행

```bash
npm run dev
```

브라우저에서 `http://localhost:3000` 으로 자동 접속됩니다.

## 📦 빌드

프로덕션 빌드:

```bash
npm run build
```

빌드된 파일은 `dist/` 디렉토리에 생성됩니다.

빌드 결과 미리보기:

```bash
npm run preview
```

## 🛠 기술 스택

- **Vite**: 빠른 개발 서버 및 빌드 도구
- **Monaco Editor**: VS Code와 동일한 에디터
- **WebAssembly**: 브라우저에서 네이티브 속도로 실행
- **Vanilla JavaScript**: 프레임워크 없이 가볍게

## 📁 프로젝트 구조

```
web/
├── public/              # 정적 파일
│   ├── kingsejong.js    # WASM 래퍼 (빌드 후 생성)
│   └── kingsejong.wasm  # WASM 바이너리 (빌드 후 생성)
├── src/
│   ├── main.js          # 메인 애플리케이션 로직
│   ├── examples.js      # 예제 프로그램 모음
│   └── styles/
│       └── main.css     # 스타일시트
├── index.html           # HTML 템플릿
├── vite.config.js       # Vite 설정
├── package.json
└── README.md
```

## ✨ 주요 기능

### 1. 코드 편집

- **Monaco Editor** 통합 (VS Code와 동일한 편집 경험)
- 구문 강조 (추후 KingSejong 언어 정의 추가 예정)
- 자동 완성 (추후 지원 예정)
- 단축키 지원:
  - `Ctrl + Enter`: 코드 실행
  - `Ctrl + /`: 주석 토글

### 2. 실행 및 출력

- 즉시 코드 실행
- 실시간 출력 표시
- 에러 메시지 표시
- 여러 번 실행 가능 (출력 누적)

### 3. 예제 갤러리

- 10개의 예제 프로그램 제공:
  - Hello World
  - 변수와 연산
  - 조건문
  - 반복문
  - 함수
  - 재귀 (팩토리얼, 피보나치)
  - 배열
  - 클로저

### 4. 인터프리터 관리

- **실행**: 코드 실행
- **지우기**: 출력 영역 초기화
- **초기화**: 인터프리터 환경 리셋 (모든 변수 삭제)

## 🎨 커스터마이징

### 예제 추가

`src/examples.js` 파일에 새 예제를 추가할 수 있습니다:

```javascript
export const examples = [
    // ... 기존 예제들
    {
        title: '새 예제 제목',
        description: '예제 설명',
        code: `출력("예제 코드")`
    },
];
```

### 스타일 변경

`src/styles/main.css` 파일에서 색상, 폰트, 레이아웃 등을 수정할 수 있습니다.

CSS 변수로 주요 색상을 쉽게 변경:

```css
:root {
    --primary-color: #4CAF50;  /* 주요 색상 */
    --bg-primary: #ffffff;      /* 배경 색상 */
    /* ... */
}
```

### KingSejong 언어 정의 (Monaco)

Monaco Editor에 KingSejong 언어 정의를 추가하여 더 나은 구문 강조를 제공할 수 있습니다.

`src/main.js`에서 다음과 같이 추가:

```javascript
monaco.languages.register({ id: 'kingsejong' });

monaco.languages.setMonarchTokensProvider('kingsejong', {
    keywords: ['변수', '정수', '실수', '문자열', '만약', '아니면', '반복한다', '함수', '반환', '출력'],
    operators: ['+', '-', '*', '/', '%', '=', '==', '!=', '<', '>', '<=', '>='],
    // ...
});
```

## 🌐 배포

### GitHub Pages

1. 빌드:
   ```bash
   npm run build
   ```

2. `dist/` 디렉토리를 GitHub Pages에 배포

3. GitHub 저장소 설정에서 Pages 활성화

### Vercel / Netlify

1. GitHub 저장소 연결

2. 빌드 설정:
   - Build command: `npm run build`
   - Output directory: `dist`
   - Install command: `npm install`

3. 자동 배포

## 📝 할 일

- [ ] KingSejong 언어 정의 추가 (Monaco)
- [ ] 자동 완성 기능
- [ ] 코드 공유 기능 (URL 인코딩)
- [ ] 다크 모드 / 라이트 모드 토글
- [ ] 더 많은 예제 추가
- [ ] 에러 위치 하이라이팅
- [ ] 실행 시간 측정
- [ ] 메모리 사용량 표시

## 🐛 문제 해결

### WASM 모듈을 로드할 수 없습니다

**원인**: WASM 파일이 `public/` 디렉토리에 없습니다.

**해결**:
```bash
# WASM 다시 빌드
cd ..
./scripts/build-wasm.sh
```

### Monaco Editor가 표시되지 않습니다

**원인**: 의존성이 설치되지 않았거나 Vite가 제대로 실행되지 않았습니다.

**해결**:
```bash
rm -rf node_modules package-lock.json
npm install
npm run dev
```

## 📄 라이선스

MIT License

---

**작성일**: 2025-11-14
**버전**: v0.1.0
**작성자**: KingSejong Team
