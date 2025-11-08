# KingSejong Programming Language

**한글로 프로그래밍하는 자연스러운 언어**

KingSejong은 한국어의 문법적 특성을 활용한 혁신적인 프로그래밍 언어입니다. 한글 조사를 활용한 메서드 체이닝, 자연스러운 범위 표현, 명시적 타입 시스템으로 직관적인 코드 작성을 지원합니다.

## 주요 특징

### 1. 조사 기반 메서드 체이닝
한국어의 조사를 활용하여 자연스러운 메서드 체이닝을 구현합니다.

```ksj
배열을 정렬하고 역순으로 나열하고 출력하라
// 영어: array.sort().reverse().print()
```

지원하는 조사:
- `을/를`: 목적격 조사
- `의`: 소유격 조사
- `로/으로`: 방향/수단 조사
- `에서`: 위치 조사

### 2. 자연스러운 범위 표현
수학적으로 직관적인 범위 표현을 제공합니다.

```ksj
1부터 10까지  // [1, 2, ..., 10] - 10 포함!
1부터 10미만  // [1, 2, ..., 9] - 10 미포함
```

### 3. 명시적 타입 시스템
한글로 타입을 명시하여 가독성을 높입니다.

```ksj
정수 나이 = 25
실수 키 = 175.5
문자열 이름 = "홍길동"
논리 참거짓 = 참
```

### 4. 직관적인 제어문
한글로 작성된 제어문으로 코드 흐름을 명확하게 표현합니다.

```ksj
만약 (나이 >= 20) {
    "성인입니다"를 출력하라
} 아니면 {
    "미성년자입니다"를 출력하라
}

1부터 10까지를 반복 (숫자) {
    숫자를 출력하라
}
```

## 빌드 및 설치

### 요구사항
- C++23 호환 컴파일러 (GCC 13+, Clang 16+, MSVC 2022+)
- CMake 3.20 이상
- GoogleTest (테스트용)

### 빌드 방법

```bash
# 저장소 클론
git clone https://github.com/yourusername/kingsejong-lang.git
cd kingsejong-lang

# 빌드 디렉토리 생성
mkdir build && cd build

# CMake 설정
cmake ..

# 빌드
cmake --build .

# 테스트 실행
ctest --output-on-failure
```

### 실행

```bash
# REPL 모드
./kingsejong

# 파일 실행
./kingsejong script.ksj
```

## 프로젝트 구조

```
kingsejong-lang/
├── src/                    # 소스 코드
│   ├── lexer/             # 어휘 분석기
│   ├── parser/            # 구문 분석기
│   ├── ast/               # 추상 구문 트리
│   ├── evaluator/         # 평가기
│   └── main.cpp           # 진입점
├── tests/                  # 테스트 코드
├── examples/              # 예제 코드
├── docs/                  # 문서
│   ├── TODOLIST.md        # 작업 목록
│   ├── LANGUAGE_DESIGN_PHILOSOPHY.md
│   └── SEJONG_FEATURES.md
├── KNOWN_ISSUES.md        # 알려진 이슈
├── CMakeLists.txt         # 빌드 설정
├── LICENSE                # MIT 라이센스
└── README.md              # 이 파일
```

## 구현 상태

### Phase 1 진행 현황 (핵심 기능)

| 기능 | 상태 | 테스트 | 비고 |
|------|------|--------|------|
| F1.1: Token 시스템 | ✅ 완료 | 100% | UTF-8 한글 지원 |
| F1.2: Josa 인식기 | ✅ 완료 | 100% | 받침 규칙 기반 |
| F1.3: 범위 표현식 | ✅ 완료 | 100% | 부터/까지/미만/초과/이하/이상 |
| F1.13: 조건문 (if/else) | ✅ 완료 | 100% | 만약/아니면 |
| F1.12: 반복문 | 🟡 부분 완료 | 81% (17/21) | N번 반복, 범위 for |

**알려진 이슈**: 반복문 일부 테스트 실패 (4/21) - 자세한 내용은 [KNOWN_ISSUES.md](KNOWN_ISSUES.md) 참조

### 전체 테스트 현황

```
총 테스트: 255개
통과: 250개 (98.0%)
실패: 5개 (2.0%)
비활성화: 3개
```

주요 실패 원인:
- Assignment statement 미구현 (1개)
- Expression 기반 반복 횟수 파싱 (2개)
- 변수 기반 범위 경계 파싱 (1개)
- Josa 표현식 파싱 (1개)

## 개발 로드맵

프로젝트는 4단계로 진행됩니다:

- **Phase 0** (Week 1-2): 프로젝트 초기 설정
- **Phase 1** (Month 1-3): 핵심 차별화 기능 (조사 인식, 범위 표현, 타입 시스템)
- **Phase 2** (Month 4-6): 실용 기능 (함수, 모듈, 표준 라이브러리)
- **Phase 3** (Month 7-12): 고급 기능 (GC, 최적화, 크로스 컴파일)

자세한 내용은 [TODOLIST.md](docs/TODOLIST.md)를 참조하세요.

## 기여하기

기여를 환영합니다! 기여 방법은 다음과 같습니다:

1. 이 저장소를 Fork 합니다
2. Feature 브랜치를 생성합니다 (`git checkout -b feature/AmazingFeature`)
3. 변경사항을 커밋합니다 (`git commit -m 'Add some AmazingFeature'`)
4. 브랜치에 Push 합니다 (`git push origin feature/AmazingFeature`)
5. Pull Request를 생성합니다

자세한 가이드라인은 [CONTRIBUTING.md](docs/CONTRIBUTING.md)를 참조하세요.

## 라이센스

이 프로젝트는 MIT 라이센스 하에 배포됩니다. 자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.

## 감사의 말

이 프로젝트는 한국어의 아름다움과 프로그래밍의 논리성을 결합하려는 시도입니다.
세종대왕의 창제 정신을 이어받아, 누구나 쉽게 프로그래밍할 수 있는 언어를 만들고자 합니다.
