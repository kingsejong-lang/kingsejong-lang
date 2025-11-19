# KingSejong 성능 벤치마크 결과

**테스트 환경**: macOS Darwin 24.5.0, Apple Silicon
**날짜**: 2025-11-20

## 벤치마크 결과

### KingSejong 단독 성능

| 벤치마크 | 평균 시간 | 최소 | 최대 |
|---------|----------|------|------|
| arithmetic | 0.043s | 0.043s | 0.043s |
| fibonacci(25) | 4.68s | 4.52s | 4.99s |
| loop_intensive | 0.051s | 0.049s | 0.055s |

### Python 3 비교

| 벤치마크 | KingSejong | Python 3 | 비율 |
|---------|-----------|----------|------|
| fibonacci(25) | 4.80s | 0.05s | Python 96x 빠름 |
| loop_intensive | 0.048s | 0.095s | KingSejong 2x 빠름 |

## 분석

### 재귀 호출 (fibonacci)
- Python이 약 **96배** 빠름
- 원인: 함수 호출 오버헤드가 큼
- 개선 방안: JIT 컴파일러 최적화, 함수 인라이닝

### 반복문 (loop_intensive)
- KingSejong이 **2배** 빠름
- 바이트코드 VM이 단순 산술/반복에서 효율적
- 스택 기반 연산이 잘 최적화됨

## 향후 개선 계획

1. **JIT 컴파일러 Tier-2**: 함수 인라이닝
2. **타입 특화 최적화**: 정수 연산 최적화
3. **루프 언롤링**: 반복문 최적화
4. **테일 콜 최적화**: 재귀 호출 최적화

## 실행 방법

```bash
# 전체 벤치마크 실행
cd benchmarks
python3 run_benchmarks.py --interpreter ../build/bin/kingsejong --runs 5

# Python 비교
python3 python/fibonacci.py
../build/bin/kingsejong fibonacci.ksj
```
