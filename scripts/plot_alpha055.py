#!/usr/bin/env python3
"""Regenerate alpha=0.55 ordinary-random-churn graphs and summaries from raw logs.

Does not run any benchmark and never edits input measurements.
Five complete trials per n are required by default.
"""
import argparse
import csv
import math
from collections import defaultdict
from pathlib import Path
from statistics import mean, stdev

# Matplotlib is already used by scripts/plot_results.py in the team repository.
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

SIZES = {1000: 1_000_000, 2000: 4_000_000, 4000: 16_000_000}
STRATEGIES = ["successor", "random_policy", "scapegoat_array", "scapegoat_dsw"]
LABELS = {
    "successor": "Hibbard successor",
    "random_policy": "Hibbard randomized",
    "scapegoat_array": "Scapegoat array (alpha=0.55)",
    "scapegoat_dsw": "Scapegoat DSW (alpha=0.55)",
}
COLORS = {
    "successor": "#c45132",
    "random_policy": "#2376ad",
    "scapegoat_array": "#27826a",
    "scapegoat_dsw": "#8354a1",
}
EXPECTED_COLUMNS = [
    "workload", "n", "trial", "seed", "strategy", "pairs", "trace_hash",
    "average_depth", "height", "ns_per_operation", "local_rebuilds",
    "whole_tree_rebuilds", "nodes_rebuilt", "dsw_rotations",
]


def load_trials(folder: Path):
    """Verify complete data and store depth measurements per trial."""
    datasets = defaultdict(dict)
    for size, total_pairs in SIZES.items():
        for trial in range(1, 6):
            filename = folder / f"random-n{size}-trial{trial}.csv"
            if not filename.is_file():
                raise ValueError(f"Missing full trial: {filename}")
            with filename.open(newline="", encoding="utf-8") as stream:
                reader = csv.DictReader(stream)
                if reader.fieldnames != EXPECTED_COLUMNS:
                    raise ValueError(f"Incorrect CSV format: {filename}")
                rows = list(reader)
            if len(rows) != 400:
                raise ValueError(f"Incomplete trial {filename}: expected 400 data rows, found {len(rows)}")
            by_strategy = defaultdict(list)
            for row in rows:
                if row["workload"] != "random" or int(row["n"]) != size or int(row["trial"]) != trial:
                    raise ValueError(f"Unexpected workload, population or trial in {filename}")
                by_strategy[row["strategy"]].append(row)
            if set(by_strategy) != set(STRATEGIES):
                raise ValueError(f"Missing tree strategy in {filename}")
            hashes = set()
            seeds = set()
            for strategy in STRATEGIES:
                series = by_strategy[strategy]
                if len(series) != 100:
                    raise ValueError(f"Expected 100 checkpoints for {strategy} in {filename}")
                expected_positions = [total_pairs * step // 100 for step in range(1, 101)]
                positions = [int(row["pairs"]) for row in series]
                if positions != expected_positions:
                    raise ValueError(f"Incorrect checkpoints for {strategy} in {filename}")
                hashes.update(row["trace_hash"] for row in series)
                seeds.update(row["seed"] for row in series)
                datasets[size, strategy][trial] = [float(row["average_depth"]) for row in series]
            if len(hashes) != 1 or len(seeds) != 1:
                raise ValueError(f"Strategies do not share input trace/seed in {filename}")
            if list(seeds)[0] != str(20261117 + trial):
                raise ValueError(f"Unexpected seed for {filename}")
    return datasets


def compute_summaries(datasets):
    checkpoint_rows = []
    final_rows = []
    for size, total_pairs in SIZES.items():
        for strategy in STRATEGIES:
            trials = datasets[size, strategy]
            for checkpoint_index in range(100):
                samples = [trials[trial][checkpoint_index] for trial in range(1, 6)]
                checkpoint_rows.append({
                    "n": size,
                    "strategy": strategy,
                    "pairs": (checkpoint_index + 1) * total_pairs // 100,
                    "mean_depth": mean(samples),
                    "sd_depth": stdev(samples),
                    "trials": 5,
                })
            final_samples = [trials[trial][-1] for trial in range(1, 6)]
            final_rows.append({
                "n": size,
                "strategy": strategy,
                "trials": 5,
                "mean_final_depth": mean(final_samples),
                "sample_sd": stdev(final_samples),
            })
    return checkpoint_rows, final_rows


def save_csv(path, rows):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0].keys()))
        writer.writeheader()
        writer.writerows(rows)


def draw_size(axis, rows, size):
    by_strategy = defaultdict(list)
    for row in rows:
        if row["n"] == size:
            by_strategy[row["strategy"]].append(row)
    for strategy in STRATEGIES:
        values = by_strategy[strategy]
        x = [row["pairs"] for row in values]
        y = [row["mean_depth"] for row in values]
        lower = [row["mean_depth"] - row["sd_depth"] for row in values]
        upper = [row["mean_depth"] + row["sd_depth"] for row in values]
        axis.plot(x, y, color=COLORS[strategy], label=LABELS[strategy], linewidth=1.6)
        axis.fill_between(x, lower, upper, color=COLORS[strategy], alpha=0.17)
    axis.axhline(2 * math.log(size), color="#555555", linestyle="--", linewidth=1, label="2 ln(n)")
    axis.axhline(math.sqrt(size), color="#999999", linestyle=":", linewidth=1, label="sqrt(n)")
    axis.set_title(f"n = {size:,}")
    axis.set_xlabel("Completed delete-insert pairs")
    axis.set_ylabel("Average node depth (edges)")
    axis.grid(alpha=0.17)
    axis.legend(fontsize=7, loc="center right")


def draw_figures(checkpoints, destination):
    destination.mkdir(parents=True, exist_ok=True)
    fig, axes = plt.subplots(1, 3, figsize=(18, 5), constrained_layout=True)
    for axis, size in zip(axes, SIZES):
        draw_size(axis, checkpoints, size)
    fig.suptitle("Ordinary random churn, Scapegoat alpha = 0.55: five-trial mean +/- sample SD")
    fig.savefig(destination / "ordinary-random-alpha055-three-populations.png", dpi=180, bbox_inches="tight")
    fig.savefig(destination / "ordinary-random-alpha055-three-populations.svg", bbox_inches="tight")
    plt.close(fig)
    fig, axis = plt.subplots(figsize=(8.5, 5), constrained_layout=True)
    draw_size(axis, checkpoints, 1000)
    axis.set_title("Ordinary random churn: n = 1,000 (five trials)")
    fig.savefig(destination / "ordinary-random-alpha055-n1000.png", dpi=180, bbox_inches="tight")
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, default=Path("results/raw/analyst/alpha055-random"))
    parser.add_argument("--summary", type=Path, default=Path("results/summary/analyst/alpha055-random"))
    parser.add_argument("--figures", type=Path, default=Path("results/figures/analyst/alpha055-random"))
    args = parser.parse_args()
    datasets = load_trials(args.input)
    checkpoints, finals = compute_summaries(datasets)
    save_csv(args.summary / "checkpoint-depth-summary.csv", checkpoints)
    save_csv(args.summary / "final-depth-summary.csv", finals)
    draw_figures(checkpoints, args.figures)
    print("Verified 15 complete trials (5 per population), 4 tree strategies per trial")
    print("Wrote 1,200 checkpoint summaries, 12 final-depth summaries, and three graph files")


if __name__ == "__main__":
    main()
