# KingSejong 설정 파일 가이드

> **버전**: v0.5.0
> **최종 업데이트**: 2025-11-21
> **대상**: Linter 및 Formatter 설정 방법

---

## 개요

KingSejong 언어는 코드 품질 향상을 위한 **Linter**와 **Formatter**를 제공합니다.
프로젝트 루트에 설정 파일을 배치하여 팀 전체의 코딩 스타일을 통일할 수 있습니다.

---

## 1. Linter 설정 (`.ksjlintrc`)

### 1.1 기본 설정

프로젝트 루트에 `.ksjlintrc` 파일을 생성하세요:

```json
{
  "rules": {
    "unused-variable": "warning",
    "dead-code": "warning",
    "no-self-comparison": "error",
    "constant-condition": "warning",
    "empty-block": "warning",
    "no-magic-number": "hint",
    "no-shadowing": "warning",
    "no-unused-parameter": "hint"
  }
}
```

### 1.2 규칙 심각도 레벨

각 규칙에 다음 심각도를 설정할 수 있습니다:

| 심각도 | 설명 | 동작 |
|--------|------|------|
| `"off"` | 비활성화 | 규칙 적용 안 함 |
| `"hint"` | 힌트 | 개선 제안만 표시 |
| `"info"` | 정보 | 정보성 메시지 표시 |
| `"warning"` | 경고 | 경고 메시지 표시 |
| `"error"` | 에러 | 에러로 처리 (빌드 실패 가능) |

### 1.3 사용 가능한 규칙

#### 1.3.1 `unused-variable` (미사용 변수)

선언되었지만 사용되지 않은 변수를 탐지합니다.

```ksj
# 경고 발생
정수 x = 10  # 'x'가 사용되지 않음
```

**권장 설정**: `"warning"`

#### 1.3.2 `dead-code` (도달 불가 코드)

실행될 수 없는 코드를 탐지합니다.

```ksj
함수 예제() {
    반환 10
    출력("이 코드는 실행되지 않습니다")  # 경고: 도달 불가 코드
}
```

**권장 설정**: `"warning"`

#### 1.3.3 `no-self-comparison` (자기 비교)

변수를 자기 자신과 비교하는 코드를 탐지합니다.

```ksj
정수 x = 5
만약 (x == x) {  # 에러: 자기 자신과 비교
    출력("항상 참입니다")
}
```

**권장 설정**: `"error"`

#### 1.3.4 `constant-condition` (상수 조건)

항상 참 또는 거짓인 조건문을 탐지합니다.

```ksj
만약 (참) {  # 경고: 항상 참인 조건
    출력("항상 실행됩니다")
}
```

**권장 설정**: `"warning"`

#### 1.3.5 `empty-block` (빈 블록)

비어있는 코드 블록을 탐지합니다.

```ksj
만약 (x > 0) {
    # 경고: 빈 블록
}
```

**권장 설정**: `"warning"`

#### 1.3.6 `no-magic-number` (매직 넘버)

의미 없는 숫자 리터럴을 탐지합니다.

```ksj
# 힌트 발생
정수 넓이 = 너비 * 3.14159  # 3.14159는 상수로 정의하는 것이 좋습니다

# 개선된 코드
정수 파이 = 3.14159
정수 넓이 = 너비 * 파이
```

**권장 설정**: `"hint"`

#### 1.3.7 `no-shadowing` (변수 가리기)

외부 스코프의 변수를 내부 스코프에서 다시 선언하는 것을 탐지합니다.

```ksj
정수 x = 10

함수 예제() {
    정수 x = 20  # 경고: 외부 변수 'x'를 가립니다
}
```

**권장 설정**: `"warning"`

#### 1.3.8 `no-unused-parameter` (미사용 매개변수)

함수 매개변수가 사용되지 않는 경우를 탐지합니다.

```ksj
함수 더하기(a, b) {
    반환 a  # 힌트: 매개변수 'b'가 사용되지 않습니다
}
```

**권장 설정**: `"hint"`

### 1.4 고급 설정

규칙별로 세부 옵션을 설정할 수 있습니다:

