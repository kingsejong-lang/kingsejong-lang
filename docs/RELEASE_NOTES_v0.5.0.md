# KingSejong v0.5.0 릴리스 노트

**릴리스 날짜**: 2025-11-23
**Production Readiness**: 8.9/10

---

## 🎉 개요

KingSejong v0.5.0은 코드 품질, 성능, 그리고 사용자 경험에 중점을 둔 메이저 업데이트입니다. 이번 릴리스는 대규모 리팩토링, Playground 대폭 개선, 그리고 표준 라이브러리 확장을 포함합니다.

### 주요 하이라이트

- ✨ **Playground 17개 템플릿** - 온라인 플레이그라운드에서 바로 시작 가능
- 🔧 **VM & Evaluator 리팩토링** - 코드 가독성 93% 향상
- 📚 **표준 라이브러리 251개 함수** - HTTP, DB, 암호화 등 실무 기능
- ✅ **1,506개 테스트 100% 통과** - 높은 품질과 안정성
- 🛠️ **코드 품질 도구** - Linter/Formatter 설정 파일 지원

---

## ✨ 새로운 기능

### 1. Playground 대폭 개선

온라인 Playground가 크게 개선되어 더욱 강력한 학습 및 실험 도구가 되었습니다.

#### 17개 즉시 사용 가능한 템플릿

**기본 템플릿 (9개)**
- Hello World - 첫 번째 프로그램
- 피보나치 수열 - 재귀 함수 예제
- 함수 예제 - 함수와 고차 함수
- 함수형 배열 - 걸러낸다, 변환한다, 축약한다
- 계산기 - 간단한 계산기
- 클래스 - 객체지향 프로그래밍
- 비동기 - async/await 패턴
- 배열 - 배열 조작 및 메서드
- 반복문 - for/while 루프

**stdlib 템플릿 (8개)**
- JSON - JSON 파싱 및 조작
- 시간/날짜 - 날짜 및 시간 처리
- 파일 I/O - 파일 읽기/쓰기
- 유틸리티 - 유용한 헬퍼 함수
- HTTP - REST API 호출 및 HTTP 요청
- 데이터베이스 - SQLite 연동
- 정규표현식 - 패턴 매칭
- 암호화 - 해싱 및 암호화

#### 편의 기능
- **코드 공유** - URL 기반 코드 공유로 협업 용이
- **결과 다운로드** - 실행 결과를 파일로 저장
- **클립보드 복사** - 원클릭 코드 복사

### 2. 표준 라이브러리 확장 (251개 함수)

실무에서 바로 사용 가능한 강력한 표준 라이브러리를 제공합니다.

#### 새로운 라이브러리

**stdlib/collections.ksj (49개 함수)**
```kingsejong
# 리스트 생성 및 조작
목록 = 리스트_생성([1, 2, 3, 4, 5])
리스트_추가(목록, 6)
리스트_정렬(목록, "desc")

# 맵(딕셔너리)
사전 = 맵_생성()
맵_넣기(사전, "이름", "킹세종")
값 = 맵_가져오기(사전, "이름")

# 집합
집합 = 집합_생성([1, 2, 3])
집합_추가(집합, 4)
```

**stdlib/http.ksj (21개 함수)**
```kingsejong
# HTTP GET 요청
응답 = HTTP_GET("https://api.example.com/data")

# HTTP POST 요청
데이터 = "{\"name\":\"킹세종\"}"
응답 = HTTP_POST("https://api.example.com/users", 데이터)

# 사용자 정의 헤더
헤더들 = [["Authorization", "Bearer token123"]]
응답 = HTTP_요청("GET", "https://api.example.com/protected", 헤더들, "")
```

**stdlib/db.ksj (14개 함수)**
```kingsejong
# SQLite 연동
디비 = DB_연결("users.db")
DB_실행(디비, "CREATE TABLE users (id INTEGER, name TEXT)")
DB_실행(디비, "INSERT INTO users VALUES (1, '킹세종')")
결과 = DB_조회(디비, "SELECT * FROM users")
DB_닫기(디비)
```

**stdlib/regex.ksj (19개 함수)**
```kingsejong
# 정규표현식 매칭
패턴 = "[0-9]+"
텍스트 = "전화번호: 010-1234-5678"
매치 = 정규식_매치(패턴, 텍스트)
```

**stdlib/crypto.ksj (23개 함수)**
```kingsejong
# SHA-256 해싱
해시 = SHA256("킹세종 언어")

# Base64 인코딩
인코딩 = Base64_인코딩("안녕하세요")
디코딩 = Base64_디코딩(인코딩)
```

**stdlib/os.ksj (24개 함수)**
```kingsejong
# 파일 시스템 조작
파일_복사("원본.txt", "복사본.txt")
파일_이동("파일.txt", "새폴더/파일.txt")
디렉토리_생성("새폴더")
목록 = 디렉토리_목록(".")
```

### 3. 코드 품질 도구

개발자 경험을 향상시키는 설정 파일 지원이 추가되었습니다.

**Linter 설정 (.ksjlintrc)**
```json
{
  "rules": {
    "unused_variables": "warn",
    "undefined_variables": "error",
    "unreachable_code": "warn",
    "missing_return": "error",
    "duplicate_keys": "error",
    "division_by_zero": "error",
    "infinite_loop": "warn",
    "type_mismatch": "warn"
  }
}
```

**Formatter 설정 (.ksjfmtrc)**
```json
{
  "indent_size": 4,
  "indent_style": "space",
  "max_line_length": 100,
  "insert_final_newline": true,
  "trim_trailing_whitespace": true,
  "normalize_blank_lines": true
}
```

