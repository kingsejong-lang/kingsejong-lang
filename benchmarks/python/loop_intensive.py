#!/usr/bin/env python3
"""반복문 집약 벤치마크 - Python 비교용"""

합계 = 0
for i in range(1, 101):
    for j in range(1, 101):
        합계 += 1

곱셈합 = 0
for i in range(1, 51):
    for j in range(1, 51):
        곱셈합 += i * j

print("반복문 완료")
print(합계)
print(곱셈합)
