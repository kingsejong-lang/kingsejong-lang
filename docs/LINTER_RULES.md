# KingSejong Linter 규칙 가이드

> **버전**: v0.5.0
> **총 규칙 수**: 13개
> **최종 업데이트**: 2025-11-21

---

## 목차

1. [개요](#개요)
2. [규칙 목록](#규칙-목록)
3. [기본 규칙 (8개)](#기본-규칙-8개)
4. [추가 규칙 (5개)](#추가-규칙-5개)
5. [설정 방법](#설정-방법)
6. [커스텀 규칙 작성](#커스텀-규칙-작성)

---

## 개요

KingSejong Linter는 코드 품질을 향상시키기 위한 정적 분석 도구입니다. `.ksjlintrc` 파일로 규칙을 설정할 수 있습니다.

### 심각도 레벨

- **error**: 치명적 오류, 빌드 실패
- **warning**: 경고, 주의 필요
- **hint**: 힌트, 개선 권장

---

## 규칙 목록

### 기본 규칙 (구현 완료) ✅

1. `unused-variable` - 사용하지 않는 변수 감지
2. `dead-code` - 도달 불가능한 코드 감지
3. `no-self-comparison` - 자기 자신과 비교 금지
4. `constant-condition` - 상수 조건문 경고
5. `empty-block` - 빈 블록 경고
6. `no-magic-number` - 매직 넘버 사용 경고
7. `no-shadowing` - 변수 섀도잉 경고
8. `no-unused-parameter` - 사용하지 않는 매개변수 경고

### 추가 규칙 (구현 예정) 📝

9. `camelcase-naming` - camelCase 네이밍 컨벤션
10. `max-line-length` - 최대 줄 길이 제한
11. `no-trailing-spaces` - 행 끝 공백 제거
12. `prefer-const` - 상수 선호 (재할당 없는 변수)
13. `no-duplicate-imports` - 중복 임포트 금지

---

## 기본 규칙 (8개)

### 1. unused-variable

**설명**: 선언했지만 사용하지 않는 변수를 감지합니다.

**나쁜 예**:
```ksj
x = 10  # x를 사용하지 않음
출력("Hello")
```

**좋은 예**:
```ksj
x = 10
출력(x)
```

**설정**:
```json
{
  "rules": {
    "unused-variable": "warning"
  }
}
```

---

### 2. dead-code

**설명**: 실행되지 않는 코드를 감지합니다.

**나쁜 예**:
```ksj
함수 테스트() {
    반환 10
    출력("이 코드는 실행되지 않음")  # 도달 불가능
}
```

**좋은 예**:
```ksj
함수 테스트() {
    출력("정상 실행")
    반환 10
}
```

---

### 3. no-self-comparison

**설명**: 자기 자신과의 비교를 금지합니다.

**나쁜 예**:
```ksj
x = 10
만약 (x == x) {  # 항상 참
    출력("항상 참")
}
```

**좋은 예**:
```ksj
x = 10
y = 10
만약 (x == y) {
    출력("같음")
}
```

---

### 4. constant-condition

**설명**: 항상 참 또는 거짓인 조건문을 경고합니다.

**나쁜 예**:
```ksj
만약 (참) {  # 항상 참
    출력("항상 실행")
}

i가 0부터 10 미만 {
    만약 (거짓) {  # 절대 실행 안 됨
        출력("실행되지 않음")
    }
}
```

**좋은 예**:
```ksj
x = 10
만약 (x > 5) {  # 변수 기반 조건
    출력("조건부 실행")
}
```

---

### 5. empty-block

**설명**: 빈 블록을 경고합니다.

**나쁜 예**:
```ksj
만약 (x > 5) {
    # 빈 블록
}

함수 처리() {
    # 구현 없음
}
```

**좋은 예**:
```ksj
만약 (x > 5) {
    출력("x는 5보다 큽니다")
}

함수 처리() {
    # TODO: 나중에 구현
    반환 null
}
```

---

### 6. no-magic-number

**설명**: 매직 넘버 사용을 경고합니다. 의미 있는 상수를 사용하세요.

**나쁜 예**:
```ksj
면적 = 반지름 * 반지름 * 3.14159
```

**좋은 예**:
```ksj
PI = 3.14159
면적 = 반지름 * 반지름 * PI
```

---

### 7. no-shadowing

**설명**: 외부 스코프의 변수를 가리는 것을 경고합니다.

**나쁜 예**:
```ksj
x = 10

함수 테스트() {
    x = 20  # 외부 x를 가림
    출력(x)
}
```

**좋은 예**:
```ksj
x = 10

함수 테스트() {
    y = 20  # 다른 이름 사용
    출력(y)
}
```

---

### 8. no-unused-parameter

**설명**: 사용하지 않는 함수 매개변수를 경고합니다.

**나쁜 예**:
```ksj
함수 계산(a, b) {  # b를 사용하지 않음
    반환 a * 2
}
```

**좋은 예**:
```ksj
함수 계산(a, b) {
    반환 a * b
}

# 또는 필요 없으면 제거
함수 계산(a) {
    반환 a * 2
}
```

---

## 추가 규칙 (5개)

### 9. camelcase-naming ✨ 새 규칙

**설명**: camelCase 네이밍 컨벤션을 권장합니다.

**나쁜 예**:
```ksj
사용자_이름 = "철수"  # snake_case
MAXVALUE = 100        # UPPER_CASE
```

**좋은 예**:
```ksj
사용자이름 = "철수"   # camelCase (한글)
maxValue = 100       # camelCase (영문)
MAX_VALUE = 100      # 상수는 허용
```

**설정**:
```json
{
  "rules": {
    "camelcase-naming": "hint"
  },
  "settings": {
    "naming-convention": "camelCase"
  }
}
```

---

### 10. max-line-length ✨ 새 규칙

**설명**: 한 줄의 최대 길이를 제한합니다.

**나쁜 예**:
```ksj
# 120자 초과
출력("이것은 매우 긴 문자열입니다. 이것은 매우 긴 문자열입니다. 이것은 매우 긴 문자열입니다. 이것은 매우 긴 문자열입니다. 이것은 매우 긴 문자열입니다.")
```

**좋은 예**:
```ksj
# 여러 줄로 분할
메시지 = "이것은 매우 긴 문자열입니다. " +
         "이것은 매우 긴 문자열입니다. " +
         "이것은 매우 긴 문자열입니다."
출력(메시지)
```

**설정**:
```json
{
  "rules": {
    "max-line-length": "hint"
  },
  "settings": {
    "max-line-length": 120
  }
}
```

---

### 11. no-trailing-spaces ✨ 새 규칙

**설명**: 행 끝의 불필요한 공백을 제거합니다.

**나쁜 예**:
```ksj
x = 10   ␣␣␣
출력(x)␣␣
```

**좋은 예**:
```ksj
x = 10
출력(x)
```

**자동 수정**: Formatter가 자동으로 제거합니다.

---

### 12. prefer-const ✨ 새 규칙

**설명**: 재할당하지 않는 변수는 상수로 선언하는 것을 권장합니다.

**나쁜 예**:
```ksj
x = 10
y = x * 2
출력(y)  # x는 재할당하지 않음
```

**좋은 예**:
```ksj
상수 X = 10
y = X * 2
출력(y)
```

---

### 13. no-duplicate-imports ✨ 새 규칙

**설명**: 같은 모듈을 중복으로 임포트하는 것을 경고합니다.

**나쁜 예**:
```ksj
가져오기 "math"
가져오기 "string"
가져오기 "math"  # 중복!
```

**좋은 예**:
```ksj
가져오기 "math"
가져오기 "string"
```

---

## 설정 방법

### .ksjlintrc 파일

프로젝트 루트에 `.ksjlintrc` 파일을 생성합니다:

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
    "no-unused-parameter": "hint",
    "camelcase-naming": "hint",
    "max-line-length": "hint",
    "no-trailing-spaces": "hint",
    "prefer-const": "hint",
    "no-duplicate-imports": "warning"
  },
  "settings": {
    "max-line-length": 120,
    "naming-convention": "camelCase"
  }
}
```

### 명령줄 실행

```bash
# Lint 실행
ksjlint myfile.ksj

# 모든 파일 검사
ksjlint src/**/*.ksj

# 자동 수정
ksjlint --fix myfile.ksj
```

### CI/CD 통합

#### GitHub Actions

```yaml
name: Lint

on: [push, pull_request]

jobs:
  lint:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Run KingSejong Linter
        run: |
          ksjlint src/**/*.ksj
```

#### Pre-commit Hook

`.git/hooks/pre-commit`:
```bash
#!/bin/bash

echo "Running KingSejong Linter..."
ksjlint src/**/*.ksj

if [ $? -ne 0 ]; then
    echo "Linting failed. Please fix the issues before committing."
    exit 1
fi
```

---

## 커스텀 규칙 작성

### 규칙 템플릿

새 규칙을 작성하려면 다음 템플릿을 사용하세요:

**src/linter/rules/MyCustomRule.h**:
```cpp
#pragma once

#include "linter/Rule.h"

namespace kingsejong {
namespace linter {

class MyCustomRule : public Rule {
public:
    MyCustomRule();

    void check(const ast::Node* node, LintContext& context) override;

    std::string getName() const override { return "my-custom-rule"; }
    std::string getDescription() const override {
        return "Custom rule description";
    }
};

} // namespace linter
} // namespace kingsejong
```

**src/linter/rules/MyCustomRule.cpp**:
```cpp
#include "linter/rules/MyCustomRule.h"

namespace kingsejong {
namespace linter {

MyCustomRule::MyCustomRule()
    : Rule("my-custom-rule", "Custom rule description")
{}

void MyCustomRule::check(const ast::Node* node, LintContext& context) {
    // 규칙 로직 구현

    // 문제 발견 시 보고
    if (/* 조건 */) {
        context.report(
            node->location(),
            Severity::WARNING,
            "Warning message"
        );
    }
}

} // namespace linter
} // namespace kingsejong
```

### 규칙 등록

**src/linter/Linter.cpp**에 규칙을 등록합니다:

```cpp
#include "linter/rules/MyCustomRule.h"

void Linter::registerDefaultRules() {
    // ... 기존 규칙들 ...

    registerRule(std::make_unique<MyCustomRule>());
}
```

---

## 규칙 우선순위

1. **error**: 즉시 수정 필요
2. **warning**: 가능한 빨리 수정
3. **hint**: 시간 날 때 개선

---

## 규칙 비활성화

특정 줄에서 규칙을 비활성화하려면:

```ksj
# ksjlint-disable-next-line no-magic-number
면적 = 반지름 * 반지름 * 3.14159

# 또는 블록 전체
# ksjlint-disable no-magic-number
계산1 = x * 3.14
계산2 = y * 2.71
# ksjlint-enable no-magic-number
```

---

## 추가 자료

- [Linter 소스 코드](../src/linter/)
- [규칙 구현 예제](../src/linter/rules/)
- [설정 가이드](./CONFIG_GUIDE.md)

---

**작성자**: KingSejong Team
**최종 수정**: 2025-11-21
**피드백**: https://github.com/anthropics/kingsejong-lang/issues
