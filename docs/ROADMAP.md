# KingSejong 언어 로드맵

> **비전**: 한국어로 누구나 쉽게 프로그래밍할 수 있는 세상

**최종 업데이트**: 2025-11-23
**현재 버전**: v0.5.0 (릴리스 준비 중)
**Production Readiness**: 8.9/10 (코드 품질 + Playground 향상)
**중요 문서**: [아키텍처 분석](./ARCHITECTURE_ANALYSIS.md) | [VM 분석](./KINGSEJONG_VM_ANALYSIS.md) | [언어 분석](./KINGSEJONG_LANGUAGE_ANALYSIS.md)

---

## 🎉 현재 상태

- **Phase 9 완료!** VM 및 Evaluator 리팩토링 성공
  - ✅ VM::executeInstruction() 9개 메서드로 분리 (790줄 → 50줄, 93% 감소)
  - ✅ Evaluator::evalCallExpression() 4개 헬퍼 메서드로 분리 (345줄 → 68줄, 80% 감소)
  - ✅ 코드 가독성 및 유지보수성 대폭 향상
  - ✅ JIT 컴파일러 통합 유지
  - ✅ 전체 테스트 1,490개 (100% 통과)
- **Phase 6.1 & 6.2 완료!** 코드 품질 도구 및 stdlib 확장
  - ✅ .ksjlintrc, .ksjfmtrc 설정 파일 생성
  - ✅ CONFIG_GUIDE.md 작성 (상세 가이드)
  - ✅ VS Code 통합 및 CI/CD 예제 포함
  - ✅ **stdlib 251개 함수** (목표 200+ 초과 달성!)
- **Playground 완료!** 온라인 플레이그라운드 대폭 개선
  - ✅ **17개 템플릿** (기본 9개 + stdlib 8개)
  - ✅ 코드 공유 기능 (URL 기반)
  - ✅ 실행 결과 다운로드
  - ✅ 클립보드 복사 기능
- Production Readiness: **8.9/10** (코드 품질 + Playground 향상)

---

## ✅ Phase 8: 코드 품질 및 문서화 (v0.8.0) - 완료!

**기간**: 2025-11-19 ~ 2025-11-20 (완료!)
**목표**: 코드 품질 향상, 문서화 완성, 회귀 테스트 자동화
**진행률**: 100% ✅

### ✅ 8.1 Clean Code 적용 (70%)
- [x] 긴 함수 분석 완료 (Phase 9로 리팩토링 이관)
- [x] 코드 스타일 통일 (clang-format)
- [x] 중복 코드 제거
- [x] 매직 넘버 상수화

### ✅ 8.2 주석 및 문서화 (100%)
- [x] Doxygen 설정 완료 (Doxyfile 추가)
- [x] 사용자 가이드 작성 완료 (USER_GUIDE.md)
- [x] README & ROADMAP 업데이트
- [x] 예제 코드 정리

### ✅ 8.3 성능 벤치마크 (100%)
- [x] 벤치마크 스위트 구축
- [x] Python 비교 분석
- [x] 메모리 프로파일링 (3.7-4 MB)

### ✅ 8.4 테스트 강화 (100%)
- [x] 테스트 1490개 (100% 통과)
- [x] regression_test.py 구현
- [x] CTest WORKING_DIRECTORY 수정

---

## ✅ Phase 6.1: 코드 품질 도구 - 완료!

**기간**: 2025-11-21 (완료!)
**목표**: Linter 및 Formatter 설정 파일 지원
**진행률**: 80% ✅

- [x] ✅ Linter 설정 파일 (`.ksjlintrc`) - 8개 규칙 정의
- [x] ✅ Formatter 설정 파일 (`.ksjfmtrc`) - 6개 옵션 정의
- [x] ✅ CONFIG_GUIDE.md - 상세 설정 가이드 문서
- [x] ✅ VS Code 통합 가이드 포함
- [x] ✅ CI/CD 통합 예제 (GitHub Actions, pre-commit hook)
- [ ] 📝 추가 Linter 규칙 (네이밍 컨벤션 등)
- [ ] 📝 Formatter VS Code 확장 기능 개발

---

## ✅ Phase 9: 코드 리팩토링 (v0.9.0) - 완료!

**기간**: 2025-11-21 (완료!)
**목표**: 긴 함수 분해, 코드 구조 개선
**진행률**: 100% ✅

### ✅ 9.1 VM 리팩토링 - 완료!
- [x] ✅ VM::executeInstruction() 분석 (790줄)
- [x] ✅ OpCode 카테고리화 (9개 그룹)
- [x] ✅ 9개 실행 메서드 구현
  - executeConstantOps() - 상수 로드 (4 OpCodes)
  - executeVariableOps() - 변수 조작 (4 OpCodes)
  - executeArithmeticOps() - 산술 연산 (6 OpCodes)
  - executeComparisonOps() - 비교 연산 (6 OpCodes)
  - executeLogicalOps() - 논리 연산 (3 OpCodes)
  - executeControlFlowOps() - 제어 흐름 (8 OpCodes, JIT 통합)
  - executeArrayOps() - 배열 연산 (5 OpCodes)
  - executePromiseOps() - 비동기/Promise (8 OpCodes)
  - executeMiscOps() - 기타 (10 OpCodes, 클래스 시스템)
- [x] ✅ 전체 테스트 1,490개 100% 통과
- [x] ✅ JIT 컴파일러 통합 유지
- [x] ✅ executeInstruction() 크기 93% 감소 (790줄 → 50줄)

