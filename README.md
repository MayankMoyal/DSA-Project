# Project 3 · Search Trees Under Long-Running Churn

CS 253 / MA 253 Data Structures and Algorithms Laboratory · November 2026

## Team and roles

| Member | Roll number | GitHub | Role | Module | Derivation |
| --- | --- | --- | --- | --- | --- |
| Avinash | 250041006 | Avinashbhadane1 | Structure A owner | `src/structA/` | D1 |
| Krish Zinzuvadiya | 250041022 | zinzuvadiyakrish-arch | Structure B owner | `src/structB/` | D2 |
| Mallhar Totey | 250041026 | Mallhar3923V | Architect | `app/`, `include/`, `INTERFACE.md` | D3 |
| Sai | 250041007 | saibahiram567 | Oracle owner | `oracle/` | D4 |
| Mayank Moyal | 250041027 | MayankMoyal | Adversary | `adversary/` | D5 |
| Aditya | 250001005 | Aditya-2007-creator | Analyst | `bench/`, `results/` | D6 |

## Summary

This project measures how search-tree shape changes during repeated
delete–insert pairs at a fixed live size. It compares an iterative Hibbard
BST with two deletion policies and a scapegoat tree with array or in-place
Stout–Warren rebuilding. All four strategies process the same seeded trace.
In the seed-42 application run, the always-successor BST had the largest
final average depth at all three project sizes.

## Requirements

Validated on Windows with MinGW-w64 GCC 16.1.0, CMake 4.3.3 and Python
3.11.9. The build requires C++17, CMake 3.16 or newer, Python 3.8 or newer,
and `g++` on PATH for the adversary checks. On Windows also put
`mingw32-make` on PATH. The Bash wrapper uses the same Python runner.

## Build

From the repository root in PowerShell:

```powershell
python scripts/run_all.py --build-only
```

The build writes executables to `.build/bin/`.

## Quick start

```powershell
& .\.build\bin\churn_simulator.exe 3 1 42 exact 1
```

The output contains four setup rows and four final rows. The first final row
is:

```csv
hibbard_successor,exact,3,1,42,1,2,3,1710469705799210278,0,0,0
```

## Tests

```powershell
python scripts/run_all.py --tests-only
```

CTest runs the focused structure suites, the standalone Structure A churn
verifier and the simulator CSV regression tests. When the Oracle module is
present, it also runs the invariant and sorted-array fixtures, the one-million
operation differential test with seed `20261117`, and the replay fixture.
A passing run ends with `100% tests passed`. See [TESTING.md](TESTING.md)
for individual commands and the adversary checks.

## Evaluation interface

```text
churn_simulator <n> <pairs> <seed> [random|exact|shallow_exact|shallow_fresh] [sample_every]
```

All arguments are positional. Sizes range from 1 through 1,000,000. The
pair count and seed are unsigned 64-bit decimals. Zero pairs samples
initialization only. The default workload is `random`. The default interval
is `max(1, floor(pairs/100))`.

The application reads its arguments and prints CSV to stdout. Rows are grouped
by checkpoint, with the same strategy order as the result table below. The CSV
header is:

```csv
strategy,workload,n,pairs_done,seed,average_depth,height,live_size,trace_hash,rebuild_events,nodes_rebuilt,stout_warren_rotations
```

A valid run exits with code 0. An invalid configuration or failed audit exits
with a nonzero code and writes a diagnostic to stderr. See
[INTERFACE.md](INTERFACE.md) for the contract.

## Benchmarks and figures

One command builds, runs tests and reproduces the experiments available in the
checkout:

```powershell
python scripts/run_all.py
```

On Linux, use `bash scripts/run_all.sh`. Append `--quick` for
a 10,000-pair simulator run and a short Analyst run when its source is present.

The full command runs the baseline, seeded simulator runs of 1, 4 and 16
million pairs per strategy, and the adversary reproduction checks and
insertion experiments. With the Analyst module present, it also runs the
five-trial churn study and writes its depth plots. Fresh CSVs and figures go
under timestamped directories in `results/raw/` and `results/figures/`.
The simulator runs take tens of seconds on the verified Windows machine.
Allow additional time for compilation and the five-trial study.

## Repository map

| Path | Contents |
| --- | --- |
| `app/` | Fixed-population simulator |
| `include/` | Common C++ tree adapter |
| `src/` | Tree implementations and focused suites |
| `tests/` | Simulator CLI and CSV regression tests |
| `oracle/` | Sorted-array oracle and differential runner, supplied by the Oracle branch |
| `adversary/` | Workload generators, validation tools and recorded adversarial evidence |
| `bench/` | BST baseline and the Analyst's benchmark integration point |
| `scripts/` | One-command build, test and experiment runner |
| `docs/` | Derivation sources and PDFs |
| `results/` | Output location for fresh raw CSVs and figures |

## Results at a glance

Final average node depth in edges for one seeded application run
(`random`, seed 42). Each row ran n² pairs per strategy.

| Live keys | Hibbard successor | Hibbard randomized | Scapegoat array | Scapegoat Stout–Warren |
| ---: | ---: | ---: | ---: | ---: |
| 1,000 | 15.010 | 9.688 | 11.185 | 11.518 |
| 2,000 | 17.643 | 10.328 | 12.311 | 11.769 |
| 4,000 | 22.919 | 12.449 | 16.419 | 15.831 |

The [adversarial results](adversary/RESULTS.md) cover the separate workload
study. The Analyst branch supplies the comparative report and repeated timing
measurements.

## Known limitations

The Oracle and Analyst contributions live on their respective branches in
the current checkout. The runner announces their availability and enables
their entry points when their sources are present. The team name and remaining
role-table identifiers need confirmation before submission. The command
above describes the simulator CLI. The instructor's separate hidden-test I/O
specification has yet to be confirmed.

## Tools and references

- [Tool disclosure](TOOLS.md)
- [D3 · B-tree height bound](docs/derivations/D3_250041026.pdf)
- [D5 · Adversarial schedules and cited papers](docs/derivations/D5.md)
- [Bayer and McCreight · Organization and Maintenance of Large Ordered Indices](https://doi.org/10.1007/BF00288683)