```json
{
  "rules": {
    "unused-variable": {
      "severity": "warning",
      "enabled": true
    },
    "no-magic-number": {
      "severity": "hint",
      "enabled": true
    }
  }
}
```

### 1.5 사용 예제

```bash
# Linter 실행
kingsejong --lint myfile.ksj

# 설정 파일 지정
kingsejong --lint --config .ksjlintrc myfile.ksj
```

---

## 2. Formatter 설정 (`.ksjfmtrc`)

### 2.1 기본 설정

프로젝트 루트에 `.ksjfmtrc` 파일을 생성하세요:

```json
{
  "indentSize": 4,
  "useSpaces": true,
  "spaceAroundOperators": true,
  "spaceAfterComma": true,
  "spaceBeforeBrace": true,
  "maxLineLength": 100
}
```

### 2.2 설정 옵션

#### 2.2.1 `indentSize` (들여쓰기 크기)

- **타입**: 정수
- **기본값**: `4`
- **설명**: 들여쓰기 공백 또는 탭의 개수

```ksj
# indentSize: 4
함수 예제() {
    만약 (참) {
        출력("4칸 들여쓰기")
    }
}

# indentSize: 2
함수 예제() {
  만약 (참) {
    출력("2칸 들여쓰기")
  }
}
```

**권장 설정**: `4`

#### 2.2.2 `useSpaces` (공백 사용)

- **타입**: 불린
- **기본값**: `true`
- **설명**: 들여쓰기에 공백을 사용할지, 탭을 사용할지 결정

```json
{
  "useSpaces": true   // 공백 사용 (권장)
}
```

```json
{
  "useSpaces": false  // 탭 사용
}
```

**권장 설정**: `true`

#### 2.2.3 `spaceAroundOperators` (연산자 주변 공백)

- **타입**: 불린
- **기본값**: `true`
- **설명**: 연산자 앞뒤에 공백 추가

```ksj
# spaceAroundOperators: true
정수 결과 = a + b

# spaceAroundOperators: false
정수 결과=a+b
```

**권장 설정**: `true`

#### 2.2.4 `spaceAfterComma` (쉼표 뒤 공백)

- **타입**: 불린
- **기본값**: `true`
- **설명**: 쉼표 뒤에 공백 추가

```ksj
# spaceAfterComma: true
함수 예제(a, b, c) {
    배열 숫자들 = [1, 2, 3]
}

# spaceAfterComma: false
함수 예제(a,b,c) {
    배열 숫자들 = [1,2,3]
}
```

**권장 설정**: `true`

#### 2.2.5 `spaceBeforeBrace` (중괄호 앞 공백)

- **타입**: 불린
- **기본값**: `true`
- **설명**: 여는 중괄호 `{` 앞에 공백 추가

```ksj
# spaceBeforeBrace: true
함수 예제() {
    만약 (참) {
        출력("공백 있음")
    }
}

# spaceBeforeBrace: false
함수 예제(){
    만약 (참){
        출력("공백 없음")
    }
}
```

**권장 설정**: `true`

#### 2.2.6 `maxLineLength` (최대 줄 길이)

- **타입**: 정수
- **기본값**: `100`
- **설명**: 한 줄의 최대 문자 수 (자동 줄 바꿈은 아직 미지원)

```ksj
# maxLineLength: 80
함수 매우긴함수이름(
    매개변수1,
    매개변수2,
    매개변수3
) {
    # 80자 초과 시 경고
}
```

**권장 설정**: `100` (프로젝트에 따라 80~120)

### 2.3 프로파일별 설정

팀별 코딩 스타일에 맞는 프로파일을 선택할 수 있습니다:

#### Google 스타일

```json
{
  "indentSize": 2,
  "useSpaces": true,
  "spaceAroundOperators": true,
  "spaceAfterComma": true,
  "spaceBeforeBrace": true,
  "maxLineLength": 80
}
```

#### Microsoft 스타일

```json
{
  "indentSize": 4,
  "useSpaces": true,
  "spaceAroundOperators": true,
  "spaceAfterComma": true,
  "spaceBeforeBrace": true,
  "maxLineLength": 120
}
```

#### 컴팩트 스타일

