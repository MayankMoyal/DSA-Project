# Running the tests

Run these commands from the repository root in PowerShell. You need Python 3 and a C++17 compiler available as `python` and `g++`.

## Structure tests

Build and run each suite separately. Run an executable only after its compilation succeeds, so an old executable cannot hide a build failure.

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -Isrc/structA src/structA/hibbard_bst.cpp src/tests/test_struct_A.cpp -o "$env:TEMP\dsa-tests-a.exe"
if ($LASTEXITCODE -ne 0) { throw "Structure A build failed" }
& "$env:TEMP\dsa-tests-a.exe"
if ($LASTEXITCODE -ne 0) { throw "Structure A tests failed" }

g++ -std=c++17 -O2 -Wall -Wextra -Isrc/structB src/structB/scapegoat_tree.cpp src/tests/test_struct_B.cpp -o "$env:TEMP\dsa-tests-b.exe"
if ($LASTEXITCODE -ne 0) { throw "Structure B build failed" }
& "$env:TEMP\dsa-tests-b.exe"
if ($LASTEXITCODE -ne 0) { throw "Structure B tests failed" }
```

The successful runs print:

```text
Structure A focused checks passed
All Structure B tests passed.
```

The Structure A tests cover the APIs available on the checked-out branch. Passing an older branch's suite does not establish coverage of deletion policies added later on main.

## Adversary checks

```powershell
python adversary/reproduce.py
```

This builds the generator tests and command-line tools, checks reproducibility and malformed traces, then runs 15 Scapegoat insertion experiments: five trials at each of 1000, 2000 and 4000 keys. Look for these success messages among the output:

```text
Adversary checks passed: 200000 generated pairs, all five insertion patterns, invalid inputs and boundary keys
Strict numeric CLI checks passed: malformed values, population bounds, leading zeroes, zero pairs and full-width seeds
Standalone generator, CLI, replay validation, reproducibility and malformed-trace checks passed
Reproduced teammate-current Scapegoat insertion experiments: 15 runs
```

The command should finish with exit code zero. In PowerShell, check `$LASTEXITCODE` immediately afterward. A compiler error, traceback or nonzero exit code means the run failed, even if earlier stages printed success.

Generated executables and insertion CSVs are saved under `adversary/.work/`. That directory is ignored by Git. The final note about the separate reference project is informational: this command does not rerun the full comparative churn timing dataset. See [adversary/README.md](adversary/README.md) for the optional reference-project run and experiment scope.

## Baseline CSV

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -Isrc/structA src/structA/hibbard_bst.cpp bench/bst_baseline.cpp -o "$env:TEMP\dsa-baseline.exe"
if ($LASTEXITCODE -ne 0) { throw "Baseline build failed" }
& "$env:TEMP\dsa-baseline.exe" | Set-Content -Encoding ascii "$env:TEMP\dsa-baseline.csv"
if ($LASTEXITCODE -ne 0) { throw "Baseline run failed" }
```

After a successful build and run, the CSV is at `$env:TEMP\dsa-baseline.csv`. It has this header and 30 data rows: two insertion orders, three sizes and five trials per combination.

```csv
mode,n,trial,seed,insert_ms,lookup_ms,lookups
```

Each row's `lookups` should equal `n`. Timings vary between runs. This baseline measures insertion and successful lookup; the full churn experiment has separate requirements described in [bench/README.md](bench/README.md).
