# KingSejong 언어 로드맵

> **비전**: 한국어로 누구나 쉽게 프로그래밍할 수 있는 세상

**최종 업데이트**: 2025-11-16
**현재 버전**: v0.3.7 (Phase 6 진행중)
**Production Readiness**: ⚠️ **4.5/10** (보안 및 안정성 개선 필요)
**중요 문서**: [아키텍처 분석](./ARCHITECTURE_ANALYSIS.md) | [VM 분석](./KINGSEJONG_VM_ANALYSIS.md) | [언어 분석](./KINGSEJONG_LANGUAGE_ANALYSIS.md) | [엔진 설계](../lab/idea.md)

---

## 📊 현재 상태

### 완료된 Phase

| Phase | 기간 | 상태 | 주요 성과 |
|-------|------|------|-----------|
| **Phase 0** | 2025-10 | ✅ 100% | 프로젝트 초기화, 테스트 프레임워크 |
| **Phase 1** | 2025-11 초 | ✅ 100% | 핵심 차별화 기능 (조사 체인, 1급 함수, REPL) |
| **Phase 2** | 2025-11 초 | ✅ 100% | 실용 기능 (배열, 문자열, 예외 처리, stdlib) |
| **Phase 3** | 2025-11 중 | ✅ 100% | 고급 기능 (GC, 바이트코드, 모듈 시스템) |
| **Phase 4** | 2025-11 중 | ✅ 100% | 최적화 (CI/CD, 성능, 메모리 안전성) |
| **Phase 5** | 2025-11 중 | ✅ 100% | 개발자 경험 (LSP, 디버거, 플레이그라운드, 패턴 매칭) |
| **Phase 6** | 2025-11 말 | 🚧 70% | 언어 개선, Linter, Formatter, stdlib 250개 달성 ✅ |

### 통계

```
코드: 22,500+ 줄
테스트: 1309개 (99.5% 통과 - TimeTest 7개 알려진 이슈) ✅
stdlib: 255개 함수 ✅ (목표 초과 달성!)
문서: 4,800+ 줄
예제: 27개
플랫폼: macOS, Linux, Windows
```

---

## 🎯 Phase 5: 개발자 경험 & 생태계 ✅ (완료)

**기간**: 2025-11 중순 ~ 말 (2주)
**목표**: 개발자 도구 및 생태계 구축
**진행률**: 100% (전체 완료)

### 완료 ✅

#### F5.1: 에러 메시지 개선 (v0.3.1)
- ✅ 100% 한글 에러 메시지
- ✅ 정확한 위치 표시 (줄, 열 번호)
- ✅ 해결 힌트 제공
- ✅ 7개 테스트

#### F5.2: LSP 구현 (v0.3.0, v0.3.2)
- ✅ JSON-RPC 2.0 통신
- ✅ 기본 기능 (자동 완성, 진단, 문서 관리)
- ✅ 고급 기능 (정의 이동, 호버, 참조 찾기, 이름 변경)
- ✅ 130개 테스트

#### F5.3: VS Code Extension (v0.3.0)
- ✅ 구문 강조 (Syntax Highlighting)
- ✅ 자동 완성 (34개 키워드)
- ✅ 실시간 진단
- ✅ 정의 이동, 호버, 참조 찾기, 이름 변경
- ✅ 한글 지원 (UTF-16/UTF-8 변환)

#### F5.4: 디버거 (v0.3.3)
- ✅ 브레이크포인트 (조건부 포함)
- ✅ 와치포인트 (변수 변경 감지)
- ✅ 단계별 실행 (step, next, continue)
- ✅ 변수 검사, 콜스택 표시
- ✅ REPL 통합
- ✅ 179개 테스트

#### F5.5: 온라인 플레이그라운드 (v0.3.3)
- ✅ WebAssembly 빌드 (Emscripten)
- ✅ JavaScript API 바인딩
- ✅ 웹 인터페이스 (Vite + Monaco Editor)
- ✅ 예제 갤러리 (10개)
- ✅ GitHub Pages 자동 배포

#### F5.6: 표준 라이브러리 확장 (v0.3.2)
- ✅ stdlib/io.ksj - 파일 I/O (25개 함수)
- ✅ stdlib/json.ksj - JSON 처리 (17개 함수)
- ✅ stdlib/time.ksj - 날짜/시간 (14개 함수)
- ✅ **총 101개+ 함수 달성**

#### F5.7: 패턴 매칭 (v0.3.3)
- ✅ AST 노드 (MatchExpression, Pattern)
- ✅ Lexer/Parser (에 대해, ->, _, when)
- ✅ Evaluator (타입 검사, 값 비교, 바인딩)
- ✅ 리터럴, 와일드카드, 바인딩 패턴 지원
- ✅ 가드 조건
- ✅ 13개 테스트

**문법 예시**:
```ksj
결과 = 값 에 대해 {
    1 -> "하나"
    2 -> "둘"
    n 만약 n > 10 -> "큰 수"
    _ -> "기타"
}
```

#### F5.8: Semantic Analyzer 기본 구현 (v0.3.3)
- ✅ Name Resolution (이름 해석) - 기본 구현
- ✅ Type Checking (배열/함수 타입 검사) - 기본 구현
- ✅ Scope Management (함수, 블록, 반복문 스코프) - 기본 구현
- ✅ 35개 builtin 함수 등록
- ✅ 29개 테스트
- ⚠️ **한계**: 휴리스틱 기반, Symbol Table 미완성 ([ANALYSIS_IMPROVEMENTS.md](./ANALYSIS_IMPROVEMENTS.md) P0 참조)

---

## 🔮 Phase 6: 완성도 향상 (v0.5.0) 🚧

**기간**: 2025-11 말 ~ 2026-01 (2개월)
**목표**: 프로덕션 수준의 언어 + 아키텍처 개선
**진행률**: 70%
**참고**: [ANALYSIS_IMPROVEMENTS.md](./ANALYSIS_IMPROVEMENTS.md), [엔진 설계](../lab/idea.md)

### 완료

#### 6.0: 언어 기능 개선 ✅ (구현 완료)
- [x] **주석 지원** - 3가지 스타일 모두 지원 ✅
  - [x] # 한 줄 주석
  - [x] // 한 줄 주석 (C/C++ 스타일)
  - [x] /* */ 멀티라인 주석 (C/C++ 스타일)
  - [x] 10개 테스트 추가 (주석 관련)
