#!/usr/bin/env python3
"""피보나치 벤치마크 - Python 비교용"""

def fibonacci(n):
    if n <= 1:
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)

result = fibonacci(25)
print(f"피보나치(25) = {result}")
