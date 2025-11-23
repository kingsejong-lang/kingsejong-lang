# KingSejong 언어 개발 작업 목록

> **프로젝트**: KingSejong Programming Language (`.ksj`)
> **최종 업데이트**: 2025-11-24
> **현재 버전**: v0.5.0+
> **상태 기호**: ✅ 완료 | 🚧 진행중 | 📝 예정 | ⏸️ 보류

---

## ✅ 최근 완료된 작업

### Phase 10: Tail Call Optimization - 100% 완료!
- [x] TAIL_CALL opcode 추가 (OpCode.h/cpp)
- [x] 컴파일러에서 tail call 자동 감지 (Compiler.cpp)
- [x] VM에서 CallFrame 재사용 구현 (VM.cpp)
- [x] 테스트 및 검증 (1,519개 테스트 100% 통과)
- [x] 성능 벤치마크 측정
  - Tail call fibonacci(25): 0.013초 (기존 4.241초 대비 326배 개선)
  - 꼬리 재귀 합계(n=1000): 정상 작동
- [x] 문서 업데이트 (PERFORMANCE_ANALYSIS.md, ROADMAP.md)
- **성과**: 꼬리 재귀 패턴에서 326배 성능 향상, 스택 오버플로우 방지

### Phase 8: 코드 품질 및 문서화 - 100% 완료!
- [x] Doxygen 설정 완료 (Doxyfile 추가)
- [x] 사용자 가이드 작성 완료 (USER_GUIDE.md)
- [x] 성능 회귀 테스트 자동화 (regression_test.py)
- [x] 전체 1,506개 테스트 100% 통과
- [x] CTest WORKING_DIRECTORY 수정
- [x] 긴 함수 분석 완료 → Phase 9 리팩토링으로 이관
- [x] Analysis 문서 정리 (구식 문서 삭제)
- [x] README.md 버전 정정 (v0.5.0)
- [x] stdlib 함수 문서화 완료 (STDLIB_REFERENCE.md - 251개 함수)
- [x] Linter 규칙 확장 완료 (LINTER_RULES.md - 13개 규칙)
- [x] README.md 대폭 업데이트 (새 문서 반영)

### Phase 6.1: 코드 품질 도구 - 완료!
- [x] Linter 설정 파일 (`.ksjlintrc`) - 8개 규칙 정의
- [x] Formatter 설정 파일 (`.ksjfmtrc`) - 6개 옵션 정의
- [x] CONFIG_GUIDE.md - 설정 가이드 문서 작성
- [x] VS Code 통합 가이드 포함
- [x] CI/CD 통합 예제 포함

### Phase 9: 코드 리팩토링 - 100% 완료!

#### Step 1: VM 리팩토링 - 완료!
- [x] VM::executeInstruction() 리팩토링 (790줄 → 50줄, 93% 감소)
- [x] 9개 카테고리 메서드 구현
  - executeConstantOps() (4 OpCodes)
  - executeVariableOps() (4 OpCodes)
  - executeArithmeticOps() (6 OpCodes)
  - executeComparisonOps() (6 OpCodes)
  - executeLogicalOps() (3 OpCodes)
  - executeControlFlowOps() (8 OpCodes, JIT 통합)
  - executeArrayOps() (5 OpCodes)
  - executePromiseOps() (8 OpCodes)
  - executeMiscOps() (10 OpCodes, 클래스 시스템)
- [x] 전체 1,490개 테스트 100% 통과
- [x] JIT 컴파일러 통합 유지
- [x] 클래스 시스템 지원 유지

#### Step 2: Evaluator 리팩토링 - 완료!
- [x] Evaluator::evalCallExpression() 분석 (345줄)
- [x] 4개 헬퍼 메서드 구현
  - evalPromiseMethodCall() (Promise 메서드 호출, ~108줄)
  - evalClassMethodCall() (클래스 메서드 호출, ~69줄)
  - evalConstructorCall() (생성자 호출, ~56줄)
  - evalRegularFunctionCall() (일반 함수 호출, ~81줄)