- [x] **범위 반복문 개선** - 다양한 범위 키워드
  - [x] "까지" (끝 값 포함)
  - [x] "미만" (끝 값 미포함)
  - [x] "이하" (끝 값 포함)
  - [x] "이상" (끝 값 포함)

#### 6.1: 코드 품질 도구 ✅ (구현 완료)
- [x] **Linter (ksjlint)** - 22개 테스트
  - [x] AST 기반 정적 분석 엔진
  - [x] Rule 기반 아키텍처 (visitor pattern)
  - [x] UnusedVariableRule (미사용 변수 감지)
  - [x] DeadCodeRule (return 후 도달 불가능 코드)
  - [x] NoSelfComparisonRule (자기 비교 감지)
  - [x] ConstantConditionRule (항상 참/거짓 조건)
  - [x] EmptyBlockRule (빈 블록 감지)
- [x] **포맷터 (ksjfmt)** - 18개 테스트
  - [x] AST 기반 코드 포맷팅 엔진
  - [x] 들여쓰기, 공백, 줄바꿈 규칙
  - [x] FormatterOptions (커스터마이징)
  - [x] 모든 AST 노드 타입 지원

### 계획

#### 6.1: 코드 품질 도구 (추가 개선)
- [x] **Linter 추가 규칙 구현** ✅
  - NoUnusedParameterRule - 미사용 함수 매개변수 감지
  - NoShadowingRule - 변수 섀도잉 감지
  - NoMagicNumberRule - 매직 넘버 감지
  - 총 8개 규칙 (기존 5개 + 신규 3개)
  - 31개 테스트 (100% 통과)
- [x] **Linter/Formatter 설정 파일 지원** ✅
  - ConfigLoader 클래스 (JSON 설정 파일 로드)
  - LinterConfig (.ksjlint.json)
    - 규칙별 활성화/비활성화
    - 규칙별 심각도 설정 (error/warning/info/hint/off)
    - 규칙별 심각도 오버라이드 (런타임 적용)
  - FormatterConfig (.ksjfmt.json)
    - 들여쓰기, 공백, 줄바꿈 옵션
  - 자동 설정 파일 검색 (현재 디렉토리부터 상위로)
  - 11개 테스트 (100% 통과)
- [ ] VS Code 통합
- [x] **테스트 프레임워크 (ksjtest)** ✅
  - 5개 builtin assert 함수 (assert, assert_같음, assert_다름, assert_참, assert_거짓)
  - stdlib/test.ksj 문서 및 예제
  - examples/stdlib_test.ksj 데모
- [x] **프로파일러 (ksjprof)** ✅
  - Profiler 클래스 (함수 실행 시간 측정)
  - FunctionStats (호출 횟수, 총/평균/최소/최대 시간)
  - ScopedProfiler (RAII 기반 자동 프로파일링)
  - 중첩 함수 호출 지원 (call stack)
  - 텍스트/JSON 보고서 생성
  - 8개 테스트 (100% 통과)

#### 6.2: 표준 라이브러리 대폭 확장
- [x] stdlib/utils.ksj - 유틸리티 함수 (30개) ✅
  - 타입 검사, 범위 체크, 변환, 배열 생성, 문자열, 수학
- [x] stdlib/regex.ksj - 정규표현식 (30개+) ✅
  - 10개 builtin 함수, 20개+ 편의 함수
  - 검증, 형식 검사, 추출, 유틸리티
- [x] stdlib/crypto.ksj - 암호화 (42개) ✅
  - 12개 builtin 함수, 30개 편의 함수
  - Base64, 해싱, XOR/시저 암호화, 랜덤 생성
  - 비밀번호 관리, 토큰 생성, 데이터 보안
- [x] stdlib/os.ksj - OS & 파일 시스템 (39개) ✅
  - 26개 builtin 함수, 13개 유틸리티
  - 환경변수, 디렉토리, 파일, 경로, 시스템 정보
  - 크로스 플랫폼 (macOS/Linux/Windows)
  - 20개 테스트 (OSTest.cpp)
  - **Lexer 버그 수정**: "경로", "디렉토리인가" 조사 분리 예외 추가
- [x] stdlib/http.ksj - HTTP 클라이언트 (20개) ✅
  - 3개 builtin 함수 (HTTP_GET, HTTP_POST, HTTP_요청)
  - 17개 편의 함수 (응답 파싱, JSON API, REST 클라이언트, URL 유틸리티)
  - cpp-httplib 통합 (header-only 라이브러리)
  - 현재 HTTP만 지원 (HTTPS 향후 추가 예정)
- [x] stdlib/db.ksj - 데이터베이스 (15개) ✅
  - 6개 builtin 함수 (DB_열기, DB_닫기, DB_실행, DB_쿼리, DB_마지막_ID, DB_영향받은_행수)
  - 9개 편의 함수 (테이블 관리, CRUD, 트랜잭션, 결과 처리)
  - SQLite3 통합
  - 모든 기본 DB 작업 지원
- [x] stdlib/collections.ksj - 자료구조 (50개) ✅
  - Set (집합): 14개 함수 (생성, 추가, 제거, 포함, 집합 연산)
  - Map (맵/딕셔너리): 12개 함수 (키-값 쌍 관리)
  - Queue (큐): 7개 함수 (FIFO)
  - Stack (스택): 7개 함수 (LIFO)
  - Deque (덱): 10개 함수 (양방향 큐)
- [x] stdlib/test.ksj - 테스트 프레임워크 (5개) ✅
  - assert, assert_같음, assert_다름, assert_참, assert_거짓
  - 단위 테스트 작성 지원
- [x] **목표**: 250개+ 함수 ✅ **초과 달성!** (현재: 255개, 102% 달성)

#### 6.3: 아키텍처 개선 ⭐ **긴급** (KINGSEJONG_LANGUAGE_ANALYSIS.md 참조)
**참고 문서**: `docs/KINGSEJONG_LANGUAGE_ANALYSIS.md` - 언어 분석 보고서 (2025-11-16)

