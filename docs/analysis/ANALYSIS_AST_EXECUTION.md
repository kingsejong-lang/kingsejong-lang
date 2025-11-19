# AST & 실행 엔진 분석 (Phase 8 스냅샷)

**기준 버전**: v0.8.0-dev  
**최종 업데이트**: 2025-11-20  
**대상**: `src/ast`, `src/evaluator`, `src/bytecode`, `src/jit`

## 1. AST 현황
- 모든 Phase 7 기능(클래스, 패턴 매칭, Async/Await)이 AST 계층에 반영됨.
- 조사 표현식(JosaExpression)과 범위 표현식(RangeExpression)은 Evaluator·Compiler 양쪽에서 일관된 코드 경로를 사용.
- 긴 함수 목록: `parser::parseClassStatement`, `Evaluator::evalCallExpression`. → Phase 9 리팩토링 대상.

## 2. Evaluator (Tree-Walking)
- 클래스/메서드/this 처리 완료, 접근 제어는 파서 레벨에서만 enforced (향후 Semantic 분석 단계에서 보강 예정).
- 타입 없는 변수 선언은 `env_->set()`을 통해 동적으로 생성되므로, 문서와 코드 시그널을 맞추기 위해 LANGUAGE_REFERENCE/TUTORIAL 갱신 필요.
- 조사항목과 범위문 모두 100% 테스트 커버리지 확보 (EvaluatorTest/ParserTest).

## 3. Bytecode VM + JIT Tier 1
- Stack VM + Tier 1 템플릿 JIT 공존. HotPathDetector 임계값: 루프 100회.
- Tier 1은 산술/비교/논리/제어/스택 24개 OpCode를 지원하며, 실제 값 로딩도 완료.
- regression_test.py가 성능 회귀 감시를 담당해야 함 (Phase 8.4 남은 작업).

## 4. 메모리 관리
- 세대별 GC(Young/Old, 승격 정책) 도입 완료.
- Incremental GC/메모리 풀링 미구현 → Phase 6 carry-over.

## 5. 우선순위 액션
1. VM::executeInstruction() 분리 (핫 경로 가독성 개선)
2. Evaluator::evalCallExpression()과 evalJosaExpression() 분할
3. regression_test.py를 CI에 연결하여 JIT/VM 회귀 감시
4. Incremental GC 설계안 준비 (Phase 9 목표)
