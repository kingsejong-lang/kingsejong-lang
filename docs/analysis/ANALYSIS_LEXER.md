# Lexer 분석 (Phase 8 기준)

**기준 버전**: v0.8.0-dev  
**최신 테스트**: LexerTest 44개, ParserTest 44개, 전부 통과

## 현황
- 한국어 키워드/조사/범위 키워드 + Phase 7 클래스 키워드(클래스/생성자/공개/비공개/자신/상속/상위) 모두 Lexer 토큰 집합에 포함.
- 주석(`#[...]` 단일 라인) 지원은 구현되어 있으며, 튜토리얼/레퍼런스 문서 업데이트가 남아 있음.
- 타입 없는 변수 선언을 허용하므로, Identifier 토큰 유효성 검사만 수행하고 타입 키워드는 선택 사항.

## 최근 변경
1. snapshot()/restore() 기반 lookahead 보조 API 유지 → Parser의 LL(4) 구현과 호환.
2. 토큰 스트림의 Unicode 처리 개선 (UTF8Utils 통합) → 조사 체인 파싱 정확도 향상.

## 해야 할 일
- Lexer 주석 동작에 대한 문서화 (LANGUAGE_REFERENCE, 튜토리얼 전반) → Phase 8.2.
- 새로운 stdlib 키워드가 도입될 경우 Token.h/Token.cpp/TokenType enum을 즉시 업데이트하는 체크리스트 필요.