- [x] evalCallExpression() 크기 80% 감소 (345줄 → 68줄)
- [x] 전체 1,490개 테스트 100% 통과 유지
- [x] 리팩토링 계획 문서 작성 (EVALUATOR_REFACTORING_PLAN.md)

## 🚧 현재 집중 작업

### 벤치마크 개선
- [ ] array_ops.ksj 문법 업데이트
- [ ] memory_test.ksj "반복" 키워드 수정
- [ ] 벤치마크 재실행 및 검증

### v0.6.0 릴리스 준비
- [x] Tail Call Optimization 구현 ✅
- [ ] 함수 인라이닝 (JIT Tier 2) 검토
- [ ] 성능 회귀 테스트 확장
- [ ] 릴리스 노트 작성

## 🛠 Phase 6 남은 과제

### 6.1: 코드 품질 도구 - 95% 완료!
- [x] Linter 설정 파일 (`.ksjlintrc`) ✅
- [x] 추가 Linter 규칙 (네이밍 컨벤션 등) ✅ - 13개 규칙 완료
- [x] LINTER_RULES.md 문서 작성 ✅
- [ ] Formatter VS Code 확장 기능 개발
- [x] Formatter 설정 파일 (`.ksjfmtrc`) ✅

### 6.2: 표준 라이브러리 확장 - 100% 완료!
- [x] stdlib/http.ksj - HTTP 클라이언트 (완료)
- [x] stdlib/db.ksj - 데이터베이스 유틸리티 (완료)
- [x] stdlib/collections.ksj - 컬렉션 라이브러리 (완료)
- [x] stdlib 함수 문서화 ✅ - STDLIB_REFERENCE.md 작성
- [x] **목표 달성**: 표준 라이브러리 251개 함수 완전 문서화 (14개 모듈)

### ✅ 6.3: 메모리 풀링 - 완료!
- [x] ObjectPool 템플릿 구현 (src/memory/ObjectPool.h)
- [x] ValuePool 싱글톤 래퍼 (src/memory/ValuePool.h)
- [x] 16개 테스트 작성 및 통과
- [x] 벤치마크 및 분석 완료
- **결과**: 시스템 allocator가 더 빠름 (GC 통합 보류)
- **향후**: 증분 GC 고려

## 📚 문서화 TODO
- [x] ✅ 디버거 사용 가이드 (DEBUGGER_GUIDE.md 완성)
- [x] ✅ 표준 라이브러리 레퍼런스 (STDLIB_REFERENCE.md - 251개 함수)
- [x] ✅ Linter 규칙 가이드 (LINTER_RULES.md - 13개 규칙)
- [x] ✅ 플레이그라운드 튜토리얼 (PLAYGROUND_TUTORIAL.md - Monaco, WASM, 배포)
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

## 🚀 향후 계획

### v0.9.0 (2025-11 말)
- Phase 8 완료 (Clean Code, Documentation)
- 테스트 커버리지 95%+
- 성능 회귀 테스트 자동화

### v1.0.0 (2025-12)
- 언어 사양서 (Specification)
- 안정화 및 버그 수정
- 공식 릴리스

### v1.1.0 (2026 Q1)
- 다중 구현체 지원
- 국제화 (i18n)
- 추가 stdlib 확장

---

**마지막 업데이트**: 2025-11-24
**현재 상태**: Phase 6, 8, 9, 10 완료! TCO 최적화 성공
**다음 우선순위**: 벤치마크 개선, 함수 인라이닝, v0.6.0 릴리스 준비
**최근 성과**:
- ✅ **Tail Call Optimization 구현** - 꼬리 재귀 326배 성능 개선
- ✅ VM::executeInstruction() 93% 감소 (790줄 → 50줄)
- ✅ Evaluator::evalCallExpression() 80% 감소 (345줄 → 68줄)
- ✅ stdlib 251개 함수 완전 문서화 (STDLIB_REFERENCE.md)
- ✅ Linter 13개 규칙 문서화 (LINTER_RULES.md)
- ✅ 디버거 사용 가이드 작성 (DEBUGGER_GUIDE.md)
- ✅ 메모리 풀링 분석 완료 (MEMORY_POOLING_DESIGN.md)
- ✅ 전체 1,519개 테스트 100% 통과 유지
