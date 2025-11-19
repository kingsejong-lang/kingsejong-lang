# KingSejong 언어 로드맵

> **비전**: 한국어로 누구나 쉽게 프로그래밍할 수 있는 세상

**최종 업데이트**: 2025-11-20
**현재 버전**: v0.8.0-dev (Phase 8 진행 중)
**Production Readiness**: Phase 8 마무리 후 재측정 예정
**중요 문서**: [아키텍처 분석](./ARCHITECTURE_ANALYSIS.md) | [VM 분석](./KINGSEJONG_VM_ANALYSIS.md) | [언어 분석](./KINGSEJONG_LANGUAGE_ANALYSIS.md)

---

## 🚧 현재 상태

- Phase 8은 코드 품질·문서화·테스트 강화에 집중하고 있으며, 완결을 위해 API 문서와 사용자 가이드가 필요합니다.
- 긴 함수 리팩토링(VM::executeInstruction, Evaluator::evalCallExpression, Evaluator::evalJosaExpression)은 분석만 끝난 상태라 Phase 9 리팩토링 티켓으로 이관해야 합니다.
- stdlib/http, stdlib/db, Linter/Formatter 설정 파일 등 Phase 6 잔여 과제가 남아 있습니다.
- TEST_COVERAGE / SESSION_SUMMARY 등 요약 문서는 삭제했으므로 Phase 8 완료 시점에 다시 측정해 공유합니다.

---

## 🎯 Phase 8: 코드 품질 및 문서화 (v1.0.0)

**기간**: 2025-11 ~ 2025-12
**목표**: 코드 품질 향상, 문서화 완성, 회귀 테스트 자동화
**진행률**: 83% (API 문서 · 사용자 가이드 · 회귀 테스트 남음)

### 8.1 Clean Code 적용 (P1)
- [ ] VM::executeInstruction()를 역할별 헬퍼로 분리 (현재 792줄)
- [ ] Evaluator::evalCallExpression() 분해 (351줄 → 호출 준비/실행/정리 단계화)
- [ ] Evaluator::evalJosaExpression() 로직 정리 (조사/패턴 처리 분리)

### 8.2 주석 및 문서화 (P1)
- [ ] Doxygen 기반 **API 문서 생성** (HTML 출력 검증, 배포 경로 결정)
- [ ] 사용자 가이드/튜토리얼 확장 (패턴 매칭, 디버거 흐름, 타입 선언 가이드)
- [ ] 예제 코드 재검증: 타입 명시·`타입()` 함수 사용법·주석 처리 일치 여부 점검

### 8.4 테스트 강화 (P2)
- [ ] regression_test.py 임계값 조정 및 CI 통합 (성능 회귀 자동 감지)
- [ ] 커버리지 리포트 재측정 후 공유 (TEST_COVERAGE.md 재작성)
- [ ] Summary 문서(세션/테스트) 재생성: 최종 코드 정리 후 스냅샷 보관

---

## 🛠 Phase 6 남은 과제

### 6.1 코드 품질 도구
- [ ] Linter 설정 파일 (`.ksjlintrc`) 지원
- [ ] 추가 규칙 (네이밍 컨벤션, 불필요한 세미콜론 등)
- [ ] Formatter VS Code 통합
- [ ] Formatter 설정 파일 (`.ksjfmtrc`)

### 6.2 표준 라이브러리 확장
- [ ] stdlib/http.ksj - HTTP 클라이언트/서버 함수 20개
- [ ] stdlib/db.ksj - 데이터베이스 유틸리티 15개
- [ ] 전체 함수 수 200+ 달성 (현재 173개 수준)

### 6.3 성능 최적화
- [ ] 증분 GC (Incremental GC)
- [ ] 메모리 풀링 전략 (Allocator/Pool 재사용)
- [ ] 목표: 2~5배 실행 성능 향상

---

## 🔮 Phase 9 이후: 장기 계획

### 9.1 표준 라이브러리 고도화
- [ ] stdlib/collections.ksj - 리스트, 맵, 집합 고급 API
- [ ] stdlib/gui.ksj - 간단한 GUI, Canvas, 이벤트 루프
- [ ] stdlib/network.ksj - 소켓, HTTP 서버, WebSocket
- [ ] stdlib/ml.ksj - 머신러닝/수치 계산 바인딩

### 9.2 LLVM 백엔드 (선택)
- [ ] LLVM IR 생성 파이프라인
- [ ] AOT 컴파일 및 실행 파일 생성
- [ ] 목표 성능: V8 대비 80~90%

### 9.3 멀티스레딩
- [ ] 스레드 생성/관리 API
- [ ] 동기화 도구 (뮤텍스, 세마포어)
- [ ] 채널 기반 통신 (Go 스타일)

### 9.4 언어 문법 개선 (v2.0.0)
- [ ] 세미콜론 필수화 설계 및 마이그레이션 가이드
- [ ] Linter/Formatter에 자동 변환 옵션 제공 (`--add-semicolons`)
- [ ] 유예 기간 6개월 (v1.5 경고 → v2.0 강제)

---

## 📅 타임라인 (향후)

```
2025-12   v1.0.0 릴리스 (Phase 8 완료)
2026-03   v1.5.0 - 세미콜론 경고, Clean Code 2차
2026-09   v2.0.0 - 세미콜론 필수화, Incremental GC
2027-06   v3.0.0 - LLVM 백엔드, 네이티브 컴파일
```

---

## 🎯 성공 기준 (Upcoming)

### v1.0.0
- 🎯 Production Readiness 8.5/10 이상 (Phase 8 완료 기준)
- 🎯 API 문서 & 사용자 가이드 완비
- 🎯 성능 회귀 테스트 자동화, 커버리지 보고서 공유

### v2.0.0
- 🎯 세미콜론 필수 문법 전환 및 자동 마이그레이션 도구
- 🎯 Incremental GC + 메모리 풀링 도입
- 🎯 사용자 코드 호환성 가이드 및 경고 체계

---

## 🚀 다음 우선순위 작업

1. **Phase 8 마무리**: API 문서, 사용자 가이드, 회귀 테스트/커버리지 재측정
2. **Phase 6 carry-over**: Linter/Formatter 설정 파일, stdlib/http·db 구현
3. **Phase 9 준비**: 긴 함수 리팩토링, Incremental GC 설계, 세미콜론 필수화 계획 수립

---

## 💡 장기 비전

- **기술**: Register-based VM + 선택적 JIT + LLVM 백엔드
- **철학**: 훈민정음의 디지털 후계자
- **목표**: 한국어로 프로그래밍하는 자연스러운 언어
- **커뮤니티**: 교육, 기업, 연구 분야에서 활용

---

**마지막 업데이트 메모**: Phase 8 작업 집중 (API 문서 · 사용자 가이드 · 성능 회귀 테스트 준비 중)
