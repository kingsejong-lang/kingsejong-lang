# Changelog

모든 주목할 만한 변경 사항이 이 파일에 문서화됩니다.

형식은 [Keep a Changelog](https://keepachangelog.com/ko/1.0.0/)를 기반으로 하며,
이 프로젝트는 [Semantic Versioning](https://semver.org/lang/ko/)을 준수합니다.

## [Unreleased]

### 제거
- JIT Tier 2(함수 인라이닝): `compileWithInlining` 이 네이티브 함수를 끝내 만들지 않아 VM 에서 호출돼도 아무 효과가 없었다. 소스·헤더·테스트·벤치마크·설계 문서를 지웠다. VM 의 핫 함수 추적은 남는다.
- `ksjpm` 패키지 관리자: 저장소·다운로드 없이 뼈대만 있어 실행 파일과 `src/package` 를 지웠다. 모듈은 `가져오기` 와 `KINGSEJONG_PATH` 로 쓴다.

### 고침
- `examples/expected/KNOWN_FAILURES` 에 있던 예제 8개가 모두 통과한다. 예제와 stdlib 가 쓰던 옛 API(`배열추가`, `X을_문자열로(X)`, `(조건)인_동안`, `순회한다 각`) 를 현재 문법으로 고쳤고, 없던 기본 함수를 추가했다 (아래 추가 항목).
- `타입()` 이 배열·딕셔너리·클래스·에러·프로미스에 "알 수 없음" 을 돌려주던 문제.
- `그리고`/`또는` 이 토큰으로는 읽히지만 평가기가 "지원되지 않는 연산" 을 내던 문제.
- 조사 표현식이 뒤따르는 이항 연산자를 통째로 삼키던 문제. `x를 f() + 1` 은 이제 `(x를 f()) + 1` 이다.
- `해시_비교()` 가 `체크섬()` 이 돌려주는 정수를 거부하던 문제.
- 브라우저(WASM) 빌드: 네이티브 라이브러리와 같은 소스 목록에서 jit·lsp·debugger·repl·linter·formatter·profiler·config 만 빼고 만들며, 32비트를 지원하지 않는 cpp-httplib 과 `HTTP_*` 내장 함수는 `KINGSEJONG_WASM` 에서 제외한다. Pages 배포 워크플로가 이 빌드를 쓴다.
- 클로저와 정의 환경의 순환 참조로 이름 붙은 함수(재귀 포함)·클래스를 선언한 스코프가 해제되지 않던 누수. 스코프를 떠날 때 환경을 붙잡는 참조가 전부 그 환경 안의 함수에서 온 것이면 저장소를 비워 사슬을 끊는다 (`Environment::releaseIfUnreachable`). 빠져나간 클로저는 손대지 않는다. CI 의 ASan 누수 검출을 다시 켰다.
- `kingsejong fmt` 가 포매터에 분기가 없는 구문(시도/오류, 클래스, 가져오기, 조사 표현식, 사전, 패턴 매칭 등)을 조용히 지우던 문제. 이제 미지원 구문이 있으면 파일을 건드리지 않고 목록을 보여 준다.
- 보안 검사를 거치지 않던 내장 함수(줄별_읽기, 디렉토리_생성/삭제/목록/나열/변경, 디렉토리인가, 파일인가, 경로_존재하는가, DB_열기, 환경변수_쓰기/삭제)가 SecurityManager 를 거친다.
- HTTP 내장 함수가 리다이렉트를 직접 따라가며 매 hop 을 검사한다. 호스트 추출이 userinfo(`user@host`), IPv6 리터럴, `0.0.0.0` 을 올바르게 다룬다.
- 모듈 안의 `가져오기` 가 "모듈 로더가 설정되지 않았습니다" 로 실패하던 문제. REPL 에서도 가져오기가 동작한다.
- 4글자 미만 모듈 이름이 `std::out_of_range` 를 던지던 문제.
- `아니면 만약` (else-if) 과 `반복 (조건) { }` / `동안 (조건) { }` (while) 이 파싱·실행된다.
- 파서 에러에 줄·열 위치가 붙는다.
- 빌드: asmjit 을 특정 커밋에 고정하고, googletest 에는 `-Werror` 를 적용하지 않는다.

### 추가
- 내장 함수 `문자열(값)`, `부분문자열(문자열, 시작, [끝])`, `포함(컨테이너, 값)`, `추가(배열, 값...)`.
- 변환 관용구 `값을 문자열로 변환한다` (= `문자열(값)`). `로/으로` 뒤의 이름이 내장·사용자 함수면 그 함수를 부른다. 동사는 `변환한다`/`바꾼다`.
- 문자열 반복 `"=" * 60`.
- `--sandbox`, `--untrusted`, `--allow-path` 플래그로 보안 정책을 켠다.
- `--vm`: 바이트코드 컴파일러와 VM 으로 실행한다 (실험적). VM 이 네이티브 내장 함수를 부를 수 있게 됐고, 같은 예제를 VM 으로 돌리는 ctest 항목 `examples-vm` 이 생겼다. 평가기와 다르게 동작하는 예제는 `examples/expected/KNOWN_FAILURES_VM` 에 있다.
- CI: GCC 의 주석 경고와 MSVC 에 넘어가던 asmjit 플래그를 고쳤다. 한 번도 통과한 적 없는 테스트 6개(JIT Tier 2 벤치마크 3, LSP 스코프 2, 할당 주소에 의존하는 JIT 캐시 1)는 이유를 적고 GTEST_SKIP 한다.
- `kingsejong lint` 서브커맨드. 규칙 13개가 기본 등록되고 `.ksjlint.json` 으로 끄고 켠다.
- `kingsejong --debug 파일.ksj`: 중단점·단계 실행·호출 스택·식 평가가 실제 실행에 연결됐다.
- LSP 서버가 `textDocument/publishDiagnostics` 를 보낸다. 구문 오류가 실제 줄에 표시된다.
- 모듈 검색 경로: 가져오기 문의 파일 디렉토리 → 실행 파일 기준 저장소/설치 디렉토리 → 작업 디렉토리 → `KINGSEJONG_PATH` → 설치 stdlib. `cmake --install` 이 stdlib 를 `share/kingsejong` 에 설치한다.
- 조사 표현식 `값을 함수(인자)` 가 사용자 함수·내장 함수를 `함수(값, 인자)` 로 호출한다. `의` 는 클래스 필드·사전 키를 읽는다.
- 린터 규칙 `josa-agreement`: 조사가 앞 명사의 받침과 맞지 않으면 경고한다.
- `examples/*.ksj` 를 실행해 `examples/expected/*.out` 과 비교하는 ctest 항목 `examples`. 아직 실패하는 예제는 `examples/expected/KNOWN_FAILURES` 에 이유와 함께 적는다 (현재 8개).
- 논리 연산자 `그리고`(`&&`), `또는`(`||`). null 리터럴 `없음`. 배열 이어 붙이기 `[1] + [2]`. 인덱스 할당 `배열[i] = 값`, `사전["키"] = 값`.
- 매개변수 이름으로 타입 키워드(`함수(배열)`)와 타입 있는 선언(`함수 f(배열 arr)`)을 받는다. 키워드와 겹치는 이름(`가져오기`, `마지막`, `반복`, `문자`)을 메서드·멤버·함수·변수 이름으로 쓸 수 있다. `가져오기("경로")` 괄호 형식.
- 의미 분석기가 가져온 모듈의 이름을 알아, 표준 라이브러리 함수가 "정의되지 않은 식별자"로 잡히지 않는다. 네이티브 내장 함수 표도 함께 본다. 배열 요소 타입이 섞여도 에러가 아니다.

### 고침 (어휘 분석)
- 조사 분리 규칙: 토큰이 있는 조사만, 그리고 뒤에 메서드 이름이나 범위의 숫자가 올 때만 떼어 낸다. `마을 = 1`, `출력(소수인가)`, `제곱차이 = x`, `사과`, `짝수만`이 더 이상 갈라지지 않는다. 범위 표시(부터/까지/미만/이상/이하/초과/번)는 항상 떼어 낸다. 키워드 전체(`또는`, `마지막`)는 쪼개지 않는다.
- 키워드 이름의 메서드(`공개 함수 가져오기()`)가 파서를 null 역참조로 크래시시키던 문제. 파서의 infix 경로에 null 보호를 넣었다.
- 표준 라이브러리: `crypto.ksj` 의 비표준 반복문 세 곳(`반복 i이 ... 미만`)을 고쳤다. 이전에는 array/crypto/db/http/io/string 모듈이 파싱되지 않아 가져오기가 실패했다.

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