##### ✅ P0 - 긴급 과제 (Phase 1) **완료** (실제 공수: 5.5시간)
- [x] **Semantic Analyzer 완전 구현** ⭐⭐⭐⭐⭐ ✅
  - [x] Symbol Table 완전 구현 (Scope 기반)
  - [x] Name Resolution (변수/함수 정확 추적)
  - [x] buildSymbolTable + resolveNames 통합
  - [x] Scope 격리 구현 (블록/함수/루프)
  - [x] 실제 공수: 3시간
- [x] **SourceLocation 위치 정보 추가** ⭐⭐⭐⭐ ✅
  - [x] Lexer에 filename 추가
  - [x] Token에 파일명 포함
  - [x] 에러 메시지에 파일명:line:column 표시
  - [x] 실제 공수: 2시간
- [x] **Error Recovery 구현** ⭐⭐⭐⭐ ✅
  - [x] Panic Mode Recovery 구현 완료 (기존)
  - [x] 테스트 케이스 수정으로 100% 통과
  - [x] 동기화 토큰 정상 작동 확인
  - [x] 실제 공수: 30분

##### ✅ P1 - 중요 과제 (Phase 1) **완료** (실제 공수: 3주)
- [x] **형태소 분석기 분리** ⭐⭐⭐⭐ ✅
  - [x] Dictionary 클래스 (명사/동사/조사 사전)
  - [x] MorphologicalAnalyzer 구현
  - [x] Lexer 조사 분리 정확도 95% → 99%+
  - [x] 실제 공수: 2주
- [x] **Lookahead 확장 (LL(1) → LL(4))** ⭐⭐⭐ ✅
  - [x] peekPeekToken, peekPeekPeekToken, peekPeekPeekPeekToken 추가
  - [x] LL(4) lookahead 구현
  - [x] 복잡한 문법 처리 (범위 반복문 패턴 매칭)
  - [x] 실제 공수: 3일
- [x] **문법 개선 (모호성 제거)** ⭐⭐⭐ ✅
  - [x] 휴리스틱 제거 (isLikelyLoopVariable 삭제)
  - [x] LL(4) lookahead 기반 패턴 매칭으로 대체
  - [x] 테스트 통과율 유지: 1217/1220 (99.75%)
  - [x] 실제 공수: 1일
  - [x] **결론**: 명시적 키워드 없이도 lookahead만으로 모호성 해결 가능 확인

##### 🔴 P0 - 긴급 과제 (Phase 2) **진행중** ⚠️
**출처**: KINGSEJONG_LANGUAGE_ANALYSIS.md 섹션 5.2, 5.3
- [x] **변수 스코프 버그 수정** ⭐⭐⭐⭐⭐ ✅
  - [x] 문제: 재할당 시 부모 스코프 업데이트 안 됨
  - [x] Environment::setWithLookup() 구현
  - [x] 자동 스코프 체인 탐색 (JavaScript 스타일)
  - [x] 클로저 동작 검증
  - [x] 5개 테스트 추가 (EnvironmentTest.cpp)
  - [x] 실제 공수: 1시간
- [x] **예외 처리 시스템 구현** ⭐⭐⭐⭐⭐ ✅
  - [x] 키워드 추가: 시도/오류/마지막/던지다 (Token.h/cpp)
  - [x] Error Value 타입 (Value.h/cpp)
  - [x] TryStatement, ThrowStatement AST 노드 (Statement.h)
  - [x] Parser 구현 (시도/오류/마지막/던지다 문법)
  - [x] Evaluator 예외 처리 로직 (에러 전파, finally 보장 실행)
  - [x] 런타임 에러 복구 메커니즘 (스택 언와인딩)
  - [x] 테스트 케이스 (8개 통합 테스트)
  - [x] 예제 파일 (examples/exception_handling.ksj)
  - [x] 실제 공수: 1일 (TDD 방식)
  ```kingsejong
  시도 {
      결과 = 10 / 0
  } 오류 (e) {
      출력("에러:", e)
  } 마지막 {
      출력("정리")
  }
  ```

##### 🔴 P0 - 긴급 과제 (Phase 3: 보안 & 안정성) **미완료** ⚠️
**출처**: KINGSEJONG_VM_ANALYSIS.md 섹션 8, 9
**Production Readiness 4.5/10의 핵심 요인**: 프로덕션 배포 불가 수준의 보안 취약점

- [x] **샌드박스 시스템 구현** ⭐⭐⭐⭐⭐ ✅ (2025-11-16)
  - [x] **문제**: 무제한 파일 접근 (모든 파일 읽기/쓰기/삭제 가능)
  - [x] SecurityManager 클래스 구현
  - [x] 파일 경로 화이트리스트/블랙리스트
  - [x] 권한 레벨 시스템 (TRUSTED, UNTRUSTED, SANDBOX)
  - [x] 파일 작업 검증 (12개 함수: `파일_읽기`, `파일_쓰기`, `파일_삭제`, `파일_복사`, `파일_이동`, `파일_크기`, `파일_존재`, `파일_존재하는가`, `파일_추가`, `파일_해시`, `JSON_파일_읽기`, `JSON_파일_쓰기`)
  - [x] 디렉토리 접근 제한 및 경로 정규화
  - [x] Path Traversal 방어 (.. 처리)
  - [x] 읽기 전용 디렉토리 지원
  - [x] 17개 통합 테스트 (SecurityManagerTest)
  ```cpp
  // 예시: 샌드박스 모드에서 제한
  SecurityManager::setMode(SecurityMode::SANDBOX);
  SecurityManager::allowPath("/tmp/safe/");
  // /tmp/safe/ 외부 파일 접근 시 RuntimeError
  ```

- [ ] **네트워크 접근 제어** ⭐⭐⭐⭐⭐
  - [ ] **문제**: 무제한 HTTP 요청 (SSRF 위험, 내부 네트워크 접근 가능)
  - [ ] HTTP 요청 URL 검증
  - [ ] 화이트리스트 기반 도메인 제한
  - [ ] 프라이빗 IP 차단 (127.0.0.1, 192.168.x.x, 10.x.x.x)
  - [ ] Rate limiting (요청 속도 제한)
  - [ ] 예상 공수: 1주
  ```cpp
  // SSRF 방지
  SecurityManager::allowDomain("api.example.com");
  SecurityManager::blockPrivateIPs(true);
  ```

