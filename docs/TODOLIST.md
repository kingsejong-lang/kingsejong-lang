# KingSejong 언어 개발 작업 목록

> **프로젝트**: KingSejong Programming Language (`.ksj`)
> **최종 업데이트**: 2025-11-15
> **현재 버전**: v0.3.3 (Phase 5 완료)
> **상태 기호**: ✅ 완료 | 🚧 진행중 | 📝 예정 | ⏸️ 보류

---

## 📊 전체 진행 상황

### 완료된 Phase

#### Phase 0: 프로젝트 초기화 ✅ (100%)
- 프로젝트 구조, 테스트 프레임워크, 코딩 표준

#### Phase 1: 핵심 차별화 기능 ✅ (100%)
- Token, JosaRecognizer, Lexer, Parser, AST
- Evaluator, Value, Environment
- 조사 체인, 범위 표현식, 배열 리터럴
- 1급 함수, 클로저, 재귀
- 반복문, 조건문
- REPL, 파일 실행

#### Phase 2: 실용 기능 ✅ (100%)
- 배열 구현 (인덱싱, 슬라이싱, 메서드)
- 함수형 프로그래밍 (filter, map, reduce, find)
- 문자열, 실수 타입
- 에러 처리, 한글 에러 메시지
- 타입 검사
- 표준 라이브러리 기초 (math, string, array)

#### Phase 3: 고급 기능 ✅ (100%)
- 가비지 컬렉터 (Mark & Sweep)
- 바이트코드 컴파일러 (60+ OpCode, 스택 VM)
- 모듈 시스템 (import, 캐싱, 순환 참조 방지)

#### Phase 4: 최적화 및 안정화 ✅ (100%)
- CI/CD 파이프라인 (macOS, Linux, Windows)
- 성능 벤치마크 시스템
- 컴파일러 최적화 (상수 폴딩, 데드 코드 제거)
- 메모리 안전성 (ASan, UBSan, Valgrind)
- JIT 컴파일러 연구 (HotPathDetector)

#### Phase 5: 개발자 경험 & 생태계 ✅ (100%)
- ✅ 에러 메시지 개선 (한글화, 위치 표시, 힌트)
- ✅ LSP 구현 (정의 이동, 호버, 참조 찾기, 이름 변경)
- ✅ VS Code Extension (구문 강조, 자동 완성, 진단)
- ✅ 디버거 (브레이크포인트, 단계 실행, 변수 검사) - 179개 테스트
- ✅ 온라인 플레이그라운드 (WebAssembly, Monaco Editor)
- ✅ 표준 라이브러리 확장 (io, json, time) - 101개+ 함수
- ✅ 패턴 매칭 (리터럴, 와일드카드, 바인딩, 가드) - 13개 테스트
- ✅ Semantic Analyzer (Name Resolution, Type Checking, Scope) - 29개 테스트

---

## 🎯 현재 상태 (v0.3.6)

### Phase 5 완료 ✅
- ✅ LSP, 디버거, 플레이그라운드
- ✅ 패턴 매칭
- ✅ Semantic Analyzer 완성

