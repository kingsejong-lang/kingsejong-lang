# Changelog

All notable changes to the KingSejong Programming Language will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.2.0] - 2025-11-11

### Added
- **타입 추론 (Type Inference)**: 변수 선언 시 타입을 생략할 수 있는 기능 추가
  - `x = 10` (정수로 자동 추론)
  - `이름 = "홍길동"` (문자열로 자동 추론)
  - 기존 명시적 타입 선언도 계속 지원
- **명명 함수 선언 구문**: `함수 이름(매개변수) { ... }` 문법 지원
  - 예: `함수 원의_넓이(반지름) { 반환 3.14 * 반지름 * 반지름 }`
- **타입 키워드 함수 이름 사용**: `정수()`, `실수()`, `문자열()` 등을 함수 이름으로 사용 가능
  - 타입 변환 함수로 활용 가능
- **플랫폼별 설치 가이드**: macOS, Linux (Ubuntu, Fedora, Arch), Windows 상세 가이드
  - 자동화 설치 스크립트 (install.sh, install.ps1)
  - docs/INSTALL.md 630+ 줄 추가
- **JIT 컴파일러 연구**: 핫 패스 감지 시스템 구현 (Phase 5 준비)
  - HotPathDetector 클래스 (함수 호출 추적, 루프 백엣지 추적)
  - LLVM 백엔드 통합 연구 문서
  - 21개 테스트 추가

### Fixed
- **모든 테스트 통과 (567/567)**: 100% 테스트 통과율 달성
  - FloatTest.ShouldRoundNegativeFloat: 반올림 함수 수정 (floor(value + 0.5))
  - FloatTest.ShouldCompareFloatAndInteger: 정수↔실수 비교 지원 추가
  - FloatTest.ShouldUseFloatInFunction: 명명 함수 구문 파싱 수정
  - ModuleTestFixture.ShouldLoadModuleWithFunctions: heap-use-after-free 수정 (AST 수명 관리)
- **바이트코드 컴파일러 버그 수정 (23개 테스트)**:
  - Parser 생성자 오류 수정
  - 문자열 연결 오류 수정
  - AST API 변경 대응
- **메모리 안전성 개선**:
  - GCTest heap-use-after-free 버그 5건 수정
  - AddressSanitizer (ASan) 클린
  - UndefinedBehaviorSanitizer (UBSan) 클린
- **Lexer 조사 분리 개선**: 언더스코어(_)나 숫자 뒤의 한글은 조사로 분리하지 않음
  - 예: "원의_넓이"가 "원의_넓" + "이"로 잘못 분리되던 문제 수정
- **Value 비교 연산 개선**: 정수와 실수 간 자동 타입 변환 지원
  - `3.5 > 3` → `true`
  - `3.0 == 3` → `false` (equality는 타입 엄격)

### Changed
- **반올림 함수 동작 변경**: "round half up" 방식으로 변경
  - 이전: `std::round(-2.5)` → `-3`
  - 현재: `floor(value + 0.5)` → `-2`
- **ModuleLoader AST 수명 관리**: ModuleCacheEntry 도입으로 RAII 준수
  - Function 객체가 AST body에 대한 raw pointer를 안전하게 참조

### Performance
- **컴파일러 최적화**: 상수 폴딩, 데드 코드 제거, 피홀 최적화
- **벤치마크 시스템**: 4개 벤치마크 프로그램 및 자동화 스크립트
- **CI 벤치마크 자동화**: GitHub Actions에서 자동 성능 측정

### Documentation
- README.md: 테스트 현황 업데이트 (567개 통과)
- TODOLIST.md: Phase 1-4 완료 상태 반영
- docs/research/JIT_COMPILER_RESEARCH.md: 1000+ 줄 JIT 연구 문서
- docs/research/LLVM_INTEGRATION.md: 800+ 줄 LLVM 통합 분석
- docs/INSTALL.md: 630+ 줄 플랫폼별 설치 가이드

## [0.1.0] - 2025-11-10

### Added
- **핵심 차별화 기능 (Phase 1)**:
  - Token 시스템 (60+ 토큰 타입)
  - JosaRecognizer (한글 조사 자동 인식)
  - Lexer (UTF-8 한글 처리)
  - Parser (Pratt Parsing 알고리즘)
  - AST (26가지 노드 타입)
  - Evaluator (Value 시스템, Environment)
  - 1급 함수 (클로저, 재귀)
  - REPL 및 파일 실행
  - 10개 예제 프로그램

- **실용 기능 (Phase 2)**:
  - 배열 구현 (리터럴, 인덱싱, 슬라이싱)
  - 함수형 메서드 (걸러낸다, 변환한다, 축약한다, 찾다)
  - 메서드 체이닝 (정렬한다, 역순으로_나열한다)
  - 문자열 타입
  - 실수 타입
  - 에러 처리 시스템
  - 한글 에러 메시지
  - 타입 검사
  - 표준 라이브러리 (math, string, array - 45개 함수)

- **고급 기능 (Phase 3)**:
  - 모듈 시스템 (가져오기, 캐싱, 순환 참조 방지)
  - 가비지 컬렉터 (Mark & Sweep, 순환 참조 감지)
  - 바이트코드 컴파일러 (60+ OpCode, 스택 기반 VM)

- **최적화 및 안정화 (Phase 4)**:
  - CI/CD 파이프라인 (macOS, Linux, Windows)
  - 바이트코드 함수 지원 (클로저, 재귀)
  - 성능 벤치마크 시스템
  - 컴파일러 최적화 패스

### Documentation
- 완전한 언어 레퍼런스 (679줄)
- 초보자 튜토리얼 (650줄, 10개 챕터)
- 라이브러리 확장 가이드 (537줄)
- 21개 예제 프로그램
- GC 설계 문서

### Testing
- 567개 테스트 (100% 통과)
- AddressSanitizer/UndefinedBehaviorSanitizer 통합
- Valgrind 메모리 검사

---

## Release Notes

### v0.2.0 주요 개선사항
이번 릴리스는 **타입 추론**, **메모리 안전성**, **플랫폼 지원** 개선에 초점을 맞췄습니다.

**새로운 기능:**
- 타입 추론으로 더 간결한 코드 작성 가능
- 명명 함수 선언 구문으로 가독성 향상
- 플랫폼별 자동화 설치 스크립트

**안정성:**
- 모든 567개 테스트 통과 (100%)
- 메모리 안전성 검증 완료 (ASan/UBSan)
- heap-use-after-free 버그 완전 수정

**성능:**
- JIT 컴파일러 연구 완료 (Phase 5 준비)
- 컴파일러 최적화 패스 적용
- 벤치마크 자동화

### v0.1.0 주요 특징
한글 프로그래밍 언어의 기반을 완성했습니다.

**핵심 차별화:**
- 조사 기반 메서드 체이닝 (`배열을 정렬한다`)
- 자연스러운 범위 표현 (`1부터 10까지`)
- N번 반복 구문 (`5번 반복한다`)

**실용 기능:**
- 함수형 프로그래밍 (filter, map, reduce)
- 모듈 시스템 (45개 표준 라이브러리 함수)
- 가비지 컬렉터 (자동 메모리 관리)

**개발 환경:**
- REPL (대화형 실행)
- 3개 플랫폼 지원 (macOS, Linux, Windows)
- 완전한 문서화 (튜토리얼, 레퍼런스, 예제)

[Unreleased]: https://github.com/0xmhha/kingsejonglang/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/0xmhha/kingsejonglang/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/0xmhha/kingsejonglang/releases/tag/v0.1.0
