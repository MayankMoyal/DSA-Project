"""Regenerate all recorded graphs from CSVs, without rerunning measurements."""
import argparse
import csv
import math
import os
from pathlib import Path
import statistics

ROOT = Path(__file__).resolve().parents[1]
os.environ.setdefault("MPLCONFIGDIR", str(ROOT / ".build" / "matplotlib"))
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

STRATEGIES = ["successor", "random_policy", "scapegoat_array", "scapegoat_dsw"]
LABELS = ["Hibbard successor", "Hibbard randomized", "Scapegoat array", "Scapegoat DSW"]
COLORS = ["#c45132", "#2376ad", "#27826a", "#8354a1"]
SIZES = [1000, 2000, 4000]


def read(path):
    with path.open(newline="", encoding="utf-8-sig") as stream:
        return list(csv.DictReader(stream))


def stats(values):
    # Each group must contain five independent trials.
    if len(values) != 5:
        raise ValueError(f"Expected five trials, got {len(values)}")
    average = statistics.mean(values)
    spread = statistics.stdev(values)
    return average, spread


def save(fig, path):
    path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(path, dpi=150, bbox_inches="tight")
    if path.suffix == ".svg":
        # Clean trailing whitespace as in the original script.
        lines = path.read_text(encoding="utf-8").splitlines()
        cleaned = []
        for line in lines:
            cleaned.append(line.rstrip())
        path.write_text("\n".join(cleaned) + "\n", encoding="utf-8")
    plt.close(fig)


def add_value(groups, key, value):
    # Ordinary dictionary: create a list when we see a group for the first time.
    if key not in groups:
        groups[key] = []
    groups[key].append(value)


def add_depth(groups, row):
    n = int(row["n"])
    strategy = row["strategy"]
    pairs = int(row["pairs"])
    depth = float(row["average_depth"])
    if n not in groups:
        groups[n] = {}
    add_value(groups[n], (strategy, pairs), depth)


def curves(ax, grouped, n):
    # For each strategy, plot the mean and mean +/- one sample SD.
    for index in range(len(STRATEGIES)):
        strategy = STRATEGIES[index]
        points = []
        for name, pairs in grouped:
            if name == strategy:
                points.append(pairs)
        points.sort()
        if not points:
            raise ValueError(f"Missing strategy {strategy}")
        means = []
        lower = []
        upper = []
        for pairs in points:
            average, spread = stats(grouped[(strategy, pairs)])
            means.append(average)
            lower.append(average - spread)
            upper.append(average + spread)
        ax.plot(points, means, label=LABELS[index], color=COLORS[index])
        ax.fill_between(points, lower, upper, color=COLORS[index], alpha=0.15)
    ax.axhline(2 * math.log(n), color="#555555", linestyle="--", label="2 ln(n) reference")
    ax.axhline(math.sqrt(n), color="#999999", linestyle=":", label="sqrt(n) reference")
    ax.set_xlabel("Completed delete-insert pairs")
    ax.set_ylabel("Average node depth (edges)")
    ax.set_title(f"n = {n:,}")
    ax.grid(alpha=0.2)


def analyst(raw, destination, svg=False):
    groups = {}
    for row in read(raw):
        add_depth(groups, row)
    for n in sorted(groups):
        fig, ax = plt.subplots(figsize=(8, 4.7))
        curves(ax, groups[n], n)
        ax.legend(fontsize=8, ncol=2)
        fig.suptitle("Ordinary random churn: five-trial mean and sample SD")
        fig.tight_layout()
        if svg:
            name = f"depth-{n}.svg"
        else:
            name = f"random-{n}-mean-depth.png"
        save(fig, destination / name)


def read_workload(raw, workload):
    groups = {}
    finals = {}
    paths = sorted(raw.glob(f"{workload}-*.csv"))
    if len(paths) != 15:
        raise ValueError(f"{workload}: expected 15 complete files")
    for path in paths:
        rows = read(path)
        last = {}
        for row in rows:
            add_depth(groups, row)
            strategy = row["strategy"]
            # Replacing last[strategy] keeps only its last checkpoint in this trial.
            last[strategy] = row
        for strategy, row in last.items():
            n = int(row["n"])
            if int(row["pairs"]) != n * n:
                raise ValueError(f"Incomplete run: {path}")
            add_value(finals, (n, strategy), float(row["average_depth"]))
    return groups, finals


