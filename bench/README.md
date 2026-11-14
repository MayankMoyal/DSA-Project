# Analyst: BST insertion/lookup baseline

This is an **early benchmark** of the available `HibbardBst` interface on `main`.
It measures insertion time and successful lookup time in two situations:

- `sorted`: insert keys `0, 1, ..., n-1`, which creates a worst-case chain in an unbalanced BST.
- `shuffled`: insert the same keys in a seeded random permutation.

For each mode, run five trials at `n = 1000, 2000, 4000`. Every trial writes a CSV row containing the mode, size, trial, seed, insertion time, lookup time, and number of successful lookups.

## Build and run

Run from the repository root. On Windows PowerShell with `g++` installed:

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -Isrc/structA src/structA/hibbard_bst.cpp bench/bst_baseline.cpp -o bst_baseline.exe
.\bst_baseline.exe > baseline.csv
```

On Linux/macOS:

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Isrc/structA src/structA/hibbard_bst.cpp bench/bst_baseline.cpp -o bst_baseline
./bst_baseline > baseline.csv
```

Use the raw CSV rows to compute each mode/size's mean and spread (e.g. sample standard deviation) across the five trials. Report the CPU, cache sizes, compiler version, flags and operating system alongside the measurements.

## What this benchmark does *not* establish

This is **not** the full Project 3 experiment: it does not perform insertion/deletion churn, compare Hibbard deletion policies, measure average node depth over `n^2` churn pairs, or compare against Scapegoat Tree rebuilding. Those require the completed and integrated interfaces for Structure A and Structure B. In particular, worst-case sorted insertion is not evidence that *random churn* has linear cost.

## Analyst's next steps

1. Agree with the structure owners on `erase`, `averageDepth`, and rebuild/operation counter interfaces.
2. Before collecting churn results, write D6's amortized `O(log n)` Scapegoat Tree cost model and a qualitative, testable predicted rebuild-frequency model.
3. Implement seeded fixed-size churn (delete one present key, insert one absent key) for `n = 1000, 2000, 4000`, with at least `n^2` pairs each.
4. Benchmark both Hibbard deletion policies and both Scapegoat rebuild implementations, reporting repeated-run spread, average depth, rebuild frequency, counters, and cache profiling when claiming cache effects.
5. Save raw CSV with seeds under `results/`, create figures and document prediction-versus-measurement gaps in `REPORT.md`.

## AI/tool disclosure

This initial baseline and its README were drafted with ChatGPT assistance on 2026-11-14. They must be disclosed in the team's root `TOOLS.md` before the final tagged release. The owning student should compile, test, review, understand, and document subsequent changes; no performance results are claimed here.
