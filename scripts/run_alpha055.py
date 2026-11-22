#!/usr/bin/env python3
"""Re-run fixed-population random churn with both Scapegoat rebuild methods at alpha=0.55.

Defaults to full n^2 pairs and five trials per size. Output goes to a fresh folder;
the existing checked-in measurements are never overwritten.
"""
import argparse
import subprocess
from pathlib import Path
import os

SIZES = [1000, 2000, 4000]


def benchmark_path():
    name = "churn_benchmark.exe" if os.name == "nt" else "churn_benchmark"
    return Path(".build/bin") / name


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--benchmark", type=Path, default=benchmark_path())
    parser.add_argument("--output", type=Path, default=Path(".build/alpha055-local-replay"))
    parser.add_argument("--quick", action="store_true", help="One quick 100-node demonstration, not report evidence")
    args = parser.parse_args()
    if not args.benchmark.is_file():
        raise FileNotFoundError(f"Cannot find benchmark executable: {args.benchmark}; run python scripts/run_all.py --build-only")
    if args.output.exists():
        raise FileExistsError(f"Choose a fresh output directory: {args.output}")
    args.output.mkdir(parents=True)

    experiments = [(100, 10_000, 1)] if args.quick else [(n, n * n, trial) for n in SIZES for trial in range(1, 6)]
    for n, pairs, trial in experiments:
        seed = 20261117 + trial
        checkpoint = max(1, pairs // 100)
        output = args.output / f"random-n{n}-trial{trial}.csv"
        command = [
            str(args.benchmark.resolve()), str(n), str(pairs), str(seed), str(trial),
            "random", str(checkpoint), "all", "0.55",
        ]
        print(f"Running n={n}, pairs={pairs}, trial={trial}, seed={seed}, alpha=0.55", flush=True)
        with output.open("w", encoding="utf-8", newline="") as stream:
            subprocess.run(command, stdout=stream, check=True)
        print(f"Saved {output}", flush=True)
    if args.quick:
        print(f"Quick run complete. CSV saved in {args.output}")
    else:
        print(f'Verify the full study with: python scripts/plot_alpha055.py --input "{args.output}"')


if __name__ == "__main__":
    main()