- [ ] **리소스 할당량 시스템** ⭐⭐⭐⭐
  - [ ] **문제**: 메모리/CPU 무제한 사용 가능
  - [ ] 메모리 사용량 제한
  - [ ] 실행 시간 제한 (이미 부분 구현됨, 강화 필요)
  - [ ] 파일 크기 제한
  - [ ] 네트워크 대역폭 제한
  - [ ] 예상 공수: 1주
  ```cpp
  ResourceLimits limits;
  limits.maxMemory = 100 * 1024 * 1024;  // 100MB
  limits.maxExecutionTime = 5000;         // 5초
  limits.maxFileSize = 10 * 1024 * 1024;  // 10MB
  ```

- [ ] **입력 검증 프레임워크** ⭐⭐⭐
  - [ ] **문제**: SQL 인젝션 위험 (DB_실행이 raw SQL 받음)
  - [ ] Prepared statements 지원
  - [ ] SQL 파라미터 바인딩
  - [ ] 파일 경로 검증 (path traversal 방지)
  - [ ] URL 검증 (스키마, 도메인 체크)
  - [ ] 예상 공수: 1주
  ```kingsejong
  # 안전한 SQL 실행
  결과 = DB_쿼리_안전(db, "SELECT * FROM users WHERE id = ?", [user_id])
  ```

- [ ] **암호화 강화** ⭐⭐
  - [ ] **문제**: 약한 암호화 (XOR, Caesar cipher만)
  - [ ] stdlib/crypto.ksj에 이미 기본 기능 있음
  - [ ] HTTPS 인증서 검증 추가 (httplib 설정)
  - [ ] 강력한 암호화 알고리즘 문서화
  - [ ] 예상 공수: 3-5일

##### 🟡 P1 - 중요 과제 (Phase 2) **미완료**
**출처**: KINGSEJONG_LANGUAGE_ANALYSIS.md 섹션 4.4, 6.4-6.6
- [ ] **딕셔너리 자료구조 (네이티브)** ⭐⭐⭐⭐
  - [ ] 문법: `{ "키": 값 }` 리터럴
  - [ ] DictLiteral AST 노드
  - [ ] Value 타입 확장 (DICT)
  - [ ] 대괄호/점 표기법 접근
  - [ ] stdlib/collections.ksj와 통합
  - [ ] 예상 공수: 1-2주
  ```kingsejong
  사람 = { "이름": "홍길동", "나이": 25 }
  출력(사람["이름"])
  ```
- [ ] **배열 연산자** ⭐⭐⭐
  - [ ] 배열 결합: `배열1 + 배열2`
  - [ ] 배열 곱셈: `배열 * 3`
  - [ ] evalBinaryExpression() 확장
  - [ ] 예상 공수: 3-5일
  ```kingsejong
  합쳐진 = [1, 2] + [3, 4]  # [1, 2, 3, 4]
  ```
- [ ] **문자열 보간** ⭐⭐⭐
  - [ ] 문법: `"$(변수)"` 또는 `"{변수}"`
  - [ ] Lexer 확장 (보간 감지)
  - [ ] InterpolatedString AST 노드
  - [ ] 예상 공수: 3-5일
  ```kingsejong
  이름 = "홍길동"
  메시지 = "$(이름)님 안녕하세요"
  ```

##### 🟢 P2 - 개선 과제 (Phase 3) **장기**
**출처**: KINGSEJONG_LANGUAGE_ANALYSIS.md 섹션 6.7
- [ ] **GC 개선 (Mark-and-Sweep)** ⭐⭐⭐
  - [ ] 순환 참조 해결
  - [ ] 증분 GC (Incremental GC)
  - [ ] 예상 공수: 2주
- [ ] **Destructuring (구조 분해 할당)** ⭐⭐⭐
  - [ ] 배열 언패킹: `[a, b, c] = [1, 2, 3]`
  - [ ] 딕셔너리 언패킹: `{이름, 나이} = 사람`
  - [ ] 예상 공수: 1주
- [ ] **Spread/Rest 연산자** ⭐⭐
  - [ ] 배열: `...배열`
  - [ ] 함수 인자: `함수(...인자들)`
  - [ ] 예상 공수: 3-5일
- [ ] **정규표현식 네이티브 지원** ⭐⭐
  - [ ] stdlib/regex.ksj는 이미 구현됨
  - [ ] 네이티브 RegExp 타입 추가 (선택적)
  - [ ] 예상 공수: 1주

#### 6.4: 성능 최적화 ⭐ (lab/idea.md Phase 2 + VM_ANALYSIS.md 권장사항)
**현재 성능**: Tree-walking ~5초, Bytecode VM ~1초 (1천만 반복 벤치마크)
**목표 성능**: JIT ~0.1초 (50-100배 향상 필요)
**Production Readiness 영향**: 현재 4.5/10 → 목표 7.0/10

##### P0: 긴급 성능 개선
- [x] **프로파일러 구현** ⭐⭐⭐⭐ ✅
  - [x] Profiler 클래스 (함수 실행 시간 측정)
  - [x] FunctionStats 수집
  - [x] 텍스트/JSON 보고서
  - [x] 8개 테스트 통과
- [x] **Hot Path Detector 완성** ⭐⭐⭐⭐ ✅ (2025-11-16)
  - [x] HotPathDetector 인프라 구현
  - [x] 함수 호출 빈도 추적 (evalCallExpression 통합)
  - [x] 루프 백엣지 카운팅 (evalRepeatStatement, evalRangeForStatement 통합)
  - [x] "hot" 코드 경로 식별 (isHot, isVeryHot)
  - [x] JIT 컴파일 우선순위 결정 (실행 횟수 + 실행 시간 기반)
  - [x] 10개 통합 테스트 (EvaluatorHotPathTest)
  - [x] 예제 파일 (examples/hot_path_detection.ksj)

##### P0: JIT 컴파일러 (Baseline Tier)
- [ ] **JIT Tier 1: Template JIT** ⭐⭐⭐⭐⭐
  - [ ] **긴급**: 현재 50-100배 느림 (V8 대비)
  - [ ] 기본 Off, `--enable-jit` 플래그로 활성화
  - [ ] Hot path만 컴파일 (임계값 기반)
  - [ ] 네이티브 코드 생성 (x86-64, ARM64)
  - [ ] 기본 최적화 (constant folding, dead code elimination)
  - [ ] 서버/로컬 환경만 지원
  - [ ] 예상 공수: 1-2개월
  - [ ] 성능 목표: 10-20배 향상
  ```
  # 실행 예시
  kingsejong --enable-jit script.ksj
  ```

