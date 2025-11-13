# VS Code Extension 테스트 가이드

## 고급 LSP 기능 테스트

이 문서는 KingSejong VS Code Extension의 고급 LSP 기능을 테스트하는 방법을 설명합니다.

## 지원하는 기능

### 기본 기능
- ✅ 구문 강조 (Syntax Highlighting)
- ✅ 자동 완성 (Auto Completion) - 34개 키워드
- ✅ 실시간 진단 (Diagnostics) - 구문 오류 표시

### 고급 기능
- ✅ **Go to Definition (F12)** - 변수/함수 정의로 이동
- ✅ **Hover Information** - 타입 및 시그니처 정보 표시
- ✅ **Find References (Shift+F12)** - 심볼 사용처 찾기
- ✅ **Rename (F2)** - 심볼 이름 변경
- ✅ **스코프 인식** - 전역/로컬 변수 구분
- ✅ **한글 문자 지원** - UTF-16/UTF-8 자동 변환

---

## 사전 준비

### 1. 프로젝트 빌드

```bash
cd /path/to/kingsejonglang
mkdir build && cd build
cmake ..
make -j8
```

빌드가 완료되면 `build/bin/kingsejong` 실행 파일이 생성됩니다.

### 2. Extension 컴파일

```bash
cd vscode-extension
npm install
npm run compile
```

---

## Extension Development Host에서 테스트

### 1. VS Code에서 Extension 폴더 열기

```bash
cd vscode-extension
code .
```

### 2. Extension Development Host 실행

1. VS Code에서 `F5` 키를 누릅니다
2. 새 창(Extension Development Host)이 열립니다
3. 이 창에서 KingSejong Extension이 활성화됩니다

### 3. 테스트 파일 열기

Extension Development Host 창에서:

```bash
File > Open Folder...
# kingsejonglang 프로젝트 폴더 선택

# 테스트 파일 열기
test_lsp_features.ksj
```

---

## 기능별 테스트 방법

### 1. Go to Definition (F12)

**테스트 시나리오:**

```ksj
# 파일: test_lsp_features.ksj
정수 x = 10        # ← 정의 위치 (line 7)
정수 y = x + 5     # ← 여기서 x 위에 커서를 놓고 F12
```

**예상 결과:**
- F12를 누르면 → line 7의 `정수 x = 10`으로 이동
- 함수도 동일하게 동작

**테스트 케이스:**
1. 변수 `x` 위에서 F12 → 정의로 이동
2. 함수 `더하기` 호출 위치에서 F12 → 함수 정의로 이동
3. 매개변수 `a` 사용 위치에서 F12 → 매개변수 선언으로 이동

---

### 2. Hover Information

**테스트 시나리오:**

```ksj
함수 더하기(a, b) {    # ← 함수 정의
    반환 a + b
}

정수 결과 = 더하기(5, 3)  # ← 여기서 "더하기" 위에 마우스 올리기
```

**예상 결과:**
- Hover 팝업 표시:
  ```
  함수 더하기(a, b)
  ```

**테스트 케이스:**
1. 변수 위에 마우스 → `정수 x` 표시
2. 함수 위에 마우스 → `함수 더하기(a, b)` 표시
3. 매개변수 위에 마우스 → 타입 정보 표시

---

### 3. Find References (Shift+F12)

**테스트 시나리오:**

```ksj
정수 count = 0     # ← 전역 count 정의

함수 증가() {
    정수 count = 1  # ← 로컬 count 정의
    반환 count + 1  # ← 로컬 count 참조
}

정수 결과 = count + 1  # ← 전역 count 참조
```

**예상 결과:**

1. **전역 `count`에서 Shift+F12:**
   - Line 1 (정의)
   - Line 10 (참조)

2. **로컬 `count`에서 Shift+F12:**
   - Line 4 (정의)
   - Line 5 (참조)

**테스트 케이스:**
1. 전역 변수의 모든 참조 찾기
2. 로컬 변수의 참조 찾기 (전역과 구분)
3. 함수 호출 위치 모두 찾기

---

### 4. Rename (F2)

