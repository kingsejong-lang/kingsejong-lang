#!/usr/bin/env python3
"""
KingSejong 언어 벤치마크 실행 스크립트

Usage:
    python3 run_benchmarks.py [--interpreter PATH] [--runs N]
"""

import subprocess
import time
import sys
import os
import argparse
from pathlib import Path

class BenchmarkRunner:
    def __init__(self, interpreter_path, num_runs=5):
        self.interpreter = interpreter_path
        self.num_runs = num_runs
        self.results = {}

    def run_benchmark(self, benchmark_file):
        """단일 벤치마크 실행"""
        times = []

        for i in range(self.num_runs):
            start = time.time()
            try:
                result = subprocess.run(
                    [self.interpreter, benchmark_file],
                    capture_output=True,
                    text=True,
                    timeout=30
                )
                end = time.time()

                if result.returncode != 0:
                    print(f"  ❌ 실행 실패: {benchmark_file}")
                    print(f"     오류: {result.stderr}")
                    return None

                elapsed = end - start
                times.append(elapsed)
                print(f"  Run {i+1}/{self.num_runs}: {elapsed:.4f}s")

            except subprocess.TimeoutExpired:
                print(f"  ⏱️ 타임아웃: {benchmark_file}")
                return None
            except Exception as e:
                print(f"  ❌ 예외 발생: {e}")
                return None

        return times

    def calculate_stats(self, times):
        """통계 계산"""
        if not times:
            return None

        avg = sum(times) / len(times)
        min_time = min(times)
        max_time = max(times)

        # 표준편차 계산
        variance = sum((t - avg) ** 2 for t in times) / len(times)
        std_dev = variance ** 0.5

        return {
            'avg': avg,
            'min': min_time,
            'max': max_time,
            'std_dev': std_dev,
            'times': times
        }

    def run_all_benchmarks(self):
        """모든 벤치마크 실행"""
        benchmark_dir = Path(__file__).parent
        benchmark_files = sorted(benchmark_dir.glob('*.ksj'))

        print(f"🚀 KingSejong 벤치마크 실행")
        print(f"   인터프리터: {self.interpreter}")
        print(f"   실행 횟수: {self.num_runs}")
        print(f"   벤치마크 개수: {len(benchmark_files)}")
        print()

        for bench_file in benchmark_files:
            bench_name = bench_file.stem
            print(f"📊 {bench_name}:")

            times = self.run_benchmark(str(bench_file))

            if times:
                stats = self.calculate_stats(times)
                self.results[bench_name] = stats

                print(f"  ✅ 평균: {stats['avg']:.4f}s")
                print(f"     최소: {stats['min']:.4f}s")
                print(f"     최대: {stats['max']:.4f}s")
                print(f"     표준편차: {stats['std_dev']:.4f}s")
            else:
                self.results[bench_name] = None
                print(f"  ❌ 실패")

            print()

    def print_summary(self):
        """결과 요약 출력"""
        print("=" * 60)
        print("📈 벤치마크 결과 요약")
        print("=" * 60)
        print()

        print(f"{'벤치마크':<20} {'평균 (s)':<12} {'최소 (s)':<12} {'최대 (s)':<12}")
        print("-" * 60)

        total_time = 0
        success_count = 0

        for bench_name, stats in self.results.items():
            if stats:
                print(f"{bench_name:<20} {stats['avg']:<12.4f} {stats['min']:<12.4f} {stats['max']:<12.4f}")
                total_time += stats['avg']
                success_count += 1
            else:
                print(f"{bench_name:<20} {'FAILED':<12} {'-':<12} {'-':<12}")

        print("-" * 60)
        print(f"{'총 실행 시간':<20} {total_time:<12.4f}")
        print(f"{'성공한 벤치마크':<20} {success_count}/{len(self.results)}")
        print()

    def save_results(self, output_file='benchmark_results.txt'):
        """결과를 파일로 저장"""
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write("KingSejong 벤치마크 결과\n")
            f.write("=" * 60 + "\n")
            f.write(f"인터프리터: {self.interpreter}\n")
            f.write(f"실행 횟수: {self.num_runs}\n")
            f.write("\n")

            for bench_name, stats in self.results.items():
                f.write(f"{bench_name}:\n")
                if stats:
                    f.write(f"  평균: {stats['avg']:.4f}s\n")
                    f.write(f"  최소: {stats['min']:.4f}s\n")
                    f.write(f"  최대: {stats['max']:.4f}s\n")
                    f.write(f"  표준편차: {stats['std_dev']:.4f}s\n")
                    f.write(f"  실행 시간: {stats['times']}\n")
                else:
                    f.write(f"  실패\n")
                f.write("\n")

        print(f"✅ 결과 저장: {output_file}")

def main():
    parser = argparse.ArgumentParser(description='KingSejong 벤치마크 실행')
    parser.add_argument('--interpreter', type=str,
                       default='../build/kingsejong',
                       help='인터프리터 경로 (기본: ../build/kingsejong)')
    parser.add_argument('--runs', type=int, default=5,
                       help='각 벤치마크 실행 횟수 (기본: 5)')
    parser.add_argument('--output', type=str, default='benchmark_results.txt',
                       help='결과 파일 경로 (기본: benchmark_results.txt)')

    args = parser.parse_args()

    # 인터프리터 존재 확인
    if not os.path.exists(args.interpreter):
        print(f"❌ 인터프리터를 찾을 수 없습니다: {args.interpreter}")
        print(f"   빌드를 먼저 수행하세요: cmake --build build")
        sys.exit(1)

    runner = BenchmarkRunner(args.interpreter, args.runs)
    runner.run_all_benchmarks()
    runner.print_summary()
    runner.save_results(args.output)

if __name__ == '__main__':
    main()
