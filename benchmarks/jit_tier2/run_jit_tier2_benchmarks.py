#!/usr/bin/env python3
"""
JIT Tier 2 벤치마크 실행 및 성능 분석 스크립트

Usage:
    python3 run_jit_tier2_benchmarks.py [--interpreter PATH] [--runs N]
"""

import subprocess
import time
import sys
import os
import argparse
import re
from pathlib import Path

class JITTier2BenchmarkRunner:
    def __init__(self, interpreter_path, num_runs=5):
        self.interpreter = interpreter_path
        self.num_runs = num_runs
        self.results = {}

    def run_benchmark(self, benchmark_file):
        """단일 벤치마크 실행 및 JIT 통계 수집"""
        times = []
        jit_stats = None

        for i in range(self.num_runs):
            start = time.time()
            try:
                # --jit-stats 플래그로 JIT 통계 출력
                result = subprocess.run(
                    [self.interpreter, "--jit-stats", benchmark_file],
                    capture_output=True,
                    text=True,
                    timeout=30
                )
                end = time.time()

                if result.returncode != 0:
                    print(f"  ❌ 실행 실패: {benchmark_file}")
                    print(f"     오류: {result.stderr}")
                    return None, None

                elapsed = end - start
                times.append(elapsed)
                print(f"  Run {i+1}/{self.num_runs}: {elapsed:.4f}s")

                # 첫 번째 실행에서 JIT 통계 수집
                if i == 0:
                    jit_stats = self.parse_jit_stats(result.stderr)

            except subprocess.TimeoutExpired:
                print(f"  ⏱️ 타임아웃: {benchmark_file}")
                return None, None
            except Exception as e:
                print(f"  ❌ 예외 발생: {e}")
                return None, None

        return times, jit_stats

    def parse_jit_stats(self, output):
        """JIT 통계 파싱"""
        stats = {
            'tier1_compilations': 0,
            'tier2_compilations': 0,
            'tier1_executions': 0,
            'tier2_executions': 0,
            'total_inlined_functions': 0
        }

        # Tier 1 통계
        match = re.search(r'Total Compilations:\s+(\d+)', output)
        if match:
            stats['tier1_compilations'] = int(match.group(1))

        match = re.search(r'Total Executions:\s+(\d+)', output)
        if match:
            stats['tier1_executions'] = int(match.group(1))

        # Tier 2 통계
        match = re.search(r'Tier 2 Compilations:\s+(\d+)', output)
        if match:
            stats['tier2_compilations'] = int(match.group(1))

        match = re.search(r'Tier 2 Executions:\s+(\d+)', output)
        if match:
            stats['tier2_executions'] = int(match.group(1))

        match = re.search(r'Total Inlined Functions:\s+(\d+)', output)
        if match:
            stats['total_inlined_functions'] = int(match.group(1))

        return stats

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
        """모든 JIT Tier 2 벤치마크 실행"""
        benchmark_dir = Path(__file__).parent
        benchmark_files = sorted(benchmark_dir.glob('*.ksj'))

        print(f"🚀 JIT Tier 2 벤치마크 실행")
        print(f"   인터프리터: {self.interpreter}")
        print(f"   실행 횟수: {self.num_runs}")
        print(f"   벤치마크 개수: {len(benchmark_files)}")
        print()

        for bench_file in benchmark_files:
            bench_name = bench_file.stem
            print(f"📊 {bench_name}:")

            times, jit_stats = self.run_benchmark(str(bench_file))

            if times:
                stats = self.calculate_stats(times)
                self.results[bench_name] = {
                    'performance': stats,
                    'jit': jit_stats
                }

                print(f"  ✅ 평균: {stats['avg']:.4f}s")
                print(f"     최소: {stats['min']:.4f}s")
                print(f"     최대: {stats['max']:.4f}s")
                print(f"     표준편차: {stats['std_dev']:.4f}s")

                if jit_stats:
                    print(f"  🔥 JIT 통계:")
                    print(f"     Tier 1 컴파일: {jit_stats['tier1_compilations']}회")
                    print(f"     Tier 2 컴파일: {jit_stats['tier2_compilations']}회")
                    print(f"     인라인된 함수: {jit_stats['total_inlined_functions']}개")
            else:
                self.results[bench_name] = None
                print(f"  ❌ 실패")

            print()

    def print_summary(self):
        """결과 요약 출력"""
        print("=" * 80)
        print("📈 JIT Tier 2 벤치마크 결과 요약")
        print("=" * 80)
        print()

        print(f"{'벤치마크':<30} {'평균 (s)':<12} {'T1 컴파일':<12} {'T2 컴파일':<12} {'인라인':<8}")
        print("-" * 80)

        total_time = 0
        success_count = 0
        total_tier1 = 0
        total_tier2 = 0
        total_inlined = 0

        for bench_name, result in self.results.items():
            if result:
                perf = result['performance']
                jit = result['jit']

                t1_comp = jit['tier1_compilations'] if jit else 0
                t2_comp = jit['tier2_compilations'] if jit else 0
                inlined = jit['total_inlined_functions'] if jit else 0

                print(f"{bench_name:<30} {perf['avg']:<12.4f} {t1_comp:<12} {t2_comp:<12} {inlined:<8}")

                total_time += perf['avg']
                total_tier1 += t1_comp
                total_tier2 += t2_comp
                total_inlined += inlined
                success_count += 1
            else:
                print(f"{bench_name:<30} {'FAILED':<12} {'-':<12} {'-':<12} {'-':<8}")

        print("-" * 80)
        print(f"{'총계':<30} {total_time:<12.4f} {total_tier1:<12} {total_tier2:<12} {total_inlined:<8}")
        print(f"{'성공한 벤치마크':<30} {success_count}/{len(self.results)}")
        print()

        # 성능 개선 분석
        print("=" * 80)
        print("🎯 성능 분석")
        print("=" * 80)
        print()
        print(f"✓ Tier 2 컴파일된 함수: {total_tier2}개")
        print(f"✓ 인라인된 함수 호출: {total_inlined}개")
        print()
        print("예상 성능 개선:")
        print("  - 작은 함수 집약: ~30% 개선 (함수 호출 오버헤드 제거)")
        print("  - 조건문 함수: ~25% 개선 (분기 예측 최적화)")
        print("  - stdlib 함수: ~20% 개선 (반복적 호출 패턴 최적화)")
        print()

    def save_results(self, output_file='jit_tier2_results.txt'):
        """결과를 파일로 저장"""
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write("JIT Tier 2 벤치마크 결과\n")
            f.write("=" * 80 + "\n")
            f.write(f"인터프리터: {self.interpreter}\n")
            f.write(f"실행 횟수: {self.num_runs}\n")
            f.write("\n")

            for bench_name, result in self.results.items():
                f.write(f"{bench_name}:\n")
                if result:
                    perf = result['performance']
                    jit = result['jit']

                    f.write(f"  성능:\n")
                    f.write(f"    평균: {perf['avg']:.4f}s\n")
                    f.write(f"    최소: {perf['min']:.4f}s\n")
                    f.write(f"    최대: {perf['max']:.4f}s\n")
                    f.write(f"    표준편차: {perf['std_dev']:.4f}s\n")

                    if jit:
                        f.write(f"  JIT 통계:\n")
                        f.write(f"    Tier 1 컴파일: {jit['tier1_compilations']}회\n")
                        f.write(f"    Tier 2 컴파일: {jit['tier2_compilations']}회\n")
                        f.write(f"    Tier 1 실행: {jit['tier1_executions']}회\n")
                        f.write(f"    Tier 2 실행: {jit['tier2_executions']}회\n")
                        f.write(f"    인라인된 함수: {jit['total_inlined_functions']}개\n")
                else:
                    f.write(f"  실패\n")
                f.write("\n")

        print(f"✅ 결과 저장: {output_file}")

def main():
    parser = argparse.ArgumentParser(description='JIT Tier 2 벤치마크 실행')
    parser.add_argument('--interpreter', type=str,
                       default='../../build/kingsejong',
                       help='인터프리터 경로 (기본: ../../build/kingsejong)')
    parser.add_argument('--runs', type=int, default=5,
                       help='각 벤치마크 실행 횟수 (기본: 5)')
    parser.add_argument('--output', type=str, default='jit_tier2_results.txt',
                       help='결과 파일 경로 (기본: jit_tier2_results.txt)')

    args = parser.parse_args()

    # 인터프리터 존재 확인
    if not os.path.exists(args.interpreter):
        print(f"❌ 인터프리터를 찾을 수 없습니다: {args.interpreter}")
        print(f"   빌드를 먼저 수행하세요: cmake --build build")
        sys.exit(1)

    runner = JITTier2BenchmarkRunner(args.interpreter, args.runs)
    runner.run_all_benchmarks()
    runner.print_summary()
    runner.save_results(args.output)

if __name__ == '__main__':
    main()
