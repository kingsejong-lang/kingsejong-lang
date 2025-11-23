# Changelog

모든 주목할 만한 변경 사항이 이 파일에 문서화됩니다.

형식은 [Keep a Changelog](https://keepachangelog.com/ko/1.0.0/)를 기반으로 하며,
이 프로젝트는 [Semantic Versioning](https://semver.org/lang/ko/)을 준수합니다.

## [v0.5.0] - 2025-11-23

### 🎉 주요 개선사항

이번 릴리스는 대규모 코드 리팩토링, Playground 개선, 그리고 코드 품질 향상에 초점을 맞췄습니다.

**Production Readiness: 8.9/10**

### ✨ 새로운 기능

#### Playground 개선
- **17개 템플릿 지원** (기본 9개 + stdlib 8개)
  - 기본 템플릿: Hello World, 피보나치, 함수, 함수형 배열, 계산기, 클래스, 비동기, 배열, 반복문
  - stdlib 템플릿: JSON, 시간/날짜, 파일 I/O, 유틸리티, HTTP, 데이터베이스, 정규표현식, 암호화
- 코드 공유 기능 (URL 기반)
- 실행 결과 다운로드 기능
- 클립보드 복사 기능

#### 표준 라이브러리 확장 (251개 함수)
- **stdlib/collections.ksj** (49개 함수) - 리스트, 맵, 집합 자료구조 및 고급 연산
- **stdlib/http.ksj** (21개 함수) - HTTP 클라이언트, REST API 호출
- **stdlib/db.ksj** (14개 함수) - SQLite 데이터베이스 연동
- **stdlib/regex.ksj** (19개 함수) - 정규표현식 매칭 및 처리
- **stdlib/crypto.ksj** (23개 함수) - 암호화, 해싱, 서명 검증
- **stdlib/os.ksj** (24개 함수) - OS 및 파일 시스템 조작
- 기존 stdlib/json.ksj, stdlib/time.ksj, stdlib/io.ksj, stdlib/utils.ksj 유지

#### 코드 품질 도구
- **.ksjlintrc** 설정 파일 지원 (8개 규칙)
- **.ksjfmtrc** 설정 파일 지원 (6개 옵션)
- CONFIG_GUIDE.md - 상세 설정 가이드
- VS Code 통합 가이드
- CI/CD 통합 예제 (GitHub Actions, pre-commit hook)

### 🔧 개선사항

#### VM & Evaluator 리팩토링 (Phase 9 완료)
- **VM::executeInstruction() 대폭 축소**
  - 790줄 → 50줄 (93% 감소)
  - 9개 실행 메서드로 분리 (OpCode 카테고리별)
    - executeConstantOps() - 상수 로드
    - executeVariableOps() - 변수 조작
    - executeArithmeticOps() - 산술 연산
    - executeComparisonOps() - 비교 연산
    - executeLogicalOps() - 논리 연산
    - executeControlFlowOps() - 제어 흐름 (JIT 통합)
    - executeArrayOps() - 배열 연산
    - executePromiseOps() - 비동기/Promise
    - executeMiscOps() - 기타 (클래스 시스템)

- **Evaluator::evalCallExpression() 대폭 축소**
  - 345줄 → 68줄 (80% 감소)
  - 4개 헬퍼 메서드로 분리
    - evalPromiseMethodCall() - Promise 메서드 호출
    - evalClassMethodCall() - 클래스 메서드 호출
    - evalConstructorCall() - 생성자 호출
    - evalRegularFunctionCall() - 일반 함수 호출

#### Clean Code 적용 (Phase 8 완료)
- 긴 함수 분석 및 분해 완료
- 코드 스타일 통일 (clang-format)
- 중복 코드 제거
- 매직 넘버 상수화

#### 문서화 강화
- Doxygen 설정 완료 (Doxyfile 추가)
- 사용자 가이드 작성 완료 (USER_GUIDE.md)
- README & ROADMAP 업데이트
- 예제 코드 정리

#### 성능 벤치마크
- 벤치마크 스위트 구축
- Python 비교 분석 완료
- 메모리 프로파일링 (3.7-4 MB)

### 🐛 버그 수정

- CTest WORKING_DIRECTORY 수정
- 다양한 버그 수정 및 안정성 개선

### 🧪 테스트

- **1,490개 테스트 모두 통과** (100% 성공률)
- regression_test.py 구현
- 테스트 커버리지 유지 및 향상

### ⚡ 성능

- JIT 컴파일러 통합 유지
- 코드 가독성 향상으로 인한 유지보수성 개선
- 메모리 풀링 연구 (향후 적용 예정)

### 📚 문서

- [아키텍처 분석](docs/ARCHITECTURE_ANALYSIS.md)
- [VM 분석](docs/KINGSEJONG_VM_ANALYSIS.md)
- [언어 분석](docs/KINGSEJONG_LANGUAGE_ANALYSIS.md)
- [사용자 가이드](docs/USER_GUIDE.md)
- [설정 가이드](docs/CONFIG_GUIDE.md)

### 🔄 변경 사항

- 없음 (하위 호환성 유지)

### ⚠️ 알려진 이슈

- 메모리 풀링: 시스템 allocator가 현재 2.4배 더 빠름 (향후 개선 예정)
- 증분 GC: 향후 버전에서 도입 예정

### 🎯 다음 단계

- Phase 6 완료: stdlib 확장 및 성능 최적화
- v1.0.0 준비: 최종 안정화 및 릴리스

---

## [v0.4.0] - 2025-11-20

### Phase 6 & 7 & 8 완료

- JIT Tier 1 컴파일러 구현
- 클래스 시스템 완성
- Async/Await 지원
- 세대별 GC 구현
- 코드 품질 및 문서화 향상

---

## [Unreleased]

### 계획 중인 기능

- 추가 Linter 규칙 (네이밍 컨벤션 등)
- Formatter VS Code 확장 기능
- stdlib/gui.ksj - GUI 라이브러리
- stdlib/network.ksj - 네트워크 라이브러리
- stdlib/ml.ksj - 머신러닝 바인딩
- LLVM 백엔드 (선택)
- 멀티스레딩 지원

---

**전체 변경 사항**: [GitHub Releases](https://github.com/kingsejong-lang/kingsejonglang/releases)
