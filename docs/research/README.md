# 연구 노트

구현에 들어가기 전에 쓴 조사 문서다. 현재 코드의 동작을 설명하지 않으며, 문서 사이트에는 싣지 않는다
(`site/.vitepress/config.mts` 의 `srcExclude: ['research/**']`).

| 문서 | 내용 | 현재 상태 |
|---|---|---|
| [JIT_COMPILER_RESEARCH.md](JIT_COMPILER_RESEARCH.md) | 바이트코드 VM 위의 JIT 컴파일러 설계 조사 (2025-11-10) | JIT Tier 1·2 는 구현했다가 2026-10 에 제거했다. 이유는 [CHANGELOG](../../CHANGELOG.md) 의 Unreleased "제거" 항목에 있다 |
| [LLVM_INTEGRATION.md](LLVM_INTEGRATION.md) | LLVM 백엔드 통합 조사 (2025-11-10) | 구현하지 않았다. [로드맵](../ROADMAP.md) 의 v0.9.0 선택 항목으로 남아 있다 |

JIT 나 LLVM 백엔드를 다시 시작할 때 출발점으로 쓴다. 로드맵은 JIT 를 다시 할 경우 VM 이 평가기와 동치가 된 뒤에 시작한다고 적고 있다.