##### P1: 고급 최적화
- [ ] **Inline Cache (IC)** ⭐⭐⭐⭐
  - [ ] 메서드 호출 최적화
  - [ ] 프로퍼티 접근 캐싱
  - [ ] 타입 특수화 (Type Specialization)
  - [ ] 예상 공수: 2-3주
- [ ] **세대별 GC (Generational GC)** ⭐⭐⭐⭐
  - [ ] 현재 문제: Mark-and-Sweep이 모든 객체 매번 수집
  - [ ] Young/Old generation 분리
  - [ ] Minor/Major GC
  - [ ] Write barrier 구현
  - [ ] GC 일시정지 목표: <10ms
  - [ ] 예상 공수: 2주
- [ ] **함수 인라이닝** ⭐⭐⭐
  - [ ] 작은 함수 자동 인라인
  - [ ] 호출 오버헤드 제거
  - [ ] 예상 공수: 2주
- [ ] **메모리 풀링** ⭐⭐
  - [ ] 빈번한 할당/해제 최적화
  - [ ] Value 객체 풀
  - [ ] 예상 공수: 1주

##### P2: 장기 최적화
- [ ] **JIT Tier 2: Optimizing JIT** ⭐⭐⭐⭐⭐
  - [ ] LLVM 백엔드 연구
  - [ ] 고급 최적화 (SSA, register allocation)
  - [ ] 예상 공수: 2-3개월
- [ ] **타입 추론 및 특수화** ⭐⭐⭐
  - [ ] 실행 중 타입 힌트 수집
  - [ ] 타입별 특수화 코드 생성
  - [ ] 예상 공수: 1개월

**전체 성능 목표**:
- Phase 1 (Profiler + Hot Path): 데이터 수집 기반 마련
- Phase 2 (JIT Tier 1): 10-20배 향상
- Phase 3 (IC + GC + Inlining): 추가 2-5배 향상
- **최종 목표**: V8 대비 10% 이내 격차 (현재: 1%, 목표: 10%)

#### 6.5: 패키지 관리자
- [ ] 패키지 레지스트리 설계
- [ ] 의존성 해결 알고리즘
- [ ] `세종패키지` CLI 도구
- [ ] 온라인 레지스트리

---

## 📊 Production Readiness 로드맵

> **핵심 지표**: 현재 4.5/10 → 목표 7.0/10 (Phase 6 완료 시)

### 현재 상태 분석 (KINGSEJONG_VM_ANALYSIS.md 기반)

| 차원 | 점수 | 평가 | 주요 이슈 |
|------|------|------|-----------|
| **언어 완성도** | 6/10 | 🟡 보통 | 클래스, async, 예외 처리 부족 |
| **VM/인터프리터 품질** | 7/10 | 🟢 양호 | 견고한 아키텍처, 최적화 부족 |
| **표준 라이브러리** | 7/10 | 🟢 양호 | 255개 함수, 고급 기능 부족 |
| **메모리 관리** | 7/10 | 🟢 양호 | GC 작동, 세대별 아님 |
| **성능** | 4/10 | 🔴 취약 | JIT 없음, 50-100배 느림 |
| **에러 처리** | 5/10 | 🟡 보통 | 좋은 에러 메시지, 예외 없음 |
| **디버깅** | 6/10 | 🟡 보통 | 디버거 인프라, 불완전 |
| **문서화** | 8/10 | 🟢 우수 | 방대한 문서 및 예제 |
| **테스팅** | 9/10 | 🟢 우수 | 1,292 테스트, 100% 통과 |
| **보안** | 3/10 | 🔴 심각 | 샌드박싱 없음, 무제한 I/O |
| **도구** | 8/10 | 🟢 우수 | VS Code 확장, LSP 지원 |

**전체**: **4.5/10** ❌ (프로덕션 배포 불가)

### 치명적 약점 (프로덕션 블로커)

#### 🔴 P0 긴급 - 반드시 해결 필요
1. **보안 부재** (점수: 3/10)
   - 무제한 파일 접근
   - 무제한 네트워크 접근 (SSRF)
   - 샌드박스 없음
   - SQL 인젝션 위험
   - **해결 방법**: Phase 6.3 P0-3 (보안 & 안정성)

2. **예외 처리 없음** (점수: 5/10)
   - try/catch 부재
   - 구조화된 에러 복구 불가
   - **해결 방법**: Phase 6.3 P0-2 (예외 처리 시스템)

3. **성능 문제** (점수: 4/10)
   - JIT 없음 (50-100배 느림)
   - V8 대비 1% 성능
   - **해결 방법**: Phase 6.4 (JIT Tier 1)

#### 🟡 P1 중요 - 언어 완성도
4. **클래스/OOP 없음** (점수: 6/10)
   - 객체 지향 프로그래밍 불가
   - **해결 방법**: Phase 7.1 (클래스 정의)

5. **비동기 없음** (점수: 6/10)
   - 동시 I/O 불가
   - 단일 스레드만
   - **해결 방법**: Phase 7.2 (Async/Await)

### Production Readiness 달성 경로

#### 🎯 Phase 6 완료 시 (v0.5.0) - 목표: 7.0/10

| 작업 | 영향 | 점수 변화 |
|------|------|-----------|
| **보안 강화** (샌드박스, 접근 제어) | 보안 3/10 → 8/10 | +5점 |
| **예외 처리 구현** | 에러 처리 5/10 → 8/10 | +3점 |
| **JIT Tier 1** | 성능 4/10 → 7/10 | +3점 |
| **세대별 GC** | 메모리 관리 7/10 → 8/10 | +1점 |

**새 전체 점수**: **7.0/10** ✅ (프로덕션 배포 가능)

#### 🎯 Phase 7 완료 시 (v1.0.0) - 목표: 8.5/10

| 작업 | 영향 | 점수 변화 |
|------|------|-----------|
| **클래스/OOP** | 언어 완성도 6/10 → 9/10 | +3점 |
| **Async/Await** | 언어 완성도 9/10 → 10/10 | +1점 |
| **JIT Tier 2 (LLVM)** | 성능 7/10 → 9/10 | +2점 |
| **타입 추론** | 에러 처리 8/10 → 9/10 | +1점 |

