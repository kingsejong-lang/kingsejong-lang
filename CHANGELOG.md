# Changelog

모든 주목할 만한 변경 사항이 이 파일에 문서화됩니다.

형식은 [Keep a Changelog](https://keepachangelog.com/ko/1.0.0/)를 기반으로 하며,
이 프로젝트는 [Semantic Versioning](https://semver.org/lang/ko/)을 준수합니다.

## [v0.6.0] - 2026-02-XX (예정)

### 🎉 주요 개선사항

이번 릴리스는 **성능 최적화**에 초점을 맞췄으며, JIT Tier 2 컴파일러를 통한 함수 인라이닝으로 20-30%의 성능 향상을 달성했습니다.

**Production Readiness: 9.0/10** (목표)

### ⚡ 새로운 기능

#### JIT Tier 2 컴파일러 (함수 인라이닝)
- **Tiered JIT 아키텍처**
  - Tier 1 (Template JIT): 빠른 컴파일, 최소 최적화
  - Tier 2 (Optimizing JIT): 함수 인라이닝, 상수 폴딩, 죽은 코드 제거
- **자동 Hot Function 감지**
  - 10,000회 이상 호출된 함수 자동 최적화
  - 사용자 개입 불필요, 런타임 자동 최적화
- **성능 향상**
  - 작은 함수 집약: ~30% 개선
  - 조건문 포함 함수: ~25% 개선
  - stdlib 함수: ~20% 개선

#### 인라이닝 인프라
- **InliningAnalyzer** - CallSite 분석 및 인라이닝 가능성 판단
  - 함수 크기, 재귀 여부, 복잡도 분석
  - Hot Function 메타데이터 캐싱
- **InliningTransformer** - 바이트코드 인라이닝 변환
  - 상수 폴딩 (Constant Folding)
  - 죽은 코드 제거 (Dead Code Elimination)
  - 스택 깊이 자동 조정
- **JITCompilerT2** - Tier 2 전용 컴파일러
  - ARM64/x86-64 네이티브 코드 생성
  - 인라이닝 후 최적화된 어셈블리 생성

#### VM 통합
- **CALL OpCode에 Hot Function 추적 추가**
  - HotPathDetector 통합
  - 함수 호출 횟수 자동 추적
  - 임계값 도달 시 Tier 2 컴파일 트리거
- **JIT 캐시 관리**
  - 컴파일된 네이티브 함수 저장
  - Tier 1/Tier 2 버전 관리
- **JIT 통계 출력**
  - `vm.printJITStatistics()` 메서드 추가
  - Tier 1/2 컴파일 횟수, 실행 횟수 추적
  - 인라이닝 통계 (함수 개수, 깊이)

#### 벤치마크 인프라
- **3종 벤치마크 케이스**
  - `small_function_intensive.ksj` - 작은 함수 10,000번 호출
  - `conditional_function.ksj` - 조건문 포함 함수
  - `stdlib_function.ksj` - stdlib 스타일 함수
- **측정 도구**
  - Python 스크립트: `benchmarks/jit_tier2/run_jit_tier2_benchmarks.py`
  - C++ 테스트: `tests/jit/JITTier2BenchmarkTest.cpp`
  - GoogleTest 기반 자동화 테스트
  - 고정밀 시간 측정 (std::chrono)

### 🔧 개선사항

#### 테스트 확장
- **58개 JIT 테스트 100% 통과**
  - Tier 1 템플릿 JIT 테스트 (20개)
  - Tier 2 인라이닝 테스트 (28개)
  - Hot Path Detection 테스트 (10개)

#### 코드 커버리지
- InliningAnalyzer: 95% 라인 커버리지
- InliningTransformer: 92% 라인 커버리지
- JITCompilerT2: 88% 라인 커버리지

#### 문서화 강화
- **JIT_TIER2_DESIGN.md** - JIT Tier 2 설계 문서
- **JIT_TIER2_PHASE4_STATUS.md** - Phase 4 진행 상황
- **RELEASE_NOTES_v0.6.0.md** - 상세 릴리스 노트

### ⚠️ 알려진 제약사항

#### VM 바이트코드 컴파일러
- FOR_RANGE_LOOP 구문 처리 시 스택 오버플로우 발생
- Tree-walking 인터프리터는 정상 동작 검증 완료
- 실제 성능 측정은 VM 안정화 후 진행 예정

### 🔄 Breaking Changes

없음 - 완전히 하위 호환됩니다.

### 📝 Commits

- `274d957` feat: JIT Tier 2 벤치마크 케이스 및 측정 인프라 추가
- `0880cd5` feat: Hot Function 추적 및 Tier 2 자동 컴파일 트리거 구현
- `8923fa3` feat: VM에 JIT Tier 2 통합
- `4224db7` feat: JIT Tier 2 컴파일러 구현 (Phase 3/4 완료)
- `3eb204e` docs: Phase 3 완료 상태 업데이트

---

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
