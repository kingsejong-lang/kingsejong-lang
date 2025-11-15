# KingSejong Language - 알려진 이슈

## ✅ 해결됨 (Resolved)

### F5.5 패턴 매칭 - 본문 표현식 파싱 실패
**날짜**: 2025-11-15
**상태**: ✅ 해결됨 (2025-11-15)
**우선순위**: P0 (긴급) → 해결

#### 문제 설명
패턴 매칭 구문에서 본문 표현식이 null로 파싱되어 실행 결과가 항상 "없음"(nil)을 반환합니다.

#### 재현 방법
```kotlin
출력(1 에 대해 { 1 -> "하나" 2 -> "둘" })
```

**예상 결과**: `하나`
**실제 결과**: `없음`

#### 디버그 로그
```
[EVAL] evalMatchExpression called, cases=2
[EVAL] matchValue=1
[EVAL] Literal pattern, patternValue=1
[EVAL] Types match
[EVAL] Integer values match!
[EVAL] matched=1
[EVAL] Pattern matched! Evaluating body...
[EVAL] Body is NULL!!!  ← 문제 지점
[EVAL] Body result: 없음
[EVAL] Returning: 없음
```

#### 근본 원인 분석
`Parser::parseMatchCase()` 함수에서 토큰 위치 관리 문제:

```cpp
// 현재 코드 (src/parser/Parser.cpp:1145-1156)
// '->' 확인
if (!expectPeek(TokenType::ARROW))
{
    errors_.push_back("패턴 매칭 케이스에는 '->'가 필요합니다");
    return MatchCase(nullptr, nullptr, nullptr);
}

nextToken(); // 본문 표현식으로 이동

// 본문 파싱
auto body = parseExpression(Precedence::LOWEST);  // ← null 반환
```

**문제점**:
- `expectPeek(ARROW)` 호출 후 `curToken`이 ARROW에 위치
- `nextToken()` 한 번만 호출하면 본문의 첫 토큰에 도달하지 못할 가능성
- 또는 토큰 스트림에서 예상치 못한 위치에 있어 `parseExpression`이 유효한 표현식을 찾지 못함

#### 영향 범위
- ✅ Lexer: 정상 (E_DAEHAE 토큰 생성 확인)
- ✅ Parser (parseMatchExpression): 정상 (MatchExpression 생성, cases 개수 정확)
- ❌ Parser (parseMatchCase): **본문 파싱 실패**
- ✅ Evaluator: 정상 (패턴 매칭 로직 정상 작동 확인)

#### 관련 파일
- `src/parser/Parser.cpp:1126-1171` - parseMatchCase 함수
- `src/parser/Parser.cpp:1086-1124` - parseMatchExpression 함수
- `src/evaluator/Evaluator.cpp:1046-1254` - evalMatchExpression 함수
- `tests/PatternMatchingTest.cpp` - 13개 테스트 중 12개 실패

#### 시도한 해결 방법
1. ✅ 토큰 이름 수정 (TRUE/FALSE → CHAM/GEOJIT)
2. ✅ 테스트 구문에 공백 추가 ("에 대해" 앞)
3. ✅ parseMatchCase에 본문 null 체크 및 에러 메시지 추가
4. ❌ 토큰 위치 조정 시도 중 (미완료)

#### 다음 단계
1. `parseMatchCase`에서 각 단계별 `curToken`과 `peekToken` 상태 로깅
2. 토큰 스트림 분석: `pattern ARROW body` 순서에서 정확한 위치 파악
3. `parseExpression(Precedence::LOWEST)` 호출 시 `curToken` 위치 검증
4. 필요시 `expectPeek` 대신 `curTokenIs` + `nextToken` 조합 사용

#### 테스트 결과 (해결 전)
```
[  PASSED  ] 1 test.   (NoMatchingCase - 예외 처리 테스트)
[  FAILED  ] 12 tests.  (본문 표현식 파싱 필요한 모든 테스트)
```

#### 해결 방법
**근본 원인**: MatchCase 생성자 파라미터 순서 오류
- MatchCase 클래스 생성자: `MatchCase(pattern, body, guard)`
- Parser에서 호출: `MatchCase(pattern, guard, body)` ❌

**수정 사항** (src/parser/Parser.cpp:1161):
```cpp
// 수정 전
return MatchCase(std::move(pattern), std::move(guard), std::move(body));

// 수정 후
return MatchCase(std::move(pattern), std::move(body), std::move(guard));
```

**재구현 세부 사항**:
1. Parser 함수들 재작성 (~140줄)
   - parseMatchExpression
   - parseMatchCase (버그 수정 포함)
   - parsePattern
   - parseArrayPattern

2. Evaluator 함수 재작성 (~160줄)
   - evalMatchExpression
   - 리터럴, 와일드카드, 바인딩, 배열 패턴 매칭 로직
   - 가드 조건 평가
   - 환경 스코핑

**테스트 결과 (해결 후)**:
```
[==========] Running 13 tests from 1 test suite.
[  PASSED  ] 13 tests.
```

---

## 📝 참고사항

### 패턴 매칭 구현 완료 사항
- ✅ AST 노드 정의 (MatchExpression, Pattern 계층)
- ✅ Lexer 토큰 추가 (E_DAEHAE, ARROW, UNDERSCORE, WHEN)
- ✅ Parser 등록 (infix parser, precedence)
- ✅ Evaluator 로직 (리터럴, 와일드카드, 바인딩, 배열 패턴)
- ✅ 가드 조건 지원
- ✅ 환경 스코핑 (패턴 바인딩용 enclosed environment)

### 기술 부채
- ⚠️ 배열 패턴의 rest 파라미터 (`...name`) 미구현 (DOT 토큰 미지원)
- ⚠️ "에 대해" 토큰화 시 앞에 공백 필수 (렉서 제약)

---

## 버전 정보
- **브랜치**: main
- **마지막 커밋**: fac9864 (feat: 패턴 매칭을 위한 Lexer 토큰 추가)
- **Phase**: F5.5 패턴 매칭 구현 중
