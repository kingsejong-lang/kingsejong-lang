# 언어 비교 분석 (Phase 8 스냅샷)

**기준 버전**: v0.8.0-dev  
**최종 업데이트**: 2025-11-20

| 항목 | KingSejong | Python | JavaScript (Node) | Swift |
|------|------------|--------|-------------------|-------|
| 구문 | 한국어 조사/범위/패턴 매칭, 세미콜론 선택 | 들여쓰기 기반 | C 스타일, async/await | C 스타일, 강타입 |
| 실행 엔진 | Tree-walk + Bytecode VM + Tier1 JIT | CPython 인터프리터 | V8 (JIT) | LLVM AOT + JIT |
| 메모리 | 세대별 GC + shared_ptr | 참조 카운트 + GC | 세대별 GC | ARC |
| 비동기 | Async/Await (Phase 7.3) | asyncio (라이브러리) | first-class | async/await |
| 도구 | LSP, 디버거, ksjpm, VS Code 확장 | 풍부 | 풍부 | Xcode/
| 표준 라이브러리 | 173 함수 (http/db 예정) | 방대 | 방대 | 방대 |
| 문서 | 언어/튜토리얼/설치 + Phase 8에서 보강 중 | 방대 | 방대 | 방대 |

## 강점
- 자연어 기반 문법 + 조사 체인 → 교육 친화적 경험
- JIT Tier 1 + 세대별 GC → 구현 난이도 대비 우수한 성능/안정성
- 개발자 도구 (LSP, 디버거, VS Code 확장, 패키지 관리자) 제공

## 개선 포인트
1. 표준 라이브러리 확장 (http/db/collections) → Python/JS 대비 경쟁력 확보
2. Incremental GC 도입으로 Pause Time 개선 (Swift/V8 수준 목표)
3. API/사용자 문서 보완으로 Swift/Python과 비슷한 온보딩 경험 제공
