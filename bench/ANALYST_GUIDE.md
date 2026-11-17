# Aditya's analyst guide

## 1. Follow one key through the benchmark

Start in `bench/churn_benchmark.cpp`. Read `make_trace`, `Measured::apply` and `replay` in that order. Mayank's generator chooses a live slot, removes its key and draws an absent key from a universe of 16n. It can redraw the removed key. The resulting trace is shared across all four strategies. A separate RNG chooses the replacement side in randomized Hibbard deletion. The tree starts each pair with n keys, visits nâˆ’1, then returns to n.

Avinash's iterative Hibbard BST exposes boolean insert/erase, inorder, validate and metrics. Krish's scapegoat exposes a void insert, boolean erase, toVector, checkInvariants and rebuild counters. The adapter preserves these APIs. Both measure depth in edges with root depth zero. Outside each timed block the harness reconstructs a sorted sequence from membership flags and compares every key and each tree's invariants. Sai owns the separate sorted-array differential tester and shrinking. Mallhar can call the analyst runner from the team demo and use the same pair counts and depth convention. Mayank owns the targeted traces and their interpretation.

## 2. Build and check the predictions

From the repository root on branch `aditya/analyst-churn`, use Windows CMD with a modern C++17 compiler. For Zig, substitute `zig c++` for `g++`.

```bat
if not exist build mkdir build
g++ -std=c++17 -O2 -DNDEBUG -Isrc/structA -Isrc/structB -Iadversary src/structA/hibbard_bst.cpp src/structB/scapegoat_tree.cpp bench/churn_benchmark.cpp -o build/churn_benchmark.exe
g++ -std=c++17 -O2 bench/predict_rate.cpp -o build/predict_rate.exe
build\predict_rate.exe
build\churn_benchmark.exe --quick
```

The locally recorded forecast predates the measured runs and records the predictions for these new experiments. Read the last page of D6 and work the inequality 0.7sâˆ’2s/3=s/30 on paper. Then explain why an unchanged fixed population never triggers the shrink threshold at these sizes.

## 3. Reproduce the data and figures

Use a fresh output folder to keep the submitted raw logs available for comparison. The runner starts the benchmark from its current directory. Run both executables from build and check for exit code 0 before using the outputs.

```bat
g++ -std=c++17 -O2 bench/analyst_study.cpp -o build/analyst_study.exe
pushd build
analyst_study.exe ..\results\raw\fresh-replay
echo Exit code: %ERRORLEVEL%
popd
```

Each study uses nÂ² pairs, 100 evenly spaced checkpoints and five trials. The full study generates 105 million pairs and replays each pair four times. Trees execute sequentially with rotated order. The timer covers tree updates. Divide time by 2 Ã— pairs for ns per individual operation. Divide local events or rebuilt nodes by pairs for work per insertion opportunity. Every summary SD uses the five independent trial values, never the 100 correlated checkpoints. The seed and full trace hash should match when rerunning the same input. Times depend on the machine.

To repeat a targeted study, invoke build/churn_benchmark.exe 1000 1000000 202642161 1 shallow_exact 10000, substituting exact or shallow_fresh as desired. An exact-fit pair reinserts the deleted key. A shallow schedule biases which live key is deleted. Use Mayank's published experiment labels when presenting the existing adversarial results.

## 4. Hand off and defend your results

- Give Mallhar the root REPORT, the three depth figures, the summary CSV and your D6 PDF. Include the benchmark runner in the team's release command. After the team merges its modules, use a fresh clone of the final tag for the presentation. A PR build and a tagged team demo are distinct milestones.
- For your three-to-four-minute segment, spend 40 seconds showing a shared trace and the timer boundary, one minute deriving the potential drop, one minute on the n=4000 depth plot and prediction error, and the remaining time on evidence limits or questions. Highlight that whole-tree rebuilds and insertion-triggered local rebuilds have different causes.
- Practise a live edit: change the checkpoint count in a single benchmark invocation, rerun a short trial with the same seed and confirm the final depth and hash. Explain why the timing and number of CSV rows can change. Practise changing alpha and recomputing the threshold before predicting how it affects rebuilds.
- For the cross-module viva, explain why replacing a two-child node with the minimum of its right subtree preserves ordering. For the board question, if every child on a depth-d path held at most alpha times its parent, the leaf size would be at most n Ã— alpha^d. This is less than one past the depth limit. Some ancestor must violate the bound.
- Ask a teammate to review the counter reset, the trigger probability and one raw-to-summary calculation on the PR. Address the feedback there. The course awards Git marks for genuine review and work across the project period. Your existing commits and these follow-up commits retain their real dates.


## Cache profiling on a Linux lab machine

From the repository root on Linux, compile with the benchmark command above, adding -g and changing the output path to ./churn_benchmark. Install Valgrind if the lab image lacks it. Run this loop from Bash. Each invocation profiles one strategy on the same seed. Preserve the four output files with the machine description.

```sh
mkdir -p results/metadata/analyst/cache
for strategy in successor random_policy scapegoat_array scapegoat_dsw
do
  valgrind --tool=cachegrind --cache-sim=yes --cachegrind-out-file=results/metadata/analyst/cache/$strategy.cg ./churn_benchmark 1000 10000 202642161 1 random 10000 "$strategy" > /dev/null
  cg_annotate --show=Dr,D1mr,Dw,D1mw,DLmr,DLmw results/metadata/analyst/cache/$strategy.cg > results/metadata/analyst/cache/$strategy.txt
done
```

Compare (D1mr+D1mw)/(Dr+Dw) for simulated L1 data miss rate. Whole-program totals include generation, validation and initialization. Use the annotated function rows to locate tree work. Cachegrind models a cache hierarchy. Record its configuration and describe simulated misses separately from native wall-clock timings.
