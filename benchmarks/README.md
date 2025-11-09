# KingSejong 벤치마크

> 성능 측정 및 최적화를 위한 벤치마크 스위트

## 개요

KingSejong 언어의 성능을 측정하기 위한 벤치마크 프로그램 모음입니다. Tree-walking 인터프리터와 바이트코드 컴파일러의 성능을 비교하고, 최적화 작업의 효과를 검증할 수 있습니다.

## 벤치마크 목록

### 1. arithmetic.ksj - 산술 연산
- **목적**: 기본 산술 연산 성능 측정
- **내용**:
  - 10,000회 덧셈 반복
  - 100회 곱셈 반복
  - 1,000회 나눗셈 반복
- **예상 실행 시간**: 0.1 - 0.5초

### 2. fibonacci.ksj - 함수 호출
- **목적**: 재귀 함수 호출 성능 측정
- **내용**:
  - 재귀 피보나치(25) 계산
  - 약 150,000회의 함수 호출
- **예상 실행 시간**: 0.5 - 2초

### 3. array_ops.ksj - 배열 조작
- **목적**: 배열 생성, 접근, 정렬 성능 측정
- **내용**:
  - 1,000개 요소 배열 생성
  - 배열 인덱스 접근 및 합계
  - 100개 요소 선택 정렬
- **예상 실행 시간**: 0.2 - 1초

### 4. loop_intensive.ksj - 반복문 집약
- **목적**: 중첩 반복문 성능 측정
- **내용**:
  - 100×100 중첩 반복문
  - 50×50 곱셈 연산
- **예상 실행 시간**: 0.1 - 0.5초

## 사용 방법

### 기본 실행

```bash
# 프로젝트 빌드
cd kingsejonglang
mkdir -p build && cd build
cmake ..
make

# 벤치마크 실행
cd ../benchmarks
python3 run_benchmarks.py
```

### 옵션

```bash
# 인터프리터 경로 지정
python3 run_benchmarks.py --interpreter /path/to/kingsejong

# 실행 횟수 지정 (기본: 5)
python3 run_benchmarks.py --runs 10

# 결과 파일 지정
python3 run_benchmarks.py --output results.txt

# 모든 옵션
python3 run_benchmarks.py --interpreter ../build/kingsejong --runs 10 --output results.txt
```

## 출력 예시

```
🚀 KingSejong 벤치마크 실행
   인터프리터: ../build/kingsejong
   실행 횟수: 5
   벤치마크 개수: 4

📊 arithmetic:
  Run 1/5: 0.1234s
  Run 2/5: 0.1256s
  Run 3/5: 0.1245s
  Run 4/5: 0.1238s
  Run 5/5: 0.1247s
  ✅ 평균: 0.1244s
     최소: 0.1234s
     최대: 0.1256s
     표준편차: 0.0008s

📊 fibonacci:
  Run 1/5: 1.5678s
  Run 2/5: 1.5702s
  Run 3/5: 1.5645s
  Run 4/5: 1.5689s
  Run 5/5: 1.5671s
  ✅ 평균: 1.5677s
     최소: 1.5645s
     최대: 1.5702s
     표준편차: 0.0020s

...

============================================================
📈 벤치마크 결과 요약
============================================================

벤치마크              평균 (s)     최소 (s)     최대 (s)
------------------------------------------------------------
arithmetic           0.1244       0.1234       0.1256
fibonacci            1.5677       1.5645       1.5702
array_ops            0.4523       0.4501       0.4567
loop_intensive       0.2134       0.2120       0.2156
------------------------------------------------------------
총 실행 시간           2.3578
성공한 벤치마크         4/4

✅ 결과 저장: benchmark_results.txt
```

## 결과 파일

`benchmark_results.txt` 파일에 상세한 결과가 저장됩니다:

```
KingSejong 벤치마크 결과
============================================================
인터프리터: ../build/kingsejong
실행 횟수: 5

arithmetic:
  평균: 0.1244s
  최소: 0.1234s
  최대: 0.1256s
  표준편차: 0.0008s
  실행 시간: [0.1234, 0.1256, 0.1245, 0.1238, 0.1247]

fibonacci:
  평균: 1.5677s
  최소: 1.5645s
  최대: 1.5702s
  표준편차: 0.0020s
  실행 시간: [1.5678, 1.5702, 1.5645, 1.5689, 1.5671]

...
```

## Tree-walking vs Bytecode 비교

향후 바이트코드 컴파일러가 완성되면 두 방식의 성능을 비교할 수 있습니다:

```bash
# Tree-walking 인터프리터
python3 run_benchmarks.py --interpreter ../build/kingsejong --output tree_walking.txt

# 바이트코드 컴파일러 (향후)
python3 run_benchmarks.py --interpreter ../build/kingsejong_bytecode --output bytecode.txt

# 결과 비교
python3 compare_results.py tree_walking.txt bytecode.txt
```

**예상 결과**:
- 산술 연산: 바이트코드 1.5-2배 빠름
- 함수 호출: 바이트코드 2-3배 빠름
- 배열 조작: 바이트코드 1.2-1.5배 빠름
- 반복문: 바이트코드 2-2.5배 빠름

## CI 통합

GitHub Actions에서 자동으로 벤치마크를 실행하여 성능 회귀를 감지할 수 있습니다:

```yaml
- name: Run Benchmarks
  run: |
    cd benchmarks
    python3 run_benchmarks.py --runs 3
```

## 새 벤치마크 추가

새로운 벤치마크를 추가하려면:

1. `benchmarks/` 디렉토리에 `.ksj` 파일 작성
2. 벤치마크 이름을 파일명으로 사용
3. `run_benchmarks.py`가 자동으로 감지하여 실행

예시:
```ksj
# benchmarks/my_benchmark.ksj

# 벤치마크 코드 작성
i가 1부터 1000까지 {
    # ...
}

출력("완료")
```

## 성능 최적화 가이드

벤치마크 결과를 바탕으로 최적화 작업을 진행할 수 있습니다:

1. **병목 지점 파악**: 가장 느린 벤치마크 확인
2. **프로파일링**: 해당 코드 상세 분석
3. **최적화 구현**: 컴파일러 또는 VM 개선
4. **재측정**: 벤치마크로 효과 검증
5. **반복**: 목표 성능 달성까지 반복

## 기여

새로운 벤치마크 제안이나 성능 개선 아이디어가 있다면 이슈나 PR을 생성해주세요!

---

**버전**: v0.1.0
**마지막 업데이트**: 2025-11-10
