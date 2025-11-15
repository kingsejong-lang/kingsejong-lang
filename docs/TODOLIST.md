# KingSejong 언어 개발 작업 목록

> **프로젝트**: KingSejong Programming Language (`.ksj`)
> **최종 업데이트**: 2025-11-15
> **현재 버전**: v0.3.2 (개발 중)
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

#### Phase 5: 개발자 경험 & 생태계 ✅ (필수 100%)
- ✅ 에러 메시지 개선 (한글화, 위치 표시, 힌트)
- ✅ LSP 구현 (정의 이동, 호버, 참조 찾기, 이름 변경)
- ✅ VS Code Extension (구문 강조, 자동 완성, 진단)
- ✅ 디버거 (브레이크포인트, 단계 실행, 변수 검사) - 179개 테스트
- ✅ 온라인 플레이그라운드 (WebAssembly, Monaco Editor)
- ✅ 표준 라이브러리 확장 (io, json, time) - 101개+ 함수

---

## 🎯 현재 작업 (v0.3.2)

### 진행 중
1. **Parser 개선** 🚧 - 괄호 표현식 처리 버그 수정
2. **F5.5 패턴 매칭** 📝 (선택) - MatchExpression, 다양한 패턴 지원

### 알려진 이슈
- **Parser 버그**: `(a + b) * c` 같은 괄호 표현식을 함수 호출로 잘못 인식
  - 위치: Parser.cpp (parseGroupedExpression vs parseCallExpression)
  - 영향: 수식 그룹화 불가
  - 우선순위: P2 (중간)

---

## 📝 다음 단계 (Phase 5 선택 작업)

### F5.5: 패턴 매칭 (3주, MEDIUM)

**목표**: 직관적인 패턴 매칭 구문 구현

#### 작업 항목
- [ ] AST 노드 (MatchExpression, Pattern 계층)
- [ ] Lexer 토큰 (E_DAEHAE, ARROW, UNDERSCORE, WHEN)
- [ ] Parser 구현
  - [ ] parseMatchExpression
  - [ ] parseMatchCase
  - [ ] parsePattern (리터럴, 와일드카드, 바인딩, 배열)
- [ ] Evaluator 구현
  - [ ] evalMatchExpression
  - [ ] 패턴 매칭 로직 (타입 검사, 값 비교, 바인딩)
  - [ ] 가드 조건 평가
- [ ] 테스트 작성 (20개+)

**문법 예시**:
```ksj
결과 = 값 에 대해 {
    1 -> "하나"
    2 -> "둘"
    [첫번째, 나머지...] -> 첫번째
    n 만약 n > 10 -> "큰 수"
    _ -> "기타"
}
```

### F5.6: 비동기 프로그래밍 (4주, LOW, 선택)

**목표**: 시제 기반 비동기 프로그래밍

- [ ] Promise Value 타입
- [ ] async/await 구문
- [ ] 이벤트 루프
- [ ] 시제 분석 (현재/미래/과거)

---

## 📊 테스트 현황

```
총 테스트: 1003개
통과율: 100%
비활성화: 1개 (괄호 표현식 버그)

주요 테스트:
- Lexer/Parser: 59개
- Evaluator: 30개
- Type System: 48개
- Array: 62개
- Float: 48개
- String: 45개
- Function: 13개
- Loop: 21개
- GC: 15개
- Bytecode: 19개
- LSP: 130개
- 디버거: 179개
- 기타: 334개
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
- **테스트**: 1003개 (100% 통과)
- **문서**: 4,200+ 줄
- **예제**: 21개

### 성능
- **테스트 실행**: ~200ms
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
**현재 상태**: Phase 5 필수 완료, 선택 작업 진행 중
**다음 우선순위**: Parser 개선 (괄호 표현식 버그 수정)
