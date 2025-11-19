# 개선 제안 목록 (Phase 8 이후)

**기준 버전**: v0.8.0-dev  
**최종 업데이트**: 2025-11-20

## P0 (즉시)
1. Doxygen 기반 API 문서 생성 및 배포 (Phase 8.2)
2. regression_test.py CI 통합 (Phase 8.4)
3. 튜토리얼/사용자 가이드의 타입 선언, `타입()` 함수 설명 통일

## P1 (Phase 9)
1. VM::executeInstruction, Evaluator::evalCallExpression, Evaluator::evalJosaExpression 분해
2. Incremental GC 설계 및 구현
3. stdlib/http, stdlib/db 제공 → stdlib 200+ 함수 달성
4. Linter/Formatter 설정 파일 지원 (.ksjlintrc / .ksjfmtrc)

## P2 (Phase 9-10)
1. 세미콜론 필수화 로드맵 구체화 (경고 → 자동 변환 → 강제)
2. MemorySafetyTest/CompilerTest 확장 (새 모듈 커버)
3. stdlib/gui, network, collections 기초 구현
4. LLVM 백엔드 사전 연구 (IR 변환 프로토타입)

각 항목은 ROADMAP.md와 동기화되어 있으며, 완료 시 본 문서의 우선순위를 재조정합니다.
