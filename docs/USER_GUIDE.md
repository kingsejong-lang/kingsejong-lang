# KingSejong 언어 사용자 가이드

> **문서 목적**: 언어 기능 레퍼런스 및 도구 사용법 (빠른 참조용)
> **버전**: v0.6.0-rc
> **최종 업데이트**: 2025-11-26
> **대상 독자**: KingSejong 언어의 특정 기능을 찾아보려는 개발자

**💡 문서 선택 가이드:**
- **처음 배우시는 분**: [튜토리얼](TUTORIAL.md)을 먼저 보세요 (단계별 학습 경로)
- **특정 기능 찾으시는 분**: 이 문서를 참조하세요 (기능별 레퍼런스)
- **완전한 문법 스펙**: [언어 레퍼런스](LANGUAGE_REFERENCE.md) 참조

---

## 📚 목차

1. [빠른 시작](#빠른-시작)
2. [개발 환경 설정](#개발-환경-설정)
3. [주요 기능 가이드](#주요-기능-가이드)
4. [도구 사용법](#도구-사용법)
5. [표준 라이브러리](#표준-라이브러리)
6. [디버깅 & 문제 해결](#디버깅--문제-해결)
7. [모범 사례](#모범-사례)
8. [FAQ](#faq)

---

## 빠른 시작

### 설치

```bash
# 저장소 클론
git clone https://github.com/yourusername/kingsejonglang.git
cd kingsejonglang

# 빌드
mkdir build && cd build
cmake ..
make -j8

# 설치
sudo make install
```

### 첫 프로그램 작성

`hello.ksj` 파일을 생성하세요:

```ksj
# 간단한 인사 프로그램
출력("안녕하세요, KingSejong!")

정수 나이 = 25
문자열 이름 = "세종"
출력(이름 + "님의 나이는 " + 나이를_문자열로(나이) + "살입니다")
```

### 실행

```bash
kingsejong hello.ksj
```

출력:
```
안녕하세요, KingSejong!
세종님의 나이는 25살입니다
```

---

## 개발 환경 설정

### VS Code 확장 설치

1. VS Code 열기
2. 확장 탭에서 "KingSejong" 검색
3. 설치 후 `.ksj` 파일 열기

**제공 기능**:
- 구문 강조
- 자동 완성
- 정의 이동 (F12)
- 호버 정보
- 오류 진단
- 코드 포맷팅 (Shift+Alt+F)

### REPL 모드

대화형으로 코드를 실행하려면:

```bash
kingsejong
```

```
KingSejong REPL v0.8.0
종료하려면 'exit' 또는 Ctrl+D를 입력하세요.

>>> 정수 x = 10
>>> 정수 y = 20
>>> 출력(x + y)
30
>>> exit
```

---

## 주요 기능 가이드

### 1. 한글 조사 체인 (독창적 문법)

KingSejong의 가장 독특한 기능입니다:

```ksj
배열 숫자들 = [1, 2, 3, 4, 5]

# 체이닝으로 데이터 변환
배열 결과 = 숫자들을
    .필터(함수(x) { 반환 x > 2 })
    .맵(함수(x) { 반환 x * 2 })

출력(결과)  # [6, 8, 10]
```

### 2. 클래스와 객체 지향

```ksj
클래스 사람 {
    생성자(이름, 나이) {
        나.이름 = 이름
        나.나이 = 나이
    }

    메서드 인사하다() {
        출력("안녕하세요, 저는 " + 나.이름 + "입니다")
    }

    메서드 나이먹다() {
        나.나이 = 나.나이 + 1
    }
}

변수 철수 = 사람("철수", 25)
철수.인사하다()
철수.나이먹다()
출력(철수.나이)  # 26
```

### 3. 비동기 프로그래밍 (Async/Await)

```ksj
async 함수 데이터_가져오기(url) {
    변수 응답 = await http.get(url)
    반환 응답.json()
}

async 함수 메인() {
    변수 데이터 = await 데이터_가져오기("https://api.example.com")
    출력(데이터)
}

메인()
```

### 4. 예외 처리

```ksj
시도 {
    정수 결과 = 10 / 0
} 잡기 (오류) {
    출력("에러 발생: " + 오류.메시지)
} 마지막 {
    출력("정리 작업 수행")
}
```

### 5. 패턴 매칭

```ksj
함수 처리하다(값) {
    매칭 (값) {
        0 => 출력("영"),
        1 => 출력("일"),
        2 => 출력("이"),
        _ => 출력("기타")
    }
}

처리하다(1)  # "일"
```

### 6. 모듈 시스템

**math_utils.ksj**:
```ksj
함수 제곱(x) {
    반환 x * x
}

함수 세제곱(x) {
    반환 x * x * x
}

내보내기 { 제곱, 세제곱 }
```

**main.ksj**:
```ksj
가져오기 "./math_utils.ksj"

출력(제곱(5))     # 25
출력(세제곱(3))   # 27
```

---

## 도구 사용법

### 1. Linter (ksjlint)

코드 품질을 검사합니다:

```bash
kingsejong --lint myfile.ksj
```

**검사 항목**:
- 미사용 변수
- 도달 불가능한 코드
- 자기 비교
- 상수 조건
- 빈 블록

### 2. Formatter (ksjfmt)

코드를 자동으로 포맷팅합니다:

```bash
kingsejong --format myfile.ksj
```

**설정 파일** (`.ksjfmt.json`):
```json
{
    "indent_size": 4,
    "use_tabs": false,
    "max_line_length": 100
}
```

### 3. Profiler

성능 분석:

```bash
kingsejong --profile myfile.ksj
```

출력:
```
함수 호출 통계:
  fibonacci: 1000회, 2.5초
  calculate: 500회, 0.8초
```

### 4. 디버거

중단점을 설정하고 단계별 실행:

```bash
kingsejong --debug myfile.ksj
```

**디버거 명령어**:
- `break <line>` - 중단점 설정
- `next` - 다음 줄 실행
- `step` - 함수 안으로 들어가기
- `continue` - 계속 실행
- `print <var>` - 변수 출력
- `quit` - 종료

---

## 표준 라이브러리

### stdlib/io.ksj - 입출력

```ksj
가져오기 "./stdlib/io.ksj"

# 파일 쓰기
파일_쓰기("test.txt", "Hello, World!")

# 파일 읽기
변수 내용 = 파일_읽기("test.txt")
출력(내용)

# 사용자 입력
변수 이름 = 입력("이름을 입력하세요: ")
출력("안녕하세요, " + 이름 + "님!")
```

### stdlib/json.ksj - JSON 처리

```ksj
가져오기 "./stdlib/json.ksj"

변수 데이터 = {
    "이름": "세종",
    "나이": 25,
    "취미": ["독서", "음악"]
}

# JSON 문자열로 변환
변수 json문자열 = json_stringify(데이터)
출력(json문자열)

# JSON 파싱
변수 파싱된 = json_parse(json문자열)
출력(파싱된.이름)
```

### stdlib/time.ksj - 시간 처리

```ksj
가져오기 "./stdlib/time.ksj"

# 현재 타임스탬프
변수 지금 = 현재_시각()
출력(지금)

# 날짜 포맷팅
변수 날짜문자열 = 시각_포맷(지금, "%Y-%m-%d %H:%M:%S")
출력(날짜문자열)

# 시간 계산
변수 내일 = 일_더하기(지금, 1)
```

### stdlib/regex.ksj - 정규표현식

```ksj
가져오기 "./stdlib/regex.ksj"

# 패턴 매칭
변수 결과 = 일치("hello@example.com", "^[a-z]+@[a-z]+\\.com$")
출력(결과)  # 참

# 이메일 검증
변수 유효 = 이메일_검증("user@domain.com")
출력(유효)  # 참
```

### stdlib/crypto.ksj - 암호화

```ksj
가져오기 "./stdlib/crypto.ksj"

# Base64 인코딩
변수 인코딩된 = base64_인코딩("Hello")
출력(인코딩된)

# 해시
변수 해시값 = sha256_해시("비밀번호")
출력(해시값)

# 비밀번호 해싱
변수 해시 = 비밀번호_해싱("my_password")
변수 확인 = 비밀번호_검증("my_password", 해시)
출력(확인)  # 참
```

### stdlib/http.ksj - HTTP 클라이언트

```ksj
가져오기 "./stdlib/http.ksj"

# GET 요청
변수 응답 = http_get("https://api.github.com")
출력(응답.status)
출력(응답.body)

# POST 요청
변수 데이터 = {"name": "세종"}
변수 결과 = http_post("https://api.example.com", 데이터)
```

---

## 디버깅 & 문제 해결

### 일반적인 오류

#### 1. "변수를 찾을 수 없습니다"

```ksj
# 잘못된 코드
출력(x)  # 오류: 변수 'x'가 선언되지 않음

# 올바른 코드
정수 x = 10
출력(x)
```

#### 2. "타입 불일치"

```ksj
# 잘못된 코드
정수 x = 10
x = "문자열"  # 오류: 정수 타입에 문자열 할당

# 올바른 코드
변수 x = 10
x = "문자열"  # OK: 변수 타입은 동적
```

#### 3. "모듈을 찾을 수 없습니다"

```ksj
# 상대 경로 사용
가져오기 "./stdlib/io.ksj"  # OK

# 절대 경로
가져오기 "/full/path/to/module.ksj"  # OK
```

### 성능 최적화 팁

1. **반복문 최적화**
   ```ksj
   # 느림
   정수 i = 0
   반복 (i < 배열.길이()) {
       # 매번 길이() 호출
   }

   # 빠름
   정수 길이 = 배열.길이()
   정수 i = 0
   반복 (i < 길이) {
       # 한 번만 호출
   }
   ```

2. **JIT 활성화**
   ```bash
   # JIT 컴파일러 활성화로 2-5배 성능 향상
   kingsejong --jit myfile.ksj
   ```

3. **메모리 관리**
   ```ksj
   # 큰 배열은 필요 없을 때 null로 설정
   배열 큰데이터 = [1, 2, 3, ...]
   # ... 사용 ...
   큰데이터 = null  # GC가 메모리 해제
   ```

---

## 모범 사례

### 1. 네이밍 컨벤션

```ksj
# 변수: snake_case 또는 camelCase
정수 사용자_나이 = 25
정수 userAge = 25

# 함수: snake_case
함수 계산_합계(a, b) {
    반환 a + b
}

# 클래스: PascalCase
클래스 사용자계정 {
    # ...
}

# 상수: UPPER_SNAKE_CASE
정수 최대_크기 = 1000
```

### 2. 코드 구조

```ksj
# 1. 모듈 가져오기
가져오기 "./stdlib/io.ksj"
가져오기 "./stdlib/json.ksj"

# 2. 상수 선언
정수 최대_재시도 = 3

# 3. 함수 정의
함수 초기화() {
    # ...
}

# 4. 메인 로직
함수 메인() {
    초기화()
    # ...
}

# 5. 실행
메인()
```

### 3. 에러 처리

```ksj
# 항상 예외 처리
함수 파일_처리(경로) {
    시도 {
        변수 내용 = 파일_읽기(경로)
        반환 내용
    } 잡기 (오류) {
        출력("파일 읽기 실패: " + 오류.메시지)
        반환 null
    }
}
```

### 4. 문서화

```ksj
# 함수 설명을 주석으로 작성
# 두 수의 합을 계산합니다
# 매개변수:
#   - a: 첫 번째 정수
#   - b: 두 번째 정수
# 반환: 두 수의 합
함수 더하기(a, b) {
    반환 a + b
}
```

---

## FAQ

### Q1: KingSejong은 어떤 프로젝트에 적합한가요?

**A**:
- 교육용 프로그래밍
- 한글 기반 스크립트
- 중소 규모 자동화 도구
- 데이터 처리 파이프라인

### Q2: 성능은 어느 정도인가요?

**A**:
- 반복문: Python보다 2배 빠름
- 재귀 호출: Python보다 느림 (최적화 예정)
- 메모리: 약 4MB (경량)

### Q3: 패키지 관리는 어떻게 하나요?

**A**:
```bash
# 패키지 설치
ksjpm install <package-name>

# 패키지 목록
ksjpm list

# 패키지 제거
ksjpm remove <package-name>
```

### Q4: 다른 언어와 연동할 수 있나요?

**A**: 현재는 독립 실행만 지원합니다. FFI (Foreign Function Interface)는 향후 계획에 있습니다.

### Q5: 프로덕션에 사용할 수 있나요?

**A**: v0.8.0은 안정성이 높지만 (8.5/10), v1.0.0 릴리스를 권장합니다.

### Q6: 커뮤니티 지원은 어디서 받나요?

**A**:
- GitHub Issues: 버그 리포트 및 기능 요청
- GitHub Discussions: 질문 및 토론
- 문서: docs/ 디렉토리

---

## 추가 리소스

- **튜토리얼**: [TUTORIAL.md](./TUTORIAL.md) - 단계별 학습
- **언어 레퍼런스**: [LANGUAGE_REFERENCE.md](./LANGUAGE_REFERENCE.md) - 완전한 문법
- **라이브러리 가이드**: [LIBRARY_GUIDE.md](./LIBRARY_GUIDE.md) - stdlib 상세
- **설치 가이드**: [INSTALL.md](./INSTALL.md) - 플랫폼별 설치
- **로드맵**: [ROADMAP.md](./ROADMAP.md) - 개발 계획

---

**마지막 업데이트**: 2025-11-20
**버전**: v0.8.0
**라이선스**: MIT
