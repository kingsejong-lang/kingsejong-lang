#!/usr/bin/env python3
"""메모리 사용량 측정 스크립트"""

import subprocess
import sys
import os
import re

def measure_memory(interpreter_path, script_path):
    """스크립트 실행 시 최대 메모리 사용량 측정 (macOS)"""

    # /usr/bin/time -l 사용하여 메모리 측정
    cmd = ['/usr/bin/time', '-l', interpreter_path, script_path]

    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=60
        )

        # stderr에서 메모리 정보 추출
        stderr = result.stderr

        # maximum resident set size 찾기
        match = re.search(r'(\d+)\s+maximum resident set size', stderr)
        if match:
            max_rss_bytes = int(match.group(1))
            max_rss_mb = max_rss_bytes / (1024 * 1024)
            return max_rss_mb

        # 다른 형식 시도
        match = re.search(r'peak memory footprint:\s+([\d.]+)([KMG])', stderr)
        if match:
            value = float(match.group(1))
            unit = match.group(2)
            if unit == 'K':
                return value / 1024
            elif unit == 'M':
                return value
            elif unit == 'G':
                return value * 1024

        print(f"메모리 정보를 파싱할 수 없음:\n{stderr}")
        return None

    except subprocess.TimeoutExpired:
        print("타임아웃")
        return None
    except Exception as e:
        print(f"오류: {e}")
        return None

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)

    interpreter = os.path.join(project_root, 'build', 'bin', 'kingsejong')

    if not os.path.exists(interpreter):
        print(f"인터프리터를 찾을 수 없음: {interpreter}")
        sys.exit(1)

    # 벤치마크 파일들
    benchmarks = [
        ('memory_test.ksj', '메모리 테스트'),
        ('arithmetic.ksj', '산술 연산'),
        ('loop_intensive.ksj', '반복문 집약'),
    ]

    print("=" * 50)
    print("KingSejong 메모리 사용량 측정")
    print("=" * 50)
    print()

    results = []

    for filename, description in benchmarks:
        filepath = os.path.join(script_dir, filename)
        if not os.path.exists(filepath):
            print(f"파일 없음: {filename}")
            continue

        print(f"측정 중: {description} ({filename})")
        memory_mb = measure_memory(interpreter, filepath)

        if memory_mb is not None:
            print(f"  최대 메모리: {memory_mb:.2f} MB")
            results.append((description, memory_mb))
        print()

    if results:
        print("=" * 50)
        print("요약")
        print("=" * 50)
        for desc, mem in results:
            print(f"  {desc}: {mem:.2f} MB")

if __name__ == '__main__':
    main()