### Phase 6 진행중 🚧
- ✅ 언어 기능 개선
  - 주석 지원 (# 단일 라인 주석)
  - 범위 반복문 개선 (까지/미만/이하/이상 키워드)
- ✅ Linter 완전 구현 (5개 규칙, 22개 테스트)
  - UnusedVariableRule, DeadCodeRule, NoSelfComparisonRule
  - ConstantConditionRule, EmptyBlockRule
- ✅ Formatter 기본 구현 완료 (18개 테스트)
  - AST 기반 코드 재포맷팅, 들여쓰기/공백 규칙
- 🚧 stdlib 대폭 확장 (200개+ 함수)
  - ✅ stdlib/utils.ksj - 유틸리티 함수 (30개)
  - ✅ stdlib/regex.ksj - 정규표현식 (30개+)
  - ✅ stdlib/crypto.ksj - 암호화 (42개)
- 📝 JIT 컴파일러 Tier 1 예정

### 전체 테스트
- ✅ **1160개 테스트 통과** (Phase 5: 1075개 + Linter: 22개 + Formatter: 18개 + Regex: 20개 + Crypto: 25개)

---

## 📝 다음 단계 (Phase 6: 완성도 향상)

### 6.0: 언어 기능 개선 ✅
- [x] 주석 지원 (# 단일 라인 주석)
  - [x] Lexer에 skipComment() 구현
  - [x] nextToken()에서 주석 자동 건너뛰기
- [x] 범위 반복문 개선
  - [x] "까지" (끝 값 포함)
  - [x] "미만" (끝 값 미포함)
  - [x] "이하" (끝 값 포함)
  - [x] "이상" (끝 값 포함)
  - [x] RangeForStatement에 endInclusive 필드 추가
  - [x] Evaluator에서 조건 분기 구현

### 6.1: 코드 품질 도구 (4주)

#### Linter (ksjlint) - 2주 ✅
- [x] AST 기반 정적 분석
- [x] 규칙 엔진 설계 (Rule base class, visitor pattern)
- [x] 기본 규칙 구현 - 22개 테스트
  - [x] UnusedVariableRule (미사용 변수 감지)
  - [x] DeadCodeRule (return 후 도달 불가능 코드)
  - [x] NoSelfComparisonRule (자기 비교 감지)
  - [x] ConstantConditionRule (항상 참/거짓 조건)
  - [x] EmptyBlockRule (빈 블록 감지)
- [ ] 설정 파일 (.ksjlintrc)
- [ ] 추가 규칙 (네이밍 컨벤션 등)

#### Formatter (ksjfmt) - 2주 ✅
- [x] AST 기반 코드 포맷팅 엔진
- [x] 들여쓰기 규칙 (커스터마이징 가능)
- [x] 공백 규칙 (연산자 주변, 콤마 뒤 등)
- [x] 줄바꿈 및 블록 포맷팅
- [x] FormatterOptions (들여쓰기 크기, 스페이스/탭 선택)
- [x] 18개 테스트 (기본 포맷팅, 블록 구조, 옵션 테스트)
- [ ] VS Code 통합
- [ ] 설정 파일 (.ksjfmtrc)

### 6.2: 표준 라이브러리 확장 (4주)
- [x] stdlib/utils.ksj - 유틸리티 함수 (30개)
  - [x] 타입 검사 (정수인가, 실수인가, 문자열인가, 배열인가, 함수인가)
  - [x] 범위 체크 (범위_안에, 양수인가, 음수인가, 짝수인가, 홀수인가)
  - [x] 변환 함수 (정수를_문자로, 참거짓을_문자열로)
  - [x] 조건 함수 (선택)
  - [x] 배열 생성 (정수_범위, 범위_단계, 채우기)
  - [x] 문자열 함수 (문자열_반복, 비어있는가)
  - [x] 수학 유틸리티 (절댓값, 최댓값, 최솟값, 제한, 교환)
  - [x] examples/stdlib_utils.ksj 예제 추가
- [x] stdlib/regex.ksj - 정규표현식 (30개+)
  - [x] builtin 함수 10개 (일치, 검색, 모두_찾기, 치환, 분리, 추출, 개수)
  - [x] 검증 함수 (이메일, URL, 전화번호, 날짜, 시간)
  - [x] 형식 검사 (숫자, 영문, 한글, 영문숫자)
  - [x] 유틸리티 (공백, 특수문자, 추출, 비밀번호 강도)
  - [x] 20개 테스트, examples/stdlib_regex.ksj 예제 추가
- [x] stdlib/crypto.ksj - 암호화 (42개)
  - [x] builtin 함수 12개 (Base64, 해시, XOR/시저 암호화, 랜덤)
  - [x] 비밀번호 관리 (해싱, 검증, 강도 검사)
  - [x] 토큰 생성 (API, OTP, PIN, UUID)
  - [x] 데이터 보안 (서명, 안전한 저장/복원)
  - [x] 25개 테스트, examples/stdlib_crypto.ksj 예제 추가
- [ ] stdlib/http.ksj - HTTP 클라이언트/서버 (20개)
- [ ] stdlib/db.ksj - 데이터베이스 (15개)
- [ ] **목표**: 200개+ 함수 (현재: 173개+)

### 6.3: 성능 최적화 (4주)
- [ ] JIT 컴파일러 Tier 1 (템플릿 JIT)
- [ ] 증분 GC (Incremental GC)
- [ ] 메모리 풀링
- [ ] **목표**: 2-5배 성능 향상

---

## 📊 테스트 현황

```
총 테스트: 1075개
통과율: 100%

주요 테스트:
- Lexer/Parser: 60개
- Evaluator: 30개
- Type System: 48개
- Array: 62개
- ArrayFunctionType: 16개 (NEW)
- Float: 48개
- String: 45개
- Function: 13개
- Loop: 21개
- GC: 15개
- Bytecode: 19개
- LSP: 130개
- 디버거: 179개
- 패턴 매칭: 13개
- Name Resolution: 13개 (NEW)
- 기타: 363개
```

---

## 🎯 마일스톤

### ✅ v0.1.0 - MVP (2025-11-10)
- 핵심 언어 기능
- 표준 라이브러리 기초
- 가비지 컬렉터
- 바이트코드 컴파일러

### ✅ v0.2.0 - 플랫폼 확장 (2025-11-10)
- CI/CD (macOS, Linux, Windows)
- 성능 최적화
- 메모리 안전성

### ✅ v0.3.0 - LSP 기본 (2025-11-13)
- Language Server Protocol
- VS Code Extension
- 자동 완성, 진단

### ✅ v0.3.1 - 에러 개선 (2025-11-13)
- 한글 에러 메시지
- 위치 표시
- 해결 힌트

### ✅ v0.3.2 - LSP 고급 (2025-11-13)
- 정의 이동
- 호버 정보
- 참조 찾기
- 이름 변경

### 📝 v0.4.0 - Phase 5 완료 (예정)
- 디버거 완성
- 온라인 플레이그라운드
- 표준 라이브러리 확장
- 패턴 매칭 (선택)

---

## 📚 문서화 현황

### 완료된 문서 ✅
- [x] README.md - 프로젝트 개요 및 빠른 시작
- [x] docs/TUTORIAL.md - 초보자 튜토리얼 (650줄, 10 챕터)
- [x] docs/LANGUAGE_REFERENCE.md - 완전한 문법 가이드 (679줄)
- [x] docs/LIBRARY_GUIDE.md - 모듈 개발 가이드 (537줄)
- [x] docs/GC_DESIGN.md - 가비지 컬렉터 설계 문서
- [x] docs/INSTALL.md - 플랫폼별 설치 가이드 (630줄)
- [x] stdlib/README.md - 표준 라이브러리 레퍼런스
- [x] vscode-extension/README.md - VS Code Extension 가이드
- [x] 21개 예제 프로그램

### 진행 중 🚧
- [ ] 디버거 사용 가이드
- [ ] 플레이그라운드 튜토리얼
- [ ] 패턴 매칭 가이드 (선택)

---

## 🔧 개발 가이드

### 빌드 및 테스트

```bash
# 빌드
mkdir build && cd build
cmake ..
make -j8

# 테스트
ctest --output-on-failure

# 메모리 검사
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -g" ..
make && ctest
```

### 커밋 규칙

```
feat: 새 기능 추가
fix: 버그 수정
refactor: 코드 리팩토링
test: 테스트 추가
docs: 문서 수정
perf: 성능 개선
```

**중요**: Co-Authored-By 라인이나 Generated-by 라인 **절대 포함 금지**

---

## 📈 성과 지표

### 코드
- **총 코드**: 17,400+ 줄
- **테스트**: 1075개 (100% 통과)
- **문서**: 4,200+ 줄
- **예제**: 21개

### 성능
- **테스트 실행**: ~160ms
- **메모리 안전성**: ASan/UBSan 클린
- **플랫폼 지원**: macOS, Linux, Windows

### 커뮤니티
- **GitHub Stars**: (진행 중)
- **기여자**: 1명
- **예제 프로그램**: 21개

---

## 🚀 향후 계획

### v0.5.0 (2026 Q1)
- 패키지 관리자
- 표준 라이브러리 200개+ 함수
- 성능 최적화 (JIT Tier 1)

### v1.0.0 (2026 Q2)
- 언어 사양서 (Specification)
- 다중 구현체 지원
- 국제화 (i18n)

---

**마지막 업데이트**: 2025-11-15
**현재 상태**: Phase 6 진행 중 (55%)
**다음 우선순위**: stdlib 확장 계속 (http → db → 성능 최적화)
