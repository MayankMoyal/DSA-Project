# Search Trees Under Long-Running Churn

## What the project does

This project studies how search trees behave when keys are repeatedly deleted and inserted. It compares an iterative binary search tree with two Hibbard deletion policies (always use the successor or choose randomly between predecessor and successor) against a scapegoat tree with array-based and in-place Stout–Warren rebuilding. The churn experiment keeps the live set at 1,000, 2,000, or 4,000 keys and runs at least n² delete–insert pairs per size, tracking average node depth and rebuilding behavior.

## Build and run on Windows

Open Command Prompt in the repository root and use a branch that contains both tree implementations, both unit test files, the Structure A churn verifier, and `bench/bst_baseline.cpp`. Use a C++17 compiler. If you installed MSYS2 UCRT64 at the default location, run:

```bat
set "PATH=C:\msys64\ucrt64\bin;%PATH%"
g++ --version
```

Copy the next line as one CMD command. It builds each executable with GCC, runs both suites and the standalone Structure A churn verifier, and runs the existing sorted/shuffled BST baseline benchmark. Each step runs only if the preceding step succeeds.

```bat
g++ -std=c++17 -O2 -Wall -Wextra -Isrc/structA src/structA/hibbard_bst.cpp src/tests/test_struct_A.cpp -o "%TEMP%\churn-tests-a.exe" && "%TEMP%\churn-tests-a.exe" && g++ -std=c++17 -O2 -Wall -Wextra -Isrc/structB src/structB/scapegoat_tree.cpp src/tests/test_struct_B.cpp -o "%TEMP%\churn-tests-b.exe" && "%TEMP%\churn-tests-b.exe" && g++ -std=c++17 -O2 -Wall -Wextra -Isrc/structA src/structA/hibbard_bst.cpp src/tests/test_struct_A_churn.cpp -o "%TEMP%\churn-verify-a.exe" && "%TEMP%\churn-verify-a.exe" && g++ -std=c++17 -O2 -Wall -Wextra -Isrc/structA src/structA/hibbard_bst.cpp bench/bst_baseline.cpp -o "%TEMP%\churn-baseline.exe" && "%TEMP%\churn-baseline.exe" > "%TEMP%\churn-baseline.csv"
```

The baseline CSV is written to `%TEMP%\churn-baseline.csv`. The Structure A verifier checks 21 million pairs per policy across the three sizes. For the full four-strategy comparison, follow the [analyst build and replay guide](bench/ANALYST_GUIDE.md). The [experiment report](REPORT.md) includes the predictions, five-trial results and depth figures.

## Team roles

| Member | Role | Responsibilities |
| --- | --- | --- |
| Avinash | Structure A owner | Implements and tests the iterative BST with Hibbard deletion under both policies: always use the in-order successor, or choose randomly between the predecessor and successor. Writes D1 on deletion correctness. |
| Krish | Structure B owner | Implements and tests the scapegoat tree, including subtree rebuilding after insertions and whole-tree rebuilding after sufficient shrinkage. Implements both array-based and in-place Stout–Warren rebuilding. Writes D2 on the existence of a scapegoat. |
| Mayank | Adversary | Designs workloads that drive the always-successor BST toward a skewed shape and trigger frequent scapegoat rebuilds. Writes D5 on the exact-fit update model and the behavior of the adversarial workloads. |
| Mallhar | Architect | Defines the shared interface and invariants in `INTERFACE.md` and builds the churn simulator that runs the same workload across the tree variants. Writes D3 on the B-tree height bound. |
| Aditya | Analyst | Builds the benchmark harness, records measurements and seeds, produces plots, and compares measured behavior with cost predictions. Writes D6 on the amortized cost of scapegoat rebuilding and predicted rebuild frequency. |
| Sai | Oracle owner | Implements the sorted-array reference, differential test harness, and invariant checks. Reduces failing traces to reproducible cases. Writes D4 on the expected average depth of a BST built by random insertion. |
