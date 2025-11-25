#!/usr/bin/env python3
"""성능 회귀 테스트 스크립트

CI에서 성능 회귀를 감지하기 위한 자동화된 벤치마크 테스트.
각 벤치마크의 실행 시간이 임계값을 초과하면 실패합니다.
"""

import subprocess
import sys
import os
import time
import json
from pathlib import Path

# 성능 임계값 (초) - 이 값을 초과하면 회귀로 판단
# 20% 마진을 포함한 값
THRESHOLDS = {
    'arithmetic': 0.1,       # 기준: 0.043s, 임계: 0.1s
    'loop_intensive': 0.1,   # 기준: 0.051s, 임계: 0.1s
    'fibonacci_15': 0.5,     # fibonacci(15) - 더 작은 값으로 CI용
    'array_ops': 0.15,       # 배열 조작 벤치마크
    'memory_test': 0.2,      # 메모리 사용량 벤치마크
}

# 각 벤치마크 실행 횟수
NUM_RUNS = 3


def get_project_root():
    """프로젝트 루트 디렉토리 반환"""
    return Path(__file__).parent.parent


def get_interpreter_path():
    """인터프리터 경로 반환"""
    project_root = get_project_root()
    interpreter = project_root / 'build' / 'bin' / 'kingsejong'

    if not interpreter.exists():
        print(f"오류: 인터프리터를 찾을 수 없음: {interpreter}")
        print("먼저 프로젝트를 빌드하세요: mkdir build && cd build && cmake .. && make")
        sys.exit(1)

    return str(interpreter)


def run_benchmark(interpreter, script_path, timeout=30):
    """단일 벤치마크 실행 및 시간 측정"""
    start_time = time.time()

    try:
        result = subprocess.run(
            [interpreter, script_path],
            capture_output=True,
            text=True,
            timeout=timeout
        )

        elapsed = time.time() - start_time

        if result.returncode != 0:
            return None, f"실행 실패: {result.stderr}"

        return elapsed, None

    except subprocess.TimeoutExpired:
        return None, f"타임아웃 ({timeout}초)"
    except Exception as e:
        return None, str(e)


def run_benchmark_suite(interpreter, benchmarks_dir):
    """전체 벤치마크 스위트 실행"""
    results = {}

    for name, threshold in THRESHOLDS.items():
        # fibonacci_15는 특별한 스크립트 필요
        if name == 'fibonacci_15':
            script_path = benchmarks_dir / 'fibonacci_15.ksj'
            if not script_path.exists():
                # fibonacci_15.ksj 생성
                create_fibonacci_15(script_path)
        else:
            script_path = benchmarks_dir / f'{name}.ksj'

        if not script_path.exists():
            print(f"경고: 벤치마크 파일 없음: {script_path}")
            continue

        times = []
        errors = []

        print(f"실행 중: {name} ({NUM_RUNS}회)", end=" ", flush=True)

        for i in range(NUM_RUNS):
            elapsed, error = run_benchmark(interpreter, str(script_path))

            if error:
                errors.append(error)
                print("X", end="", flush=True)
            else:
                times.append(elapsed)
                print(".", end="", flush=True)

        print()

        if times:
            avg_time = sum(times) / len(times)
            min_time = min(times)
            max_time = max(times)

            results[name] = {
                'avg': avg_time,
                'min': min_time,
                'max': max_time,
                'threshold': threshold,
                'passed': avg_time <= threshold,
                'runs': len(times)
            }
        else:
            results[name] = {
                'error': errors[0] if errors else "알 수 없는 오류",
                'passed': False
            }

    return results


def create_fibonacci_15(path):
    """CI용 작은 피보나치 벤치마크 생성"""
    code = '''// 피보나치 벤치마크 (CI용)
함수 피보나치(n) {
    만약 (n <= 1) {
        반환 n
    }
    반환 피보나치(n - 1) + 피보나치(n - 2)
}

정수 결과 = 피보나치(15)
출력(결과)
'''
    with open(path, 'w', encoding='utf-8') as f:
        f.write(code)


def print_results(results):
    """결과 출력"""
    print("\n" + "=" * 60)
    print("성능 회귀 테스트 결과")
    print("=" * 60)

    all_passed = True

    for name, data in results.items():
        if 'error' in data:
            status = "실패"
            detail = data['error']
            all_passed = False
        elif data['passed']:
            status = "통과"
            detail = f"{data['avg']:.3f}s (임계: {data['threshold']}s)"
        else:
            status = "회귀"
            detail = f"{data['avg']:.3f}s > {data['threshold']}s"
            all_passed = False

        print(f"  {name}: {status}")
        print(f"    {detail}")

    print("=" * 60)

    return all_passed


def save_results(results, output_path):
    """결과를 JSON 파일로 저장"""
    with open(output_path, 'w', encoding='utf-8') as f:
        json.dump(results, f, indent=2, ensure_ascii=False)
    print(f"\n결과 저장됨: {output_path}")


def main():
    print("=" * 60)
    print("KingSejong 성능 회귀 테스트")
    print("=" * 60)
    print()

    interpreter = get_interpreter_path()
    benchmarks_dir = Path(__file__).parent

    print(f"인터프리터: {interpreter}")
    print(f"벤치마크 디렉토리: {benchmarks_dir}")
    print()

    # 벤치마크 실행
    results = run_benchmark_suite(interpreter, benchmarks_dir)

    # 결과 출력
    all_passed = print_results(results)

    # 결과 저장
    output_path = benchmarks_dir / 'regression_results.json'
    save_results(results, output_path)

    # 종료 코드
    if all_passed:
        print("\n모든 성능 테스트 통과!")
        return 0
    else:
        print("\n성능 회귀 감지됨!")
        return 1


if __name__ == '__main__':
    sys.exit(main())
