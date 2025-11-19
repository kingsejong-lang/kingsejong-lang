# KingSejong 언어 개발 작업 목록

> **프로젝트**: KingSejong Programming Language (`.ksj`)
> **최종 업데이트**: 2025-11-20
> **현재 버전**: v0.8.0 (Phase 8 진행중)
> **상태 기호**: ✅ 완료 | 🚧 진행중 | 📝 예정 | ⏸️ 보류

---

## 🚧 현재 집중 작업

### Phase 8: 코드 품질 및 문서화 (v0.8.0)
- [ ] Doxygen에서 누락된 항목 보완 후 **API 문서 생성**
- [ ] 사용자 가이드 확장 (튜토리얼, 패턴 매칭, 디버거 흐름)
- [ ] 8.4 테스트 강화 - **성능 회귀 테스트** 시나리오 작성 및 자동화
- [ ] 8.1 긴 함수 분해 (VM::executeInstruction, Evaluator::evalCallExpression, Evaluator::evalJosaExpression) → Phase 9 리팩토링 티켓으로 승격

## 🛠 Phase 6 남은 과제

### 6.1: 코드 품질 도구
- [ ] Linter 설정 파일 (`.ksjlintrc`)
- [ ] 추가 Linter 규칙 (네이밍 컨벤션 등)
- [ ] Formatter VS Code 통합
- [ ] Formatter 설정 파일 (`.ksjfmtrc`)

### 6.2: 표준 라이브러리 확장
- [ ] stdlib/http.ksj - HTTP 클라이언트/서버 (20개 함수)
- [ ] stdlib/db.ksj - 데이터베이스 유틸리티 (15개 함수)
- [ ] **목표**: 표준 라이브러리 200개+ 함수 (현재 173개 수준)

### 6.3: 성능 최적화
- [ ] 증분 GC (Incremental GC)
- [ ] 메모리 풀링
- [ ] **목표**: 2-5배 성능 향상

## 📚 문서화 TODO
- [ ] 디버거 사용 가이드 (상태/변수 예제 필요)
- [ ] 플레이그라운드 튜토리얼 (Monaco, WebAssembly 흐름)
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

**마지막 업데이트**: 2025-11-20
**현재 상태**: Phase 8 진행 중 (83%)
**다음 우선순위**: Phase 8 완료 (API 문서 생성 → 성능 회귀 테스트)
