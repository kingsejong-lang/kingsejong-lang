# KingSejong 언어 개발 작업 목록

> **프로젝트**: KingSejong Programming Language (`.ksj`)
> **최종 업데이트**: 2025-11-24
> **현재 버전**: v0.5.0+
> **상태 기호**: 🚧 진행중 | 📝 예정

---

## 🚧 현재 작업

### v0.6.0 릴리스 준비
**우선순위**: 최고 | **예상 기간**: 2-3주

- [ ] 함수 인라이닝 (JIT Tier 2) 검토
  - [ ] 인라이닝 후보 함수 분석
  - [ ] 인라이닝 구현
  - [ ] 성능 벤치마크 측정
- [ ] 성능 회귀 테스트 확장
  - [ ] 추가 벤치마크 케이스 작성
  - [ ] CI/CD 통합
- [ ] 릴리스 노트 작성
  - [ ] v0.6.0 릴리스 노트 초안
  - [ ] CHANGELOG.md 업데이트

### 벤치마크 개선
**우선순위**: 중간 | **예상 기간**: 1주

- [ ] array_ops.ksj 완전 수정
  - [ ] 동적 배열 연산 대체
  - [ ] 문법 검증
- [ ] memory_test.ksj 완전 수정
  - [ ] 중첩 배열 문법 수정
  - [ ] 실행 검증
- [ ] 벤치마크 재실행 및 검증
  - [ ] 모든 벤치마크 성공 확인
  - [ ] PERFORMANCE_ANALYSIS.md 업데이트

---

## 📝 향후 계획

### 코드 품질 도구
**우선순위**: 낮음 | **예상 기간**: 1-2주

- [ ] Formatter VS Code 확장 Marketplace 배포
  - [ ] 확장 패키징
  - [ ] VS Code Marketplace 등록
  - [ ] 배포 및 홍보

### 문서화
**우선순위**: 낮음

- [ ] 패턴 매칭 가이드 작성 (선택)

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

## 🚀 장기 계획

### v0.6.0 (2026년 2월 목표)
- 함수 인라이닝 구현
- 성능 회귀 테스트 자동화
- Production Readiness 9.0/10 달성

### v0.7.0 (2026년 9월 목표)
- stdlib 고도화 (gui, network)
- 성능 개선 (Python 대비 3-5배)

### v0.8.0 (2027년 3월 목표)
- Incremental GC 도입
- 메모리 풀링 최적화

### v0.9.0 (2027년 9월 목표)
- LLVM 백엔드 완성 (선택)
- 멀티스레딩 완전 지원

---

**마지막 업데이트**: 2025-11-24
**현재 상태**: v0.5.0 릴리스 완료, TCO 최적화 완료 (326배 성능 개선)
**다음 우선순위**: 함수 인라이닝 (JIT Tier 2), v0.6.0 릴리스 준비
