# Project 3 · Search Trees Under Long-Running Churn

**Team: Underqualified** · CS 253 / MA 253 · November 2026

## Team and roles

| Member | Roll number | GitHub | Role | Module | Derivation |
| --- | --- | --- | --- | --- | --- |
| Avinash Bhadane | 250041006 | Avinashbhadane1 | Structure A | `src/structA/` | [D1](docs/derivations/D1_250041006.pdf) |
| Krish Zinzuvadiya | 250041022 | zinzuvadiyakrish-arch | Structure B | `src/structB/` | [D2](docs/derivations/D2_250041022.pdf) |
| Mallhar Totey | 250041026 | Mallhar3923V | Architect | `app/`, `include/` | [D3](docs/derivations/D3_250041026.pdf) |
| Sai | 250041007 | saibahiram567 | Oracle | `oracle/` | [D4](docs/derivations/D4_250041007.pdf) |
| Mayank Moyal | 250041027 | MayankMoyal | Adversary | `adversary/` | [D5](docs/derivations/D5_250041027.pdf) |
| Aditya | 250001005 | Aditya-2007-creator | Analyst | `bench/`, `results/` | [D6](docs/derivations/D6_250001005.pdf) |

## Summary

The application measures search-tree shape during repeated delete-insert pairs
at a fixed population. It compares iterative Hibbard deletion using the successor
or a random replacement side against scapegoat trees using array or Stout-Warren
rebuilding. All four strategies consume identical seeded inputs. In the recorded
five-trial ordinary-churn study, the always-successor BST had the largest mean
final depth at all three tested sizes; this finite experiment is not an
asymptotic proof.

## Requirements

- C++17 compiler, CMake 3.16 or newer, and Python 3.8 or newer.
- Windows: GCC/MinGW-w64 (`g++` and `mingw32-make` on PATH). Linux: GCC and Make.
- Plotting and PDF export require Python 3.9 or newer. Install the packages with `python -m pip install -r scripts/requirements.txt`.
- Development checks used Windows 11, GCC 16.2.0 and Python 3.11. Recorded timing
  studies used different stated machines/compilers; see [REPORT.md](REPORT.md).
- Standard library containers support generators, traversals and the oracle;
  they do not replace either studied tree.

## Build

From the repository root:

```powershell
python scripts/run_all.py --build-only
```

Executables go under `.build/bin/`. CMake must be installed and available on PATH.
Generated executables and debug files are excluded from Git.

## Quick start

In PowerShell:

```powershell
& .\.build\bin\churn_simulator.exe 3 1 42 exact 1
```

On Linux use `./.build/bin/churn_simulator 3 1 42 exact 1`. The CSV has a header,
four setup rows and four final rows. Its first final row is:

```csv
hibbard_successor,exact,3,1,42,1,2,3,1710469705799210278,0,0,0
```

## Tests

```powershell
python scripts/run_all.py --tests-only
```

This builds and runs the structure suites, Structure A's full-scale churn
verifier, simulator CSV checks, sorted-array/invariant fixtures, exhaustive D4
cases, one million differential operations with seed `20261117`, and replay.
A successful CTest run prints `100% tests passed` and exits zero.

For the additional Oracle full-scale campaigns and disposable-fault demo, use CMD:

```bat
oracle\run.cmd full
oracle\fault-demo.cmd
```

The full Oracle command checks 1, 4 and 16 million churn pairs at n=1000, 2000,
4000 with workload seed 42; randomized deletion uses policy seed 253. The fault
demo expects a deliberate size failure and reduces it to `I 17`, `D 17`.
See [TESTING.md](TESTING.md) and [oracle/README.md](oracle/README.md).

## Evaluation interface

```text
churn_simulator <n> <pairs> <seed> [random|exact|shallow_exact|shallow_fresh] [sample_every]
```

Arguments are positional; n is 1..1,000,000, and pairs/seed are unsigned 64-bit
decimals. Zero pairs emits initialization only. Defaults are `random` and
`max(1, floor(pairs/100))`. Input is CLI arguments; output is stdout CSV:

```csv
strategy,workload,n,pairs_done,seed,average_depth,height,live_size,trace_hash,rebuild_events,nodes_rebuilt,stout_warren_rotations
```

Root depth is zero. Rows are grouped by checkpoint, with four strategies per
checkpoint. A valid run exits 0; invalid inputs or failed audits exit nonzero
and report to stderr. The quick-start command is a complete small example.
[INTERFACE.md](INTERFACE.md) defines the contract. A separate instructor-provided
hidden-test interface has not been confirmed; this is the implemented interface.

