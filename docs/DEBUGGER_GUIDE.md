# KingSejong 디버거 사용 가이드

> **날짜**: 2025-11-21
> **버전**: v0.5.0
> **대상**: KingSejong 개발자 및 사용자

---

## 목차

1. [개요](#1-개요)
2. [디버거 시작하기](#2-디버거-시작하기)
3. [기본 명령어](#3-기본-명령어)
4. [브레이크포인트 관리](#4-브레이크포인트-관리)
5. [단계별 실행](#5-단계별-실행)
6. [변수 검사](#6-변수-검사)
7. [콜 스택 조회](#7-콜-스택-조회)
8. [와치포인트](#8-와치포인트)
9. [실전 예제](#9-실전-예제)
10. [팁 & 트릭](#10-팁--트릭)

---

## 1. 개요

KingSejong 디버거는 GDB 스타일의 대화형 디버깅 도구입니다. 브레이크포인트, 단계별 실행, 변수 검사 등의 기능을 제공합니다.

### 주요 기능

- ✅ **브레이크포인트**: 특정 줄에서 실행 중단
- ✅ **조건부 브레이크포인트**: 조건이 참일 때만 중단
- ✅ **단계별 실행**: step (함수 안으로), next (다음 줄)
- ✅ **변수 검사**: 현재 범위의 변수 값 확인
- ✅ **콜 스택**: 함수 호출 추적
- ✅ **와치포인트**: 변수 값 변경 감지
- ✅ **소스 코드 뷰어**: 현재 실행 위치 표시

---

## 2. 디버거 시작하기

### 2.1 디버거 모드로 실행

```bash
# 디버거 모드로 프로그램 실행
./bin/kingsejong --debug examples/fibonacci.ksj

# 또는
kingsejong -d examples/fibonacci.ksj
```

### 2.2 디버거 프롬프트

디버거가 시작되면 다음과 같은 프롬프트가 표시됩니다:

```
KingSejong Debugger v0.5.0
Type 'help' for commands
(ksj-debug)
```

---

## 3. 기본 명령어

### 명령어 목록

| 명령어 | 단축키 | 설명 |
|--------|--------|------|
| `help` | `h` | 도움말 표시 |
| `run` | `r` | 프로그램 실행 시작 |
| `break` | `b` | 브레이크포인트 설정 |
| `delete` | `d` | 브레이크포인트 삭제 |
| `list` | `l` | 브레이크포인트 목록 |
| `step` | `s` | 한 줄 실행 (함수 안으로) |
| `next` | `n` | 다음 줄 실행 (함수 건너뛰기) |
| `continue` | `c` | 다음 브레이크포인트까지 실행 |
| `print` | `p` | 변수 값 출력 |
| `backtrace` | `bt` | 콜 스택 출력 |
| `watch` | `w` | 와치포인트 설정 |
| `quit` | `q` | 디버거 종료 |

### 도움말 보기

```
(ksj-debug) help
Available commands:
  help (h)          - Show this help message
  run (r)           - Start program execution
  break (b)         - Set breakpoint
  ...
```

---

## 4. 브레이크포인트 관리

### 4.1 브레이크포인트 설정

**기본 형식**:
```
break <파일>:<줄번호>
```

**예제**:
```
(ksj-debug) break fibonacci.ksj:3
Breakpoint 1 set at fibonacci.ksj:3
```

### 4.2 조건부 브레이크포인트

특정 조건이 참일 때만 멈추도록 설정할 수 있습니다.

**형식**:
```
break <파일>:<줄번호> if <조건>
```

**예제**:
```
(ksj-debug) break fibonacci.ksj:3 if n > 5
Conditional breakpoint 2 set at fibonacci.ksj:3 (condition: n > 5)
```

### 4.3 브레이크포인트 목록 조회

```
(ksj-debug) list
Breakpoints:
  1: fibonacci.ksj:3
  2: fibonacci.ksj:3 (condition: n > 5)
  3: fibonacci.ksj:11
```

### 4.4 브레이크포인트 삭제

```
(ksj-debug) delete 1
Breakpoint 1 deleted
```

---

## 5. 단계별 실행

### 5.1 Step (함수 안으로 들어가기)

`step` 또는 `s` 명령어는 한 줄씩 실행하며, 함수 호출이 있으면 함수 안으로 들어갑니다.

**예제**:
```
(ksj-debug) step
-> fibonacci.ksj:2    만약 (n <= 1) {
```

### 5.2 Next (함수 건너뛰기)

`next` 또는 `n` 명령어는 한 줄씩 실행하지만, 함수 호출은 건너뜁니다.

**예제**:
```
(ksj-debug) next
-> fibonacci.ksj:11    출력("F(", i, ") =", 피보나치(i))
```

### 5.3 Continue (다음 브레이크포인트까지)

`continue` 또는 `c` 명령어는 다음 브레이크포인트까지 실행을 계속합니다.

**예제**:
```
(ksj-debug) continue
Breakpoint 2 hit at fibonacci.ksj:3
-> fibonacci.ksj:3        반환 n
```

---

## 6. 변수 검사

### 6.1 변수 값 출력

`print` 또는 `p` 명령어로 변수 값을 확인할 수 있습니다.

**기본 사용**:
```
(ksj-debug) print n
n = 5
```

**표현식 계산**:
```
(ksj-debug) print n * 2
10
```

**배열 요소 접근**:
```
(ksj-debug) print arr[0]
arr[0] = 42
```

### 6.2 모든 지역 변수 출력

```
(ksj-debug) print *
Local variables:
  n = 5
  i = 3
  피보나치 = <function>
```

---

## 7. 콜 스택 조회

### 7.1 Backtrace (콜 스택 출력)

`backtrace` 또는 `bt` 명령어로 현재 함수 호출 스택을 확인할 수 있습니다.

**예제**:
```
(ksj-debug) backtrace
#0  피보나치(n=5) at fibonacci.ksj:3
#1  피보나치(n=6) at fibonacci.ksj:5
#2  main at fibonacci.ksj:11
```

### 7.2 스택 프레임 정보

각 스택 프레임은 다음 정보를 포함합니다:
- 프레임 번호 (#0, #1, ...)
- 함수 이름
- 매개변수 값
- 소스 위치 (파일:줄번호)

---

## 8. 와치포인트

### 8.1 와치포인트 설정

`watch` 또는 `w` 명령어로 변수 값 변경을 감지할 수 있습니다.

**형식**:
```
watch <변수명>
```

**예제**:
```
(ksj-debug) watch i
Watchpoint 1 set on variable 'i'
```

### 8.2 와치포인트 트리거

변수 값이 변경되면 자동으로 중단됩니다:

```
Watchpoint 1 triggered: 'i' changed from 3 to 4
-> fibonacci.ksj:12    i = i + 1
```

---

## 9. 실전 예제

### 9.1 피보나치 디버깅

**fibonacci.ksj**:
```ksj
정수 피보나치 = 함수(n) {
    만약 (n <= 1) {
        반환 n
    }
    반환 피보나치(n-1) + 피보나치(n-2)
}

출력("=== 피보나치 수열 ===")
정수 i = 0
10번 반복한다 {
    출력("F(", i, ") =", 피보나치(i))
    i = i + 1
}
```

**디버깅 세션**:

```bash
$ kingsejong --debug fibonacci.ksj

KingSejong Debugger v0.5.0
Type 'help' for commands

# 1. 브레이크포인트 설정
(ksj-debug) break fibonacci.ksj:3
Breakpoint 1 set at fibonacci.ksj:3

# 2. 조건부 브레이크포인트 설정 (n이 5일 때만)
(ksj-debug) break fibonacci.ksj:3 if n == 5
Conditional breakpoint 2 set at fibonacci.ksj:3

# 3. 프로그램 실행
(ksj-debug) run
=== 피보나치 수열 ===
F( 0 ) = 0
F( 1 ) = 1
...

Breakpoint 2 hit at fibonacci.ksj:3 (condition: n == 5)
-> fibonacci.ksj:3        반환 n

# 4. 변수 확인
(ksj-debug) print n
n = 5

# 5. 콜 스택 확인
(ksj-debug) backtrace
#0  피보나치(n=5) at fibonacci.ksj:3
#1  피보나치(n=6) at fibonacci.ksj:5
#2  main at fibonacci.ksj:11

# 6. 계속 실행
(ksj-debug) continue
F( 5 ) = 5
...

# 7. 종료
(ksj-debug) quit
```

### 9.2 와치포인트 사용 예제

```bash
(ksj-debug) break fibonacci.ksj:11
Breakpoint 1 set at fibonacci.ksj:11

(ksj-debug) run
Breakpoint 1 hit at fibonacci.ksj:11
-> fibonacci.ksj:11    출력("F(", i, ") =", 피보나치(i))

(ksj-debug) watch i
Watchpoint 1 set on variable 'i'

(ksj-debug) continue
F( 0 ) = 0
Watchpoint 1 triggered: 'i' changed from 0 to 1
-> fibonacci.ksj:12    i = i + 1

(ksj-debug) print i
i = 1
```

---

## 10. 팁 & 트릭

### 10.1 단축키 사용

자주 사용하는 명령어는 단축키로 빠르게 입력할 수 있습니다:

```
s     # step 대신
n     # next 대신
c     # continue 대신
p i   # print i 대신
bt    # backtrace 대신
```

### 10.2 조건부 브레이크포인트 활용

큰 루프에서 특정 조건일 때만 멈추려면:

```
(ksj-debug) break loop.ksj:5 if i > 100
```

### 10.3 소스 코드 보기

현재 실행 위치 주변 소스 코드를 확인하려면:

```
(ksj-debug) list
   8→ 출력("=== 피보나치 수열 ===")
   9→ 정수 i = 0
  10→ 10번 반복한다 {
→ 11→     출력("F(", i, ") =", 피보나치(i))
  12→     i = i + 1
  13→ }
```

### 10.4 표현식 계산

`print` 명령어로 복잡한 표현식도 계산할 수 있습니다:

```
(ksj-debug) print n * 2 + 10
20

(ksj-debug) print 피보나치(3)
2
```

### 10.5 브레이크포인트 일시 비활성화

브레이크포인트를 삭제하지 않고 일시적으로 비활성화하려면:

```
(ksj-debug) disable 1
Breakpoint 1 disabled

(ksj-debug) enable 1
Breakpoint 1 enabled
```

---

## 11. 문제 해결

### Q: 브레이크포인트가 작동하지 않아요

**A**: 다음 사항을 확인하세요:
1. 파일 경로가 정확한지 (상대 경로 vs 절대 경로)
2. 줄 번호가 실행 가능한 코드인지 (주석이나 빈 줄은 불가)
3. 프로그램이 실제로 해당 줄을 실행하는지

### Q: 변수 값이 표시되지 않아요

**A**: 변수가 현재 범위(scope)에 있는지 확인하세요. 함수 내부의 지역 변수는 해당 함수 실행 중에만 접근할 수 있습니다.

### Q: 재귀 함수를 디버깅하면 너무 깊게 들어가요

**A**: `next` 명령어를 사용하면 재귀 호출을 건너뛸 수 있습니다. 또는 조건부 브레이크포인트로 특정 깊이에서만 멈추도록 설정하세요.

---

## 12. 추가 자료

- [KingSejong 언어 가이드](./USER_GUIDE.md)
- [API 문서](./API.md)
- [예제 코드](../examples/)

---

**작성자**: KingSejong Team
**최종 수정**: 2025-11-21
**피드백**: https://github.com/anthropics/kingsejong-lang/issues