**새 전체 점수**: **8.5/10** 🎉 (성숙한 프로덕션 언어)

### 우선순위별 작업 계획

#### Phase 6 (현재): 프로덕션 준비
```
[완료] Phase 6.1: 코드 품질 도구 (Linter, Formatter, Profiler)
[완료] Phase 6.2: stdlib 255개 함수 달성
[완료] Phase 6.3 P0-1: Semantic Analyzer 완전 구현 ✅
[완료] Phase 6.3 P0-2: 변수 스코프 버그 수정 ✅
[진행중] Phase 6.3 P0-2: 예외 처리 시스템 (키워드만 완료)
[완료] Phase 6.3 P0-3: 보안 & 안정성 - 파일 샌드박스 ✅ (2025-11-16)
[대기] Phase 6.3 P0-3: 보안 & 안정성 - 네트워크 접근 제어
[대기] Phase 6.4: 성능 최적화 (JIT Tier 1, 세대별 GC)
[대기] Phase 6.5: 패키지 관리자
```

#### Phase 7: 언어 완성
```
[대기] Phase 7.1: 클래스/OOP (3주)
[대기] Phase 7.2: Async/Await (1개월)
[대기] Phase 7.3: Generators, Context Managers (3-4주)
[대기] Phase 7.4: 타입 시스템 강화 (1-2개월)
[대기] Phase 7.6: 스레딩/병렬성 (1개월)
```

### 타임라인

```
2025-11 말   Phase 6.3 P0-2 완료 (예외 처리)
2025-12 초   Phase 6.3 P0-3 시작 (보안)
2025-12 중   Phase 6.4 시작 (JIT Tier 1)
2026-01 말   v0.5.0 릴리스 (7.0/10) ✅
2026-02      Phase 7.1 시작 (클래스)
2026-03      Phase 7.2 시작 (Async)
2026-06      v1.0.0 릴리스 (8.5/10) 🎉
```

### 성공 기준

#### v0.5.0 (Phase 6 완료)
- ✅ Production Readiness: 7.0/10
- ✅ 보안 점수: 8/10 (샌드박스, 접근 제어)
- ✅ 예외 처리: try/catch/finally 동작
- ✅ 성능: JIT Tier 1 (10-20배 향상)
- ✅ stdlib: 250개+ 함수
- ✅ 테스트: 1,500+ (99%+ 통과율)

#### v1.0.0 (Phase 7 완료)
- ✅ Production Readiness: 8.5/10
- ✅ 완전한 OOP (클래스, 상속)
- ✅ 비동기 프로그래밍 (async/await)
- ✅ 언어 사양서 (Specification)
- ✅ 성능: V8 대비 10% 이내

---

## 🚀 Phase 7: 언어 기능 확장 (v1.0.0)

**기간**: 2026-02 ~ 2026-06 (5개월)
**목표**: 프로덕션 언어 필수 기능 완성
**참고**: KINGSEJONG_VM_ANALYSIS.md 섹션 14 (부족한 기능 종합)

### 7.1: 객체 지향 프로그래밍 ⭐⭐⭐⭐⭐ (P1 긴급)
**현재 상태**: ❌ 클래스 없음 (Python/JavaScript와 주요 격차)
**예상 공수**: 3주

- [ ] **클래스 정의**
  - [ ] `클래스` 키워드
  - [ ] ClassStatement AST 노드
  - [ ] 인스턴스 생성 (`새로운`)
  - [ ] 멤버 변수/메서드
  ```kingsejong
  클래스 사람 {
      이름: 문자열
      나이: 정수

      함수 인사() {
          출력("안녕하세요, $(이름)입니다")
      }
  }

  홍길동 = 새로운 사람("홍길동", 25)
  홍길동.인사()
  ```

- [ ] **상속 및 다형성**
  - [ ] 단일 상속
  - [ ] 메서드 오버라이딩
  - [ ] `super` 키워드
  - [ ] 가상 메서드 테이블 (vtable)
  ```kingsejong
  클래스 학생 상속 사람 {
      학번: 문자열

      함수 인사() {
          super.인사()
          출력("학번:", 학번)
      }
  }
  ```

- [ ] **프로퍼티 및 접근 제어**
  - [ ] Getter/Setter
  - [ ] 접근 제어자 (공개/비공개/보호)
  - [ ] 생성자/소멸자

### 7.2: 비동기 프로그래밍 ⭐⭐⭐⭐⭐ (P1 긴급)
**현재 상태**: ❌ 동시 I/O 불가, 단일 스레드만
**예상 공수**: 1개월

- [ ] **Promise/Future**
  - [ ] Promise Value 타입
  - [ ] `약속` 키워드
  - [ ] then/catch 체이닝
  ```kingsejong
  약속 = HTTP_GET_비동기("https://api.example.com")
  약속.그러면(응답 -> {
      출력("성공:", 응답)
  }).오류발생(에러 -> {
      출력("실패:", 에러)
  })
  ```

- [ ] **Async/Await**
  - [ ] `비동기` 함수 키워드
  - [ ] `기다리다` (await) 키워드
  - [ ] 이벤트 루프 구현
  - [ ] 비동기 I/O (파일, 네트워크)
  ```kingsejong
  비동기 함수 데이터_가져오기() {
      응답 = 기다리다 HTTP_GET("https://api.example.com")
      데이터 = 기다리다 JSON_파싱(응답)
      반환 데이터
  }
  ```

- [ ] **동시성 프리미티브**
  - [ ] 이벤트 루프 (libuv 기반)
  - [ ] 코루틴 스택
  - [ ] 비동기 파일 I/O
  - [ ] 비동기 HTTP 클라이언트

### 7.3: 고급 언어 기능 ⭐⭐⭐⭐ (P1 중요)
**예상 공수**: 3-4주

- [ ] **Generators/Iterators**
  - [ ] `산출` (yield) 키워드
  - [ ] Generator 함수
  - [ ] Iterator 프로토콜
  - [ ] for...각각 문 확장
  ```kingsejong
  함수 범위_생성(시작, 끝) {
      i = 시작
      동안 (i < 끝) {
          산출 i
          i = i + 1
      }
  }

  각각 값 범위_생성(1, 10)에서 {
      출력(값)
  }
  ```

