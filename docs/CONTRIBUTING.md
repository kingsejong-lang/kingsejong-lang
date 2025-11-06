# KingSejong 프로젝트 기여 가이드

KingSejong 프로젝트에 관심을 가져주셔서 감사합니다! 이 문서는 프로젝트에 기여하는 방법을 설명합니다.

## 목차

1. [시작하기](#시작하기)
2. [개발 환경 설정](#개발-환경-설정)
3. [Git 워크플로우](#git-워크플로우)
4. [커밋 메시지 규칙](#커밋-메시지-규칙)
5. [Pull Request 프로세스](#pull-request-프로세스)
6. [코드 리뷰 가이드](#코드-리뷰-가이드)
7. [이슈 작성 가이드](#이슈-작성-가이드)
8. [테스트 작성](#테스트-작성)
9. [문서화](#문서화)

---

## 시작하기

### 기여할 수 있는 방법

1. **버그 수정**: 이슈에서 버그를 찾아 수정
2. **새 기능 구현**: TODOLIST.md의 Feature 구현
3. **문서 개선**: README, 가이드, 주석 개선
4. **테스트 추가**: 테스트 커버리지 향상
5. **코드 리뷰**: PR에 대한 리뷰 및 피드백
6. **이슈 리포팅**: 버그 발견 시 상세히 보고

### 기여 전 확인사항

- [ ] [CODING_STYLE.md](CODING_STYLE.md) 읽기
- [ ] [TODOLIST.md](TODOLIST.md)에서 작업할 항목 확인
- [ ] 중복 작업 방지를 위해 기존 이슈/PR 검색
- [ ] 큰 변경사항은 먼저 이슈로 논의

---

## 개발 환경 설정

### 요구사항

- C++23 호환 컴파일러
  - GCC 13 이상
  - Clang 16 이상
  - MSVC 2022 이상
- CMake 3.20 이상
- Git 2.30 이상

### 저장소 설정

```bash
# 1. Fork 후 클론
git clone https://github.com/YOUR_USERNAME/kingsejong-lang.git
cd kingsejong-lang

# 2. Upstream 설정
git remote add upstream https://github.com/0xmhha/kingsejong-lang.git

# 3. 빌드
mkdir build && cd build
cmake ..
cmake --build .

# 4. 테스트 실행
ctest --output-on-failure
```

---

## Git 워크플로우

### 브랜치 전략

```
main (보호됨)
├── feature/f0.1-project-setup
├── feature/f1.2-josa-recognizer
├── feature/f2.1-function-support
├── bugfix/issue-123-lexer-crash
└── docs/update-readme
```

### 브랜치 네이밍 규칙

- **feature/**: 새 기능 개발
  - `feature/f1.2-josa-recognizer`
  - `feature/add-error-handling`
- **bugfix/**: 버그 수정
  - `bugfix/issue-123-lexer-crash`
  - `bugfix/fix-memory-leak`
- **docs/**: 문서 작업
  - `docs/update-readme`
  - `docs/add-examples`
- **refactor/**: 리팩토링
  - `refactor/parser-cleanup`
- **test/**: 테스트 추가/수정
  - `test/lexer-edge-cases`

### 작업 프로세스

```bash
# 1. 최신 main 받기
git checkout main
git pull upstream main

# 2. Feature 브랜치 생성
git checkout -b feature/my-feature

# 3. 작업 및 커밋 (여러 번 가능)
git add .
git commit -m "feat: add feature X"

# 4. 원격 저장소에 푸시
git push origin feature/my-feature

# 5. PR 생성 (GitHub UI 또는 gh CLI)
gh pr create --base main --title "[F1.2] Feature 설명"

# 6. 리뷰 후 스쿼시 머지
# (브랜치는 자동으로 삭제됨)
```

---

## 커밋 메시지 규칙

### Conventional Commits 형식

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Type 종류

- **feat**: 새로운 기능 추가
- **fix**: 버그 수정
- **docs**: 문서 변경
- **style**: 코드 포맷팅 (기능 변경 없음)
- **refactor**: 리팩토링
- **test**: 테스트 추가/수정
- **chore**: 빌드, 설정 변경

### 예제

```bash
# Good
feat(lexer): add UTF-8 character recognition
fix(parser): resolve precedence bug in binary expressions
docs(readme): update build instructions
test(lexer): add edge case tests for Korean josa

# Bad
update code
fix bug
WIP
```

### 상세 예제

```
feat(josa): implement josa recognition for 을/를

JosaRecognizer 클래스를 추가하여 한글 단어의 마지막 글자를
분석해 적절한 조사를 선택합니다.

- hasJongseong() 메서드 구현
- recognizeJosa() 메서드 구현
- 테스트 케이스 20개 추가

Closes #42
```

### 커밋 메시지 작성 규칙

1. **제목 (Subject)**
   - 50자 이내
   - 명령형 동사 사용 ("add", "fix", "update")
   - 마침표 없음
   - 영문 소문자로 시작

2. **본문 (Body)** (선택)
   - 72자마다 줄바꿈
   - 무엇을, 왜 변경했는지 설명
   - 어떻게는 코드가 설명

3. **푸터 (Footer)** (선택)
   - `Closes #issue-number`
   - `Breaking Change: 설명`
   - `Reviewed-by: name`

---

## Pull Request 프로세스

### PR 생성 전 체크리스트

- [ ] 최신 main 브랜치와 동기화
- [ ] 빌드 성공 확인
- [ ] 모든 테스트 통과 확인
- [ ] 코딩 스타일 준수 확인
- [ ] 관련 문서 업데이트
- [ ] TODOLIST.md 상태 업데이트

### PR 제목 규칙

```
[Feature ID] 간결한 설명

예시:
[F0.1] 프로젝트 기본 설정 완료
[F1.2] JosaRecognizer 구현
[Bugfix] Lexer UTF-8 처리 오류 수정
```

### PR 설명 템플릿

```markdown
## 요약
이 PR이 무엇을 하는지 한 줄 요약

## 변경 내용
- 변경 사항 1
- 변경 사항 2
- 변경 사항 3

## 테스트
- 테스트 방법 설명
- 테스트 결과

## 체크리스트
- [ ] 빌드 성공
- [ ] 테스트 통과
- [ ] 문서 업데이트
- [ ] TODOLIST.md 업데이트

## 관련 이슈
Closes #issue-number
```

### PR 크기 가이드

- **소형 PR 권장**: 200-300줄 이하
- **대형 PR은 분리**: 500줄 이상이면 여러 PR로 나누기
- **리뷰 시간 고려**: 작을수록 빠른 리뷰 가능

### 머지 방법

**Squash Merge만 사용**

- Feature 브랜치의 여러 커밋이 하나로 통합
- main 브랜치 히스토리 깔끔하게 유지
- 브랜치는 자동으로 삭제

---

## 코드 리뷰 가이드

### 리뷰어 역할

#### 확인 사항

1. **기능 정확성**
   - 요구사항 충족 여부
   - 예상대로 동작하는지

2. **코드 품질**
   - CODING_STYLE.md 준수
   - SOLID 원칙 적용
   - 가독성

3. **테스트**
   - 충분한 테스트 커버리지
   - Edge case 처리

4. **성능**
   - 불필요한 복사 없는지
   - 메모리 누수 없는지

5. **보안**
   - 입력 검증
   - 에러 처리

#### 리뷰 코멘트 작성

```markdown
# Good - 구체적이고 건설적
제안: `std::string_view`를 사용하면 불필요한 복사를 줄일 수 있습니다.
\```cpp
void process(std::string_view str) { ... }
\```

# Bad - 모호하고 부정적
이 코드는 좋지 않습니다.
```

#### 리뷰 상태

- **Approve**: 승인 (변경 없음)
- **Request Changes**: 수정 필요
- **Comment**: 의견 제시 (승인과 무관)

### PR 작성자 역할

1. **리뷰 반영**
   - 코멘트에 신속히 대응
   - 동의하지 않으면 근거와 함께 설명

2. **변경사항 커밋**
   - 리뷰 반영 후 추가 커밋
   - 머지 시 스쿼시되므로 커밋 메시지 자유롭게

3. **토론 종료**
   - 해결된 코멘트는 "Resolve conversation"

---

## 이슈 작성 가이드

### 이슈 템플릿

#### 버그 리포트

```markdown
## 버그 설명
명확하고 간결한 버그 설명

## 재현 방법
1. '...' 실행
2. '...' 입력
3. '...' 클릭
4. 에러 발생

## 예상 동작
정상적으로 어떻게 동작해야 하는지

## 실제 동작
실제로 어떻게 동작하는지

## 환경
- OS: Ubuntu 22.04
- 컴파일러: GCC 13.1
- KingSejong 버전: 0.1.0

## 추가 정보
스크린샷, 로그 등
```

#### 기능 제안

```markdown
## 제안 내용
새 기능에 대한 설명

## 동기
왜 이 기능이 필요한지

## 예상 동작
\```ksj
// 예제 코드
배열을 정렬하고 출력하라
\```

## 대안
고려한 다른 방법들

## 추가 정보
관련 자료, 참고 링크
```

### 이슈 라벨

- **bug**: 버그
- **enhancement**: 새 기능
- **documentation**: 문서
- **good first issue**: 초보자 환영
- **help wanted**: 도움 필요
- **priority:high**: 높은 우선순위

---

## 테스트 작성

### 테스트 구조

```cpp
#include <gtest/gtest.h>
#include "Lexer.h"

class LexerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // 각 테스트 전 실행
    }

    void TearDown() override
    {
        // 각 테스트 후 실행
    }
};

TEST_F(LexerTest, ShouldRecognizeIntegerToken)
{
    // Arrange
    Lexer lexer("123");

    // Act
    auto token = lexer.nextToken();

    // Assert
    EXPECT_EQ(token.type, TokenType::Integer);
    EXPECT_EQ(token.literal, "123");
}
```

### 테스트 커버리지

- **최소 목표**: 80%
- **핵심 기능**: 90% 이상
- **Edge case 포함**
- **에러 케이스 포함**

### 테스트 실행

```bash
# 전체 테스트
ctest --output-on-failure

# 특정 테스트만
./build/bin/kingsejong_tests --gtest_filter=LexerTest.*

# Verbose 모드
ctest -V
```

---

## 문서화

### 문서 업데이트 시점

- 새 기능 추가 시
- API 변경 시
- 빌드 방법 변경 시
- 예제 코드 추가 시

### 문서 종류

1. **README.md**: 프로젝트 개요, 빌드 방법
2. **TODOLIST.md**: 작업 목록, 진행 상황
3. **코드 주석**: API 문서, 복잡한 로직 설명
4. **예제**: examples/ 디렉토리에 샘플 코드

### 주석 작성

```cpp
/**
 * @brief 다음 토큰을 반환합니다
 *
 * 현재 위치에서 다음 토큰을 읽어 반환합니다.
 * 더 이상 토큰이 없으면 EOF 토큰을 반환합니다.
 *
 * @return Token 다음 토큰
 * @throws LexerException 잘못된 문자가 있는 경우
 *
 * @example
 * \code
 * Lexer lexer("1 + 2");
 * auto token = lexer.nextToken();  // Integer "1"
 * \endcode
 */
Token nextToken();
```

---

## 도움말

### 질문하기

- **GitHub Discussions**: 일반적인 질문
- **GitHub Issues**: 버그, 기능 제안
- **코드 리뷰**: 구체적인 구현 질문

### 도움받기

- **Good First Issue**: 초보자를 위한 이슈
- **Documentation**: 문서 개선
- **Help Wanted**: 도움이 필요한 이슈

### 행동 강령

- 존중과 배려
- 건설적인 피드백
- 다양성 존중
- 협력적인 태도

---

## 체크리스트 요약

기여 전 최종 확인:

- [ ] Fork & Clone 완료
- [ ] 브랜치 생성 (올바른 네이밍)
- [ ] 코딩 스타일 준수
- [ ] 테스트 작성 및 통과
- [ ] 문서 업데이트
- [ ] 커밋 메시지 규칙 준수
- [ ] PR 템플릿 작성
- [ ] CI/CD 통과
- [ ] TODOLIST.md 업데이트

---

## 감사의 말

여러분의 기여가 KingSejong을 더 나은 프로젝트로 만듭니다.
시간을 내어 기여해주셔서 진심으로 감사합니다!