## Benchmarks and figures

```powershell
python scripts/run_all.py
```

Linux equivalent: `bash scripts/run_all.sh`. The command builds, tests, runs the
baseline and n²-pair simulator campaigns, verifies the adversary generators,
runs 15 insertion experiments and the five-trial analyst study, and regenerates
recorded figures. Allow several minutes or longer depending on hardware; the
full analyst study alone replays 105 million pairs across four strategies.

Use `--quick` for a short experiment run. To regenerate all recorded figures
from their CSVs without compiling or measuring:

```powershell
python scripts/run_all.py --figures-only
```

Recorded logs live in `results/raw/`, summaries in `results/summary/`, and plots
in `results/figures/`. New experiments use timestamped `run-*` folders, preserving
submitted measurements. [results/README.md](results/README.md) explains provenance
and historical reference limitations. Plot generation uses `scripts/plot_results.py`.
Export the matching report PDF with `python scripts/export_report.py`.
After building, run `python scripts/run_alpha055.py` to repeat the separate
five-trial study at α = 0.55. The default output is `.build/alpha055-local-replay/`.
For another run, pass `--output` with a fresh directory name. The [recorded
final depths](results/summary/analyst/alpha055-random/final-depth-summary.csv)
cover all four strategies at 1,000, 2,000 and 4,000 live keys.

## Repository map

| Path | Contents |
| --- | --- |
| `src/structA/`, `src/structB/` | Studied trees; existing focused tests in `src/tests/` |
| `include/` | Shared adapter |
| `app/` | Fixed-population simulator |
| `oracle/` | Sorted-array reference, differential tests and reducer |
| `adversary/` | Input generators, trace validation and workload explanations |
| `bench/` | Benchmark harness and cost predictions |
| `tests/` | Application regression and repository-layout checks |
| `scripts/` | Build/test/experiment runner, plot and report exporters |
| `frontend/` | Local dashboard for running the C++ simulator, viewing tree snapshots and exporting checkpoint data. See the [dashboard instructions](frontend/README.md). |
| `results/` | Raw CSVs, summaries, generated figures and measurement metadata |
| `docs/derivations/` | D1-D6 sources and `D<number>_<roll>.pdf` deliverables |
| `docs/slides/` | [Presentation PDF](docs/slides/slides.pdf), [animated PowerPoint](docs/slides/slides.pptx) and [offline demonstration](docs/slides/Live-demo.html). |
| Root documents | README, interface, report and tool records |

## Results at a glance

Recorded ordinary random churn, five trials per size, n² pairs per trial.
Mean final node depth in edges (rounded):

| Live keys | Hibbard successor | Hibbard randomized | Scapegoat array | Scapegoat DSW |
| ---: | ---: | ---: | ---: | ---: |
| 1,000 | 13.84 | 9.63 | 11.36 | 11.49 |
| 2,000 | 17.83 | 10.95 | 13.17 | 12.79 |
| 4,000 | 23.98 | 12.87 | 14.48 | 14.44 |

The table above uses α = 0.70 for both Scapegoat trees. In the separate
α = 0.55 study, both rebuild methods had lower mean final depth than randomized
Hibbard at all three sizes. The two studies used different seeds, so their
difference cannot be attributed to α alone.

The [report](REPORT.md) and [summary CSV](results/summary/analyst/random-summary.csv)
include sample SD, timings, rebuild rates, measurement setup and limitations.
Targeted reference experiments are reported separately in
[adversary/RESULTS.md](adversary/RESULTS.md).

## Known limitations

- The final release tag and instructor submission are pending.
- Derivation PDFs retain their authors' content; several still need length and
  authorship details checked against the final one-to-two-page requirement.
- Historical reference churn timings require a separate recorded implementation
  to rerun. All stored plots can be regenerated from the included CSVs.
- No usable hardware cache profile is claimed. The instructor's hidden-test I/O
  contract and each teammate's final tool records still need confirmation.
- See [docs/SUBMISSION_CHECKLIST.md](docs/SUBMISSION_CHECKLIST.md) before release.

## Tools and references

See [TOOLS.md](TOOLS.md), the per-module
records, and [REPORT.md](REPORT.md) section 13. Key sources are Hibbard (1962),
Eppinger (1983), Culberson and Munro (1990), Galperin and Rivest (1993),
Stout and Warren (1986), and Bayer and McCreight (1972).
