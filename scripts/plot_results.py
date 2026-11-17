"""Regenerate recorded figures from CSVs; never rerun or overwrite measurements."""
import argparse
from collections import defaultdict
import csv
import math
from pathlib import Path
import statistics
import os

ROOT = Path(__file__).resolve().parents[1]
os.environ.setdefault('MPLCONFIGDIR', str(ROOT / '.build' / 'matplotlib'))
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

STRATEGIES = ('successor', 'random_policy', 'scapegoat_array', 'scapegoat_dsw')
LABELS = ('Hibbard successor', 'Hibbard randomized', 'Scapegoat array', 'Scapegoat DSW')
COLORS = ('#c45132', '#2376ad', '#27826a', '#8354a1')


def read(path):
    with path.open(newline='', encoding='utf-8-sig') as stream:
        return list(csv.DictReader(stream))


def stats(values):
    if len(values) != 5:
        raise ValueError(f'Expected five trials, got {len(values)}')
    return statistics.mean(values), statistics.stdev(values)


def save(fig, path):
    path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(path, dpi=150, bbox_inches='tight')
    if path.suffix == ".svg":
        path.write_text("\n".join(line.rstrip() for line in path.read_text(encoding="utf-8").splitlines()) + "\n", encoding="utf-8")
    plt.close(fig)


def curves(ax, grouped, n):
    for strategy, label, color in zip(STRATEGIES, LABELS, COLORS):
        points = sorted(p for s, p in grouped if s == strategy)
        if not points:
            raise ValueError(f'Missing strategy {strategy}')
        values = [stats(grouped[strategy, p]) for p in points]
        means = [v[0] for v in values]
        sd = [v[1] for v in values]
        ax.plot(points, means, label=label, color=color)
        ax.fill_between(points, [m-s for m,s in zip(means,sd)],
                        [m+s for m,s in zip(means,sd)], color=color, alpha=.15)
    ax.axhline(2*math.log(n), color='#555555', linestyle='--', label='2 ln(n) reference')
    ax.axhline(math.sqrt(n), color='#999999', linestyle=':', label='sqrt(n) reference')
    ax.set(xlabel='Completed delete-insert pairs', ylabel='Average node depth (edges)', title=f'n = {n:,}')
    ax.grid(alpha=.2)


def analyst(raw, destination, svg=False):
    groups = defaultdict(lambda: defaultdict(list))
    for row in read(raw):
        groups[int(row['n'])][row['strategy'], int(row['pairs'])].append(float(row['average_depth']))
    for n, points in sorted(groups.items()):
        fig, ax = plt.subplots(figsize=(8, 4.7))
        curves(ax, points, n)
        ax.legend(fontsize=8, ncol=2)
        fig.suptitle('Ordinary random churn: five-trial mean and sample SD')
        fig.tight_layout()
        name = f'depth-{n}.svg' if svg else f'random-{n}-mean-depth.png'
        save(fig, destination / name)


def adversary(raw, destination):
    for workload in ('random', 'exact', 'shallow_exact', 'shallow_fresh'):
        groups = defaultdict(lambda: defaultdict(list))
        finals = defaultdict(list)
        paths = sorted(raw.glob(f'{workload}-*.csv'))
        if len(paths) != 15:
            raise ValueError(f'{workload}: expected 15 complete files')
        for path in paths:
            rows = read(path)
            last = {}
            for row in rows:
                n, pair = int(row['n']), int(row['pairs'])
                strategy = row['strategy']
                groups[n][strategy, pair].append(float(row['average_depth']))
                last[strategy] = row
            for strategy,row in last.items():
                if int(row['pairs']) != n*n:
                    raise ValueError(f'Incomplete run: {path}')
                finals[n,strategy].append(float(row['average_depth']))
        fig, axes = plt.subplots(1,3,figsize=(15,4.6))
        for ax,n in zip(axes,(1000,2000,4000)):
            curves(ax,groups[n],n)
        axes[0].legend(fontsize=7)
        fig.suptitle(f'{workload}: recorded reference implementation, mean and sample SD')
        fig.tight_layout()
        save(fig,destination/f'{workload}-depth.png')
        fig, ax = plt.subplots(figsize=(8,4.7))
        sizes=(1000,2000,4000)
        for strategy,label,color in zip(STRATEGIES,LABELS,COLORS):
            values=[stats(finals[n,strategy]) for n in sizes]
            ax.errorbar(sizes,[x[0] for x in values],yerr=[x[1] for x in values],marker='o',capsize=4,label=label,color=color)
        ax.plot(sizes,[2*math.log(n) for n in sizes],'--',color='#555555',label='2 ln(n) reference')
        ax.plot(sizes,[math.sqrt(n) for n in sizes],':',color='#999999',label='sqrt(n) reference')
        ax.set(xlabel='Live keys n',ylabel='Final average depth (edges)',title=f'{workload}: reference, after n² pairs')
        ax.legend(fontsize=8); ax.grid(alpha=.2); fig.tight_layout()
        save(fig,destination/f'{workload}-scaling.png')
    for implementation in ('upstream','reference'):
        values=defaultdict(list)
        for path in sorted(raw.glob(f'insertions-{implementation}-*.csv')):
            for r in read(path):
                values[int(r['n']),r['pattern'],r['mode']].append(int(r['rebuilds'])/int(r['n']))
        fig,axes=plt.subplots(1,3,figsize=(15,4.8))
        patterns=('ascending','descending','extremes','blocks','shuffle')
        for ax,n in zip(axes,(1000,2000,4000)):
            for j,mode in enumerate(('array','dsw')):
                data=[stats(values[n,p,mode]) for p in patterns]
                ax.bar([i+(j-.5)*.36 for i in range(5)],[v[0] for v in data],
                       yerr=[v[1] for v in data],width=.36,capsize=3,label=mode)
            ax.set_xticks(range(5),patterns,rotation=25)
            ax.set(title=f'n = {n:,}',ylabel='Local rebuild events / insertion'); ax.legend()
        fig.suptitle(f'{implementation}: insertion patterns, five-trial mean and sample SD')
        fig.tight_layout(); save(fig,destination/f'insertions-{implementation}.png')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-root',type=Path,default=ROOT/'results/figures')
    args=parser.parse_args()
    analyst(ROOT/'results/raw/analyst/random-raw.csv',args.output_root/'analyst')
    analyst(ROOT/'results/raw/analyst-replay/raw.csv',args.output_root/'analyst-replay',svg=True)
    adversary(ROOT/'results/raw/adversary',args.output_root/'adversary')
    print('Regenerated 16 figures from recorded CSVs; interrupted and smoke runs excluded.')


if __name__=='__main__':
    main()
