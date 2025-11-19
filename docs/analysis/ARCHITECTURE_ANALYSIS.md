# 아키텍처 분석 (Phase 8 스냅샷)

**기준 버전**: v0.8.0-dev  
**최종 업데이트**: 2025-11-20

## 시스템 개요
- 프론트엔드: Lexer → Parser → AST → Semantic Analyzer
- 실행 경로: Evaluator(Tree-walk) + Bytecode VM + Tier 1 JIT
- 도구: LSP, 디버거, ksjpm, Formatter, Linter, VS Code 확장
- 인프라: 세대별 GC, regression_test.py, 1,490 자동 테스트

## 계층별 상태
| 계층 | 상태 | TODO |
|------|------|------|
| 프론트엔드 | ✅ 기능 완성 | Panic mode 파서, 추가 진단 |
| Semantic Analyzer | ✅ Phase 6 완료 | 접근 제어 검증 보강 검토 |
| Backend (VM/JIT) | ✅ Tier 1 | Incremental GC, 메모리 풀링 |
| Dev Tools | ✅ | 문서/가이드 확장 |
| 표준 라이브러리 | 🚧 | http/db/collections |

## 기술 부채 지도
1. 긴 함수 (VM/Evaluator): Phase 9 리팩토링 티켓 필요
2. Incremental GC/Memory Pool: Design 문서 부재 → Phase 6 carry-over
3. regression_test.py를 CI에 연결하여 아키텍처 회귀 감지
4. 문서/요약 파일 재생성: Phase 8 마무리 시점에 자동화 필요

## 추천 일정
- Phase 8: 문서/테스트 자동화 마무리 → v1.0.0
- Phase 9: 리팩토링 + Incremental GC 설계
- Phase 10+: LLVM 백엔드, 멀티스레딩, 세미콜론 정책 실행