**테스트 시나리오:**

```ksj
정수 x = 10        # ← x 위에서 F2
정수 y = x + 5     # ← 자동으로 변경됨
정수 z = x * 2     # ← 자동으로 변경됨
```

**예상 동작:**
1. `x` 위에서 F2 누르기
2. 새 이름 입력 (예: `myNumber`)
3. Enter 누르기
4. 모든 `x` → `myNumber`로 변경

**테스트 케이스:**
1. 전역 변수 이름 변경 → 모든 참조 업데이트
2. 로컬 변수 이름 변경 → 로컬 스코프만 업데이트
3. 함수 이름 변경 → 정의와 모든 호출 업데이트

---

### 5. 스코프 구분 테스트

**테스트 시나리오:**

```ksj
정수 x = 10        # 전역 x

함수 테스트() {
    정수 x = 20    # 로컬 x (전역 x와 별개)
    반환 x         # 로컬 x 참조
}

정수 결과 = x + 1  # 전역 x 참조
```

**예상 결과:**
- 전역 `x`에서 Find References → line 1, line 9만 표시
- 로컬 `x`에서 Find References → line 5, line 6만 표시
- **전역과 로컬이 섞이지 않음!**

---

### 6. 한글 문자 위치 테스트

**테스트 시나리오:**

```ksj
정수 숫자 = 10     # ← "숫자" 위에서 F12
정수 결과 = 숫자 + 5
```

**예상 결과:**
- UTF-16/UTF-8 변환이 정확하게 동작
- 한글 변수명도 정확하게 인식

---

## 디버깅 팁

### LSP 로그 확인

1. VS Code 설정 열기: `Cmd+,` (macOS) 또는 `Ctrl+,` (Windows/Linux)
2. 검색: `kingsejong.trace.server`
3. 값을 `verbose`로 변경
4. Output 패널에서 "KingSejong Language Server" 선택

### 서버 재시작

Extension Development Host에서:
1. `Cmd+Shift+P` (macOS) 또는 `Ctrl+Shift+P` (Windows/Linux)
2. 입력: `Reload Window`

---

## 예상 문제 및 해결

### 1. "KingSejong language server not found"

**원인**: 빌드된 실행 파일을 찾을 수 없음

**해결:**
```bash
# 빌드 확인
ls -la build/bin/kingsejong

# Extension이 올바른 경로를 찾는지 확인
# extension.ts의 findServerExecutable() 로직 참조
```

### 2. 고급 기능이 동작하지 않음

**원인**: 서버가 capabilities를 올바르게 보고하지 않음

**확인:**
```bash
# 서버 capabilities 테스트 실행
cd build
./bin/kingsejong_tests --gtest_filter="ServerCapabilitiesTest.*"
```

**예상 출력:**
```
[ PASSED ] 3 tests.
```

### 3. 한글 문자 위치가 틀림

**원인**: UTF-16/UTF-8 변환 오류

**확인:**
```bash
# LspUtils 테스트 실행
./bin/kingsejong_tests --gtest_filter="LspUtilsTest.*"
```

---

## 성공 기준

### ✅ 모든 기능이 정상 동작하면:

1. **Go to Definition**: 변수/함수 정의로 정확하게 이동
2. **Hover**: 타입/시그니처 정보 올바르게 표시
3. **Find References**: 스코프 구분 정확 (전역/로컬)
4. **Rename**: 스코프 내 모든 심볼 일괄 변경
5. **한글 지원**: 한글 변수명 정확하게 인식

### ✅ 테스트 통과:

```bash
cd build
./bin/kingsejong_tests

# 예상 출력:
# [==========] 806 tests from 52 test suites ran.
# [  PASSED  ] 806 tests.
```

---

## 다음 단계

고급 LSP 기능이 모두 동작하면:
1. ✅ VS Code Marketplace에 Extension 게시 준비
2. ✅ 사용자 피드백 수집
3. ✅ 추가 기능 개발 (Document Symbols, Code Actions 등)

---

**작성일**: 2025-11-14
**버전**: 0.3.2
**테스트 현황**: 806/806 통과