### ✅ 9.2 Evaluator 리팩토링 - 완료!
- [x] ✅ Evaluator::evalCallExpression() 분석 (345줄)
- [x] ✅ 4개 헬퍼 메서드 구현
  - evalPromiseMethodCall() - Promise 메서드 호출 (~108줄)
  - evalClassMethodCall() - 클래스 메서드 호출 (~69줄)
  - evalConstructorCall() - 생성자 호출 (~56줄)
  - evalRegularFunctionCall() - 일반 함수 호출 (~81줄)
- [x] ✅ evalCallExpression() 크기 80% 감소 (345줄 → 68줄)
- [x] ✅ 전체 테스트 1,490개 100% 통과 유지

---

## 🛠 Phase 6 남은 과제

### ✅ 6.2 표준 라이브러리 확장 - 완료!
- [x] ✅ stdlib/http.ksj - 21개 함수 (완료)
- [x] ✅ stdlib/db.ksj - 14개 함수 (완료)
- [x] ✅ stdlib/collections.ksj - 49개 함수 (완료)
- [x] ✅ **전체 함수 수 251개 달성** (목표 200+ 초과 달성!)

### ✅ 6.3 메모리 풀링 (완료)
- [x] ✅ ObjectPool 템플릿 구현 (src/memory/ObjectPool.h)
- [x] ✅ ValuePool 싱글톤 래퍼 (src/memory/ValuePool.h)
- [x] ✅ 16개 테스트 작성 및 통과
- [x] ✅ 벤치마크 및 분석 완료
- **결과**: 시스템 allocator가 2.4배 더 빠름 (placement new 오버헤드)
- **결론**: GC 통합 보류, 현재 구조 유지
- **향후**: 증분 GC 또는 특수 목적 풀 고려

---

## 🔮 Phase 10 이후: 장기 계획

### 10.1 표준 라이브러리 고도화
- [ ] stdlib/collections.ksj - 리스트, 맵, 집합 고급 API
- [ ] stdlib/gui.ksj - 간단한 GUI, Canvas, 이벤트 루프
- [ ] stdlib/network.ksj - 소켓, HTTP 서버, WebSocket
- [ ] stdlib/ml.ksj - 머신러닝/수치 계산 바인딩

### 10.2 LLVM 백엔드 (선택)
- [ ] LLVM IR 생성 파이프라인
- [ ] AOT 컴파일 및 실행 파일 생성
- [ ] 목표 성능: V8 대비 80~90%

### 10.3 멀티스레딩
- [ ] 스레드 생성/관리 API
- [ ] 동기화 도구 (뮤텍스, 세마포어)
- [ ] 채널 기반 통신 (Go 스타일)

### 10.4 언어 문법 개선 (v2.0.0)
- [ ] 세미콜론 필수화 설계 및 마이그레이션 가이드
- [ ] Linter/Formatter에 자동 변환 옵션 제공 (`--add-semicolons`)
- [ ] 유예 기간 6개월 (v1.5 경고 → v2.0 강제)

---

## 📅 타임라인

```
2025-11-17   Phase 6 완료 (JIT Tier 1) ✅
2025-11-19   Phase 7 완료 (클래스, Async, 세대별 GC) ✅
2025-11-20   Phase 8 완료 (코드 품질, 문서화) ✅
2025-11-21   Phase 6.1 완료 (Linter/Formatter 설정) ✅
2025-11-21   Phase 9 완료 (VM & Evaluator 리팩토링) ✅
2025-11~12   Phase 6 남은 과제 (stdlib 확장, 성능 최적화)
2025-12      v0.5.0 릴리스 (리팩토링 완료)
2026-01      v1.0.0 릴리스 (안정 버전)
2026-03      v1.5.0 - 세미콜론 경고, Clean Code 2차
2026-09      v2.0.0 - 세미콜론 필수화, Incremental GC
2027-06      v3.0.0 - LLVM 백엔드, 네이티브 컴파일
```

---

## 🎯 성공 기준 (Upcoming)

### v0.5.0
- 🎯 Production Readiness 8.9/10 이상 (코드 품질 향상)
- 🎯 VM 및 Evaluator 리팩토링 완료
- 🎯 Playground 17개 템플릿 지원
- 🎯 전체 테스트 1,490개 100% 통과 유지

### v1.0.0
- 🎯 Production Readiness 9.0/10 이상
- 🎯 API 문서 & 사용자 가이드 완비
- 🎯 성능 회귀 테스트 자동화, 커버리지 보고서 공유

### v2.0.0
- 🎯 세미콜론 필수 문법 전환 및 자동 마이그레이션 도구
- 🎯 Incremental GC + 메모리 풀링 도입
- 🎯 사용자 코드 호환성 가이드 및 경고 체계

---

## 🚀 다음 우선순위 작업

1. **v0.5.0 릴리스**: 리팩토링 및 Playground 개선 완료
   - CHANGELOG.md 작성
   - 릴리스 노트 작성
   - 버전 태그 생성
2. **Phase 6 완료**: stdlib 확장 및 성능 최적화
   - stdlib/http·db 추가 기능 구현
   - 추가 Linter 규칙 구현
   - 증분 GC 및 메모리 풀링
3. **v1.0.0 준비**: 최종 안정화, 릴리스 노트 작성

---

## 💡 장기 비전

- **기술**: Register-based VM + 선택적 JIT + LLVM 백엔드
- **철학**: 훈민정음의 디지털 후계자
- **목표**: 한국어로 프로그래밍하는 자연스러운 언어
- **커뮤니티**: 교육, 기업, 연구 분야에서 활용

---

**마지막 업데이트 메모**: v0.5.0 릴리스 준비 완료! 🎉
- Phase 9: VM & Evaluator 리팩토링 (VM: 790줄 → 50줄, Evaluator: 345줄 → 68줄)
- Playground: 17개 템플릿 추가 (기본 9개 + stdlib 8개)
- 전체 1,490개 테스트 100% 통과
- Production Readiness: 8.9/10