def workload_depth_graph(groups, workload, destination):
    fig, axes = plt.subplots(1, 3, figsize=(15, 4.6))
    for i in range(len(SIZES)):
        n = SIZES[i]
        curves(axes[i], groups[n], n)
    axes[0].legend(fontsize=7)
    fig.suptitle(f"{workload}: recorded reference implementation, mean and sample SD")
    fig.tight_layout()
    save(fig, destination / f"{workload}-depth.png")


def workload_scaling_graph(finals, workload, destination):
    fig, ax = plt.subplots(figsize=(8, 4.7))
    for i in range(len(STRATEGIES)):
        strategy = STRATEGIES[i]
        means = []
        spreads = []
        for n in SIZES:
            average, spread = stats(finals[(n, strategy)])
            means.append(average)
            spreads.append(spread)
        ax.errorbar(SIZES, means, yerr=spreads, marker="o", capsize=4,
                    label=LABELS[i], color=COLORS[i])
    log_reference = []
    sqrt_reference = []
    for n in SIZES:
        log_reference.append(2 * math.log(n))
        sqrt_reference.append(math.sqrt(n))
    ax.plot(SIZES, log_reference, "--", color="#555555", label="2 ln(n) reference")
    ax.plot(SIZES, sqrt_reference, ":", color="#999999", label="sqrt(n) reference")
    ax.set_xlabel("Live keys n")
    ax.set_ylabel("Final average depth (edges)")
    ax.set_title(f"{workload}: reference, after n² pairs")
    ax.legend(fontsize=8)
    ax.grid(alpha=0.2)
    fig.tight_layout()
    save(fig, destination / f"{workload}-scaling.png")


def insertion_graph(raw, implementation, destination):
    values = {}
    for path in sorted(raw.glob(f"insertions-{implementation}-*.csv")):
        for row in read(path):
            n = int(row["n"])
            key = (n, row["pattern"], row["mode"])
            rate = int(row["rebuilds"]) / n
            add_value(values, key, rate)
    fig, axes = plt.subplots(1, 3, figsize=(15, 4.8))
    patterns = ["ascending", "descending", "extremes", "blocks", "shuffle"]
    modes = ["array", "dsw"]
    for i in range(len(SIZES)):
        n = SIZES[i]
        ax = axes[i]
        for j in range(len(modes)):
            mode = modes[j]
            means = []
            spreads = []
            positions = []
            for p in range(len(patterns)):
                pattern = patterns[p]
                average, spread = stats(values[(n, pattern, mode)])
                means.append(average)
                spreads.append(spread)
                positions.append(p + (j - 0.5) * 0.36)
            ax.bar(positions, means, yerr=spreads, width=0.36, capsize=3, label=mode)
        ax.set_xticks(range(len(patterns)), patterns, rotation=25)
        ax.set_title(f"n = {n:,}")
        ax.set_ylabel("Local rebuild events / insertion")
        ax.legend()
    fig.suptitle(f"{implementation}: insertion patterns, five-trial mean and sample SD")
    fig.tight_layout()
    save(fig, destination / f"insertions-{implementation}.png")


def adversary(raw, destination):
    workloads = ["random", "exact", "shallow_exact", "shallow_fresh"]
    for workload in workloads:
        groups, finals = read_workload(raw, workload)
        workload_depth_graph(groups, workload, destination)
        workload_scaling_graph(finals, workload, destination)
    for implementation in ["upstream", "reference"]:
        insertion_graph(raw, implementation, destination)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-root", type=Path, default=ROOT / "results" / "figures")
    args = parser.parse_args()
    raw = ROOT / "results" / "raw"
    output = args.output_root
    analyst(raw / "analyst" / "random-raw.csv", output / "analyst")
    analyst(raw / "analyst-replay" / "raw.csv", output / "analyst-replay", svg=True)
    adversary(raw / "adversary", output / "adversary")
    print("Regenerated 16 figures from recorded CSVs; interrupted and smoke runs excluded.")


if __name__ == "__main__":
    main()