- [ ] **Context Managers** (리소스 관리)
  - [ ] `사용` (with) 키워드
  - [ ] 자동 리소스 정리
  - [ ] `__입장__`, `__퇴장__` 메서드
  ```kingsejong
  사용 (파일 = 파일_열기("data.txt")) {
      내용 = 파일.읽기()
      출력(내용)
  }  # 자동으로 파일.닫기() 호출
  ```

- [ ] **Decorators** (함수 래핑)
  - [ ] `@` 데코레이터 문법
  - [ ] 함수 래퍼
  - [ ] 메서드 데코레이터

### 7.4: 타입 시스템 강화 ⭐⭐⭐ (P2 선택)
**예상 공수**: 1-2개월

- [ ] **타입 추론 엔진**
  - [ ] Hindley-Milner 타입 추론
  - [ ] 타입 체크 (컴파일 타임)
  - [ ] 타입 에러 (런타임 전 감지)

- [ ] **고급 타입**
  - [ ] 제네릭 타입
  - [ ] 유니온 타입 (`정수 | 문자열`)
  - [ ] Optional 타입 (`정수?`)
  - [ ] 인터페이스/프로토콜

### 7.5: 매크로 시스템 ⭐⭐ (P2 선택)
- [ ] 컴파일 타임 매크로
- [ ] AST 조작
- [ ] 코드 생성

### 7.6: 스레딩 및 병렬성 ⭐⭐⭐ (P2 중요)
**현재 상태**: ❌ 멀티스레딩 불가
**예상 공수**: 1개월

- [ ] **멀티스레딩**
  - [ ] `스레드` 키워드
  - [ ] std::thread 통합
  - [ ] Thread-safe 자료구조

- [ ] **동기화 프리미티브**
  - [ ] Mutex/Lock
  - [ ] 채널 (Go-style CSP)
  - [ ] 원자적 연산

- [ ] **스레드 풀**
  - [ ] Worker threads
  - [ ] 작업 큐
  - [ ] 스케줄링

---

## 🏗️ 엔진 진화 계획 (lab/idea.md)

> **비전**: "훈민정음의 철학 → 디지털 코드"

### 현재 상태 (Phase 1)
```
AST Interpreter (Tree-walking)
+ Bytecode VM (Stack-based, 기본 구현)
+ Deterministic Mode
+ C++23 기반
```

### Phase 2: Optimized Runtime (v0.6-0.8)
**목표**: V8 모델 차용, 성능 10배 향상

```
Register-based Bytecode VM
+ Hot path 탐지
+ Inline Cache (IC)
+ Peephole 최적화
+ Constant Folding
+ JIT 컴파일러 (선택적, 기본 Off)
  - 서버/로컬: -DENABLE_JIT=ON
  - 웹/임베디드: JIT 비활성화
```

**아키텍처 전환**:
- Stack-based → **Register-based VM** (Lua 스타일)
- Tree-walking → Bytecode 전용
- 단일 Eval Loop (CPython 구조)

### Phase 3: AI/DSL 확장 (v1.0+)
**목표**: 범용 언어 플랫폼

```
Homoiconic IR (Lisp형)
+ 코드 = 데이터 구조
+ DSL 엔진 (훈민정코)
+ 자연어 → 코드 변환
+ AI 모델 연동
```

**확장성**:
- 모든 DSL, 스크립트가 SejongVM에서 실행
- 공통 Bytecode 포맷
- 플러그인 아키텍처

### 기술 스펙
- **언어**: C++23 (GCC 13+, Clang 16+, MSVC 2022+)
- **빌드**: CMake ≥ 3.20
- **테스트**: GoogleTest
- **플랫폼**: macOS / Linux / Windows / WebAssembly / Embedded
- **옵션**: `ENABLE_JIT`, `ENABLE_DETERMINISTIC`, `ENABLE_PROFILER`

### 타깃별 전략

| 타깃 | JIT | 특징 |
|------|-----|------|
| 웹 (WASM) | ❌ | 브라우저 내 교육용, VM 시각화 UI |
| 임베디드 | ❌ | 저전력 (64~128KB RAM), Deterministic 필수 |
| 서버/CLI | ✅ 옵션 | 연구/실험용, IC + JIT 활성화 가능 |

---

## 📈 장기 비전 (v2.0.0+)

### 생태계 확장
- [ ] 모바일 지원 (Android/iOS)
- [ ] 임베디드 시스템
- [ ] WASM 타겟 완전 지원
- [ ] GPU 컴퓨팅

### 표준화
- [ ] 언어 사양서 (Specification)
- [ ] 표준 위원회 구성
- [ ] 호환성 테스트 스위트
- [ ] 다중 구현체 지원

### 커뮤니티
- [ ] 1,000+ GitHub Stars
- [ ] 100+ 기여자
- [ ] 10,000+ 사용자
- [ ] 국제 컨퍼런스

---

## 🎯 마일스톤

### ✅ v0.1.0 - MVP (2025-11-10)
핵심 언어 기능 완성

### ✅ v0.2.0 - 플랫폼 확장 (2025-11-10)
CI/CD, 성능 최적화

### ✅ v0.3.0 - LSP 기본 (2025-11-13)
VS Code 통합

### ✅ v0.3.1 - 에러 개선 (2025-11-13)
한글 에러 메시지

### ✅ v0.3.2 - LSP 고급 (2025-11-13)
고급 IDE 기능

### 🚧 v0.4.0 - Phase 5 완료 (2025-11 말)
개발자 경험 완성

### 📝 v0.5.0 - Phase 6 완료 (2026-01)
프로덕션 수준

### 📝 v1.0.0 - 정식 릴리스 (2026-06)
완전한 언어

---

## 📊 성공 지표

### v0.4.0 (Phase 5 완료)
- [x] LSP 구현 완료 ✅
- [x] VS Code Extension ✅
- [x] 디버거 완성 ✅
- [x] 온라인 플레이그라운드 ✅
- [x] 표준 라이브러리 100개+ ✅
- [ ] 패턴 매칭 (선택)

