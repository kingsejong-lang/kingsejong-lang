#!/usr/bin/env python3
"""examples/*.ksj 를 실제 인터프리터로 실행해 기대 출력과 비교한다.

문서와 예제가 코드보다 앞서 "완료"를 말하는 일을 막는 가장 싼 장치다. 예제마다
examples/expected/<이름>.out 이 있어야 하고, 출력이 다르면 실패한다. 네트워크·시간·
난수처럼 결정적이지 않은 예제는 examples/expected/SKIP 에 "<파일명> <이유>" 로 적으면
종료 코드만 확인한다.

    python3 tests/examples/run_examples.py --interpreter build/bin/kingsejong
    python3 tests/examples/run_examples.py --interpreter build/bin/kingsejong --update   # 기대 출력 다시 생성

CMake 가 ctest 항목(examples)으로 등록하므로 CI 의 ctest 가 함께 돌린다.
"""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


def load_list(path: Path) -> dict[str, str]:
    """"<파일명> <이유>" 줄로 된 목록 파일을 읽는다. 없으면 빈 dict."""
    entries: dict[str, str] = {}
    if path.is_file():
        for raw in path.read_text(encoding="utf-8").splitlines():
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            name, _, reason = line.partition(" ")
            entries[name] = reason.strip() or "(이유 없음)"
    return entries


def run_one(interpreter: Path, flags: list[str], example: Path, timeout: float) -> tuple[int, str, str]:
    proc = subprocess.run(
        [str(interpreter), *flags, str(example)],
        capture_output=True, text=True, timeout=timeout, encoding="utf-8", errors="replace",
    )
    return proc.returncode, proc.stdout, proc.stderr


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--interpreter", required=True, type=Path)
    ap.add_argument("--examples-dir", type=Path, default=Path("examples"))
    ap.add_argument("--expected-dir", type=Path, default=Path("examples/expected"))
    ap.add_argument("--update", action="store_true", help="기대 출력을 현재 출력으로 다시 쓴다")
    ap.add_argument("--timeout", type=float, default=60.0)
    ap.add_argument("--interpreter-flag", action="append", default=[],
                    help="인터프리터에 넘길 플래그 (예: --interpreter-flag=--vm)")
    ap.add_argument("--known-failures", type=Path, default=None,
                    help="기대 실패 목록 파일 (기본 <expected-dir>/KNOWN_FAILURES)")
    args = ap.parse_args()

    if not args.interpreter.is_file():
        print(f"인터프리터가 없습니다: {args.interpreter}", file=sys.stderr)
        return 2

    skip = load_list(args.expected_dir / "SKIP")
    # 아직 언어가 지원하지 않는 구문을 쓰는 예제. 실패해야 정상이고, 통과하기 시작하면 목록에서 빼라고 알린다.
    known_failures = load_list(args.known_failures or (args.expected_dir / "KNOWN_FAILURES"))
    args.expected_dir.mkdir(parents=True, exist_ok=True)
    examples = sorted(args.examples_dir.glob("*.ksj"))
    if not examples:
        print(f"예제가 없습니다: {args.examples_dir}", file=sys.stderr)
        return 2

    failures: list[str] = []
    passed = 0
    for example in examples:
        name = example.name
        expected_path = args.expected_dir / (example.stem + ".out")
        try:
            code, out, err = run_one(args.interpreter, args.interpreter_flag, example, args.timeout)
        except subprocess.TimeoutExpired:
            failures.append(f"{name}: 시간 초과 ({args.timeout}s)")
            continue

        if name in known_failures:
            expected = expected_path.read_text(encoding="utf-8") if expected_path.is_file() else None
            matches = code == 0 and (expected is None or out.rstrip() == expected.rstrip())
            if not matches:
                print(f"XFAIL {name} ({known_failures[name]})")
                passed += 1
            else:
                failures.append(f"{name}: 기대 실패 목록에 있는데 통과했습니다. 목록에서 빼세요")
            continue

        if code != 0:
            failures.append(f"{name}: 종료 코드 {code}\n{err.strip()[-600:]}")
            continue

        if name in skip:
            print(f"SKIP {name} ({skip[name]}; 종료 코드만 확인)")
            passed += 1
            continue

        if args.update:
            expected_path.write_text(out, encoding="utf-8")
            print(f"UPDATE {name}")
            passed += 1
            continue

        if not expected_path.is_file():
            failures.append(f"{name}: 기대 출력 파일이 없습니다 ({expected_path}). --update 로 만들거나 SKIP 에 적으세요")
            continue

        expected = expected_path.read_text(encoding="utf-8")
        if out.rstrip() != expected.rstrip():
            diff = []
            for i, (a, b) in enumerate(zip(expected.splitlines(), out.splitlines()), 1):
                if a != b:
                    diff.append(f"  줄 {i}:\n    기대: {a}\n    실제: {b}")
                    if len(diff) >= 3:
                        break
            if len(expected.splitlines()) != len(out.splitlines()):
                diff.append(f"  줄 수: 기대 {len(expected.splitlines())}, 실제 {len(out.splitlines())}")
            failures.append(f"{name}: 출력이 다릅니다\n" + "\n".join(diff))
            continue

        passed += 1
        print(f"PASS {name}")

    print(f"\n예제 {len(examples)}개: 통과 {passed}, 실패 {len(failures)}")
    for failure in failures:
        print(f"FAIL {failure}")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