```json
{
  "indentSize": 2,
  "useSpaces": true,
  "spaceAroundOperators": false,
  "spaceAfterComma": true,
  "spaceBeforeBrace": false,
  "maxLineLength": 100
}
```

### 2.4 사용 예제

```bash
# Formatter 실행 (표준 출력)
kingsejong --format myfile.ksj

# 파일에 직접 적용 (in-place)
kingsejong --format --write myfile.ksj

# 설정 파일 지정
kingsejong --format --config .ksjfmtrc myfile.ksj
```

---

## 3. VS Code 통합

### 3.1 설정 파일 자동 인식

KingSejong VS Code 확장은 프로젝트 루트의 `.ksjlintrc`와 `.ksjfmtrc`를 자동으로 인식합니다.

### 3.2 VS Code 설정 예제

`.vscode/settings.json`:

```json
{
  "kingsejong.linter.enable": true,
  "kingsejong.linter.configFile": "${workspaceFolder}/.ksjlintrc",
  "kingsejong.formatter.enable": true,
  "kingsejong.formatter.configFile": "${workspaceFolder}/.ksjfmtrc",
  "editor.formatOnSave": true
}
```

---

## 4. CI/CD 통합

### 4.1 GitHub Actions

```yaml
name: KingSejong CI

on: [push, pull_request]

jobs:
  lint:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Run Linter
        run: |
          kingsejong --lint --config .ksjlintrc **/*.ksj
```

### 4.2 Pre-commit Hook

`.git/hooks/pre-commit`:

```bash
#!/bin/bash

# Linter 실행
kingsejong --lint --config .ksjlintrc $(git diff --cached --name-only --diff-filter=ACM | grep '\.ksj$')

if [ $? -ne 0 ]; then
    echo "Linter에서 에러가 발견되었습니다. 수정 후 다시 커밋하세요."
    exit 1
fi

# Formatter 실행
kingsejong --format --write --config .ksjfmtrc $(git diff --cached --name-only --diff-filter=ACM | grep '\.ksj$')

git add $(git diff --cached --name-only --diff-filter=ACM | grep '\.ksj$')
```

---

## 5. 마이그레이션 가이드

### 5.1 기존 프로젝트에 적용

```bash
# 1. 프로젝트 루트에 기본 설정 파일 생성
cd your-project
cp path/to/kingsejonglang/.ksjlintrc .
cp path/to/kingsejonglang/.ksjfmtrc .

# 2. 전체 프로젝트 포맷팅
find . -name "*.ksj" -exec kingsejong --format --write {} \;

# 3. Lint 검사
find . -name "*.ksj" -exec kingsejong --lint {} \;
```

### 5.2 팀 설정 공유

설정 파일을 Git에 커밋하여 팀 전체가 동일한 코딩 스타일을 사용하도록 합니다:

```bash
git add .ksjlintrc .ksjfmtrc
git commit -m "feat: Linter 및 Formatter 설정 추가"
git push
```

---

## 6. 문제 해결

### 6.1 설정 파일이 인식되지 않는 경우

```bash
# 설정 파일 위치 확인
ls -la .ksjlintrc .ksjfmtrc

# 설정 파일 문법 검증
cat .ksjlintrc | jq .
cat .ksjfmtrc | jq .
```

### 6.2 특정 규칙만 비활성화

```json
{
  "rules": {
    "no-magic-number": "off",
    "no-unused-parameter": "off"
  }
}
```

### 6.3 파일별 규칙 무시

코드 내에서 주석으로 Linter 규칙을 비활성화할 수 있습니다 (향후 지원 예정):

```ksj
# ksjlint-disable-next-line no-magic-number
정수 파이 = 3.14159

# ksjlint-disable
정수 x = 100
정수 y = 200
# ksjlint-enable
```

---

## 7. 추가 리소스

- **Linter 소스**: `src/linter/`
- **Formatter 소스**: `src/formatter/`
- **규칙 정의**: `src/linter/rules/`
- **이슈 리포트**: [GitHub Issues](https://github.com/0xmhha/kingsejonglang/issues)

---

**마지막 업데이트**: 2025-11-21
**버전**: v0.5.0