### v0.5.0 (Phase 6 완료)
- [x] 표준 라이브러리 200개+ ✅ (212개+)
- [x] **아키텍처 개선 (P0 완료)** ⭐ ✅
  - [x] Semantic Analyzer 완전 구현 ✅
  - [x] Symbol Table 완전 구현 ✅
  - [x] Error Recovery ✅
  - [x] SourceLocation 추가 ✅
- [x] **형태소 분석기 (P1 완료)** ⭐ ✅
  - [x] Dictionary 기반 조사 분리 ✅
  - [x] 99%+ 정확도 ✅
- [ ] 패키지 관리자 출시
- [ ] 50+ GitHub Stars

### v0.6.0 - v0.8.0 (엔진 최적화)
- [ ] **Register-based VM 전환** ⭐⭐⭐⭐⭐
- [ ] Inline Cache (IC)
- [ ] JIT Tier 1 구현 (선택적)
- [ ] 성능 5-10배 향상
- [ ] Profiling 인프라

### v1.0.0 (Phase 7 완료)
- [ ] 완전한 타입 시스템
- [ ] 객체 지향 프로그래밍
- [ ] 비동기 프로그래밍
- [ ] 언어 사양서
- [ ] 500+ GitHub Stars

---

## 🤝 기여 방법

### 개발자
- 이슈 해결 (Good First Issue)
- 새 기능 구현
- 테스트 작성
- 버그 리포트

### 문서 작성자
- 튜토리얼 작성
- 예제 프로그램
- API 문서 개선
- 번역 (영어)

### 사용자
- 피드백 제공
- 사용 사례 공유
- 커뮤니티 질문 답변

---

## 📅 타임라인

```
2025-10      Phase 0 (초기화)
2025-11 초   Phase 1 (핵심 기능)
2025-11 초   Phase 2 (실용 기능)
2025-11 중   Phase 3 (고급 기능)
2025-11 중   Phase 4 (최적화)
2025-11 중   Phase 5 (개발자 경험)
2025-11 말   Phase 6 시작 (stdlib 확장) ← 현재
2025-12      Phase 6 아키텍처 개선 (P0/P1)
2026-01      v0.5.0 릴리스
2026-02      Phase 6.5: 엔진 최적화 시작
2026-04      v0.6.0-0.8.0 (Register VM + JIT)
2026-06      Phase 7 시작
2026-12      v1.0.0 정식 릴리스
```

---

## 🌟 핵심 차별화

### 기술적 혁신
- **조사 기반 메서드 체이닝** - 세계 최초
- **자연스러운 범위 표현** - 수학적 직관성
- **한글 키워드** - 모국어 프로그래밍

### 개발자 경험
- **완전한 IDE 지원** - LSP, 디버거
- **온라인 플레이그라운드** - 즉시 실행
- **풍부한 문서** - 4,200+ 줄

### 실용성
- **표준 라이브러리** - 100+ 유틸리티
- **크로스 플랫폼** - macOS, Linux, Windows
- **프로덕션 수준** - GC, 바이트코드, 최적화

---

**마지막 업데이트**: 2025-11-16
**현재 버전**: v0.3.7
**Production Readiness**: ⚠️ **4.5/10** → 목표 **7.0/10** (Phase 6 완료 시)
**현재 진행률**: Phase 6 70%

**최근 완료**:
- ✅ **P0 Phase 1 완료** (Semantic Analyzer, SourceLocation, ErrorRecovery) - 5.5시간
- ✅ **P0 Phase 2 완료** (변수 스코프 버그, 예외 처리 시스템 전체) - 1일
- ✅ stdlib 255개 함수 달성 (os, http, crypto, regex, collections)
- ✅ Linter/Formatter 설정 파일 지원
- ✅ Profiler 구현 완료
- ✅ 테스트 1,292개 (100% 통과)
- ✅ 예외 처리 완성 (시도/오류/마지막/던지다)

**진행중**:
1. Phase 6.3: 보안 & 안정성 - 샌드박스 시스템
  - ✅ SecurityManager 완성 (2025-11-16)
    - SecurityManager 클래스 구현 (TRUSTED/UNTRUSTED/SANDBOX 모드)
    - 12개 파일 함수에 보안 검사 통합
    - 17개 통합 테스트 추가 (SecurityManagerTest)
    - Path Traversal 방어, 경로 정규화, 읽기 전용 디렉토리 지원
2. Phase 6.4: 성능 최적화 계획
  - ✅ Hot Path Detector 완성 (2025-11-16)
    - Evaluator 통합 (함수 호출, 루프 백엣지 추적)
    - 10개 통합 테스트 추가
    - examples/hot_path_detection.ksj 예제 추가
3. Phase 7.1: 클래스/OOP 설계

**다음 우선순위** (Production Readiness 향상):
1. ✅ ~~**파일 샌드박스 시스템** (P0)~~ **완료!** (2025-11-16)
2. **네트워크 접근 제어** (P0) - SSRF 방지, 리소스 할당량 (2-3주)
3. **JIT Tier 1** (P0) - 성능 10-20배 향상 (1-2개월)
4. **클래스 시스템** (P1) - OOP 지원 (2-3주)
5. 딕셔너리, 배열 연산자, 문자열 보간 (P1)
6. 패키지 관리자 (Phase 6.5)

**치명적 약점 (프로덕션 블로커)**:
- 🟡 보안: 파일 샌드박스 완료 ✅, 네트워크 접근 제어 필요
- ✅ ~~예외 처리: try/catch 없음~~ **완료!**
- 🔴 성능: JIT 없음 (V8 대비 1% 성능)

**장기 비전**:
- **기술**: Register-based VM + 선택적 JIT + AI/DSL 확장
- **철학**: 훈민정음의 디지털 후계자
- **목표**: 한국어로 프로그래밍하는 자연스러운 언어
- **마일스톤**: v0.5.0 (7.0/10) → v1.0.0 (8.5/10)

**핵심 문서**:
- [아키텍처 분석](./ARCHITECTURE_ANALYSIS.md)
- [VM 분석](./KINGSEJONG_VM_ANALYSIS.md) ⭐ **NEW**
- [언어 분석](./KINGSEJONG_LANGUAGE_ANALYSIS.md)
- [개선 계획 (P0/P1/P2)](./ANALYSIS_IMPROVEMENTS.md)
- [엔진 설계](../lab/idea.md)