---

## 🔧 주요 개선사항

### VM & Evaluator 리팩토링

코드 가독성과 유지보수성을 위한 대규모 리팩토링이 완료되었습니다.

#### VM::executeInstruction() - 93% 코드 감소
- **이전**: 790줄의 거대한 switch 문
- **이후**: 50줄 + 9개의 전문화된 메서드
- **결과**: 코드 이해도 향상, 버그 수정 용이

**분리된 메서드**
- `executeConstantOps()` - 상수 로드 연산
- `executeVariableOps()` - 변수 조작 연산
- `executeArithmeticOps()` - 산술 연산
- `executeComparisonOps()` - 비교 연산
- `executeLogicalOps()` - 논리 연산
- `executeControlFlowOps()` - 제어 흐름 (JIT 통합)
- `executeArrayOps()` - 배열 연산
- `executePromiseOps()` - 비동기/Promise
- `executeMiscOps()` - 클래스 시스템 등

#### Evaluator::evalCallExpression() - 80% 코드 감소
- **이전**: 345줄의 복잡한 조건문
- **이후**: 68줄 + 4개의 헬퍼 메서드
- **결과**: 함수 호출 로직 명확화

**분리된 메서드**
- `evalPromiseMethodCall()` - Promise 메서드 (then, catch 등)
- `evalClassMethodCall()` - 클래스 메서드 호출
- `evalConstructorCall()` - 생성자 호출
- `evalRegularFunctionCall()` - 일반 함수 호출

### Clean Code 적용

- ✅ 긴 함수 분석 및 분해
- ✅ 코드 스타일 통일 (clang-format)
- ✅ 중복 코드 제거
- ✅ 매직 넘버 상수화

### 문서화 강화

- ✅ Doxygen 설정 완료 (API 문서 자동 생성)
- ✅ 사용자 가이드 완성 (USER_GUIDE.md)
- ✅ 설정 가이드 추가 (CONFIG_GUIDE.md)
- ✅ README & ROADMAP 업데이트

---

## 📊 성능 및 품질

### 테스트 커버리지
- **1,506개 테스트** 100% 통과
- **0개 실패** - 완벽한 안정성
- regression_test.py 도입으로 회귀 테스트 자동화

### 성능 벤치마크
- 메모리 사용량: 3.7-4 MB (효율적)
- Python 대비 성능 비교 분석 완료
- JIT 컴파일러 통합 유지

### 코드 품질
- Production Readiness: **8.9/10**
- 코드 가독성 대폭 향상
- 유지보수성 개선

---

## 🔄 호환성

### 하위 호환성
- ✅ 기존 v0.4.x 코드 100% 호환
- ✅ API 변경 없음
- ✅ 마이그레이션 불필요

### 요구사항
- C++17 이상
- CMake 3.15 이상
- Python 3.8 이상 (테스트용)

---

## 📚 문서

### 새로운 문서
- [CHANGELOG.md](../CHANGELOG.md) - 버전별 변경 사항
- [CONFIG_GUIDE.md](CONFIG_GUIDE.md) - Linter/Formatter 설정 가이드
- [RELEASE_NOTES_v0.5.0.md](RELEASE_NOTES_v0.5.0.md) - 이 문서

### 업데이트된 문서
- [README.md](../README.md) - 프로젝트 개요
- [ROADMAP.md](ROADMAP.md) - 향후 계획
- [USER_GUIDE.md](USER_GUIDE.md) - 사용자 가이드

### 참고 문서
- [아키텍처 분석](ARCHITECTURE_ANALYSIS.md)
- [VM 분석](KINGSEJONG_VM_ANALYSIS.md)
- [언어 분석](KINGSEJONG_LANGUAGE_ANALYSIS.md)

---

## 🚀 시작하기

### 설치

```bash
# 저장소 클론
git clone https://github.com/kingsejong-lang/kingsejonglang.git
cd kingsejonglang

# 빌드
mkdir build && cd build
cmake ..
make -j$(nproc)

# 테스트
ctest --output-on-failure
```

### 온라인 Playground

브라우저에서 바로 시작하세요: [KingSejong Playground](https://play.kingsejong.dev)

1. 템플릿 갤러리에서 예제 선택
2. 코드 수정 및 실행
3. 결과 확인 및 공유

### 첫 번째 프로그램

```kingsejong
출력("안녕하세요, 킹세종!")

# 함수 정의
함수 인사말(이름) {
    반환 "안녕하세요, " + 이름 + "님!"
}

메시지 = 인사말("세종대왕")
출력(메시지)

# 배열 처리
숫자들 = [1, 2, 3, 4, 5]
제곱들 = 숫자들.변환한다(함수(x) { 반환 x * x })
출력("제곱:", 제곱들)
```

---

## 🎯 다음 단계

### v0.6.0 계획
- stdlib 추가 확장 (GUI, 네트워크)
- 추가 Linter 규칙
- 성능 최적화

### v1.0.0 목표
- Production Readiness 9.0/10 이상
- 완전한 API 문서
- 커뮤니티 에코시스템 구축

---

## 🙏 감사의 말

KingSejong 언어를 사용해 주셔서 감사합니다. 피드백과 기여를 환영합니다!

- **GitHub**: [kingsejong-lang/kingsejonglang](https://github.com/kingsejong-lang/kingsejonglang)
- **이슈 제보**: [GitHub Issues](https://github.com/kingsejong-lang/kingsejonglang/issues)
- **문의**: dev@kingsejong.dev

---

**KingSejong 팀**
2025년 11월 23일
