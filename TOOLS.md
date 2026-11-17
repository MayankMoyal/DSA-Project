## Analyst — Aditya

### Tools and assistance

I used ChatGPT and Codex to help develop the C++ benchmark and study runner.

### C++ implementation and build

The analyst benchmark uses the team's Hibbard BST from `src/structA`, scapegoat tree from `src/structB`, and seeded workload generators from `adversary`. It exercises the two Hibbard deletion policies and the two scapegoat rebuilding methods through their existing interfaces.

The measured Windows build used Zig 0.14.1 as a C++17 compiler with `-O2` and `-DNDEBUG`. The benchmark uses `std::chrono::steady_clock` to time blocks of tree updates. Each delete–insert pair contains two operations, so the reported time per operation is the elapsed update time divided by twice the number of completed pairs.

### Measurement and correctness checks

Each trial starts all four strategies with the same insertion order and replays the same sequence of delete–insert pairs. The raw CSV records the seed and trace hash so that a reviewer can identify the workload used in each comparison.

The benchmark resets the scapegoat rebuild counters after constructing the initial tree. This matters because initial insertions can trigger rebuilds before the first churn pair. At each checkpoint, the benchmark compares the ordered tree contents with the expected live keys and checks the structure's invariants. These checks and the depth scans run outside the timed update block.

### Statistics and figures

The study covers 1,000, 2,000, and 4,000 live keys, with five seeded trials at each size and n² pairs per trial. The 100 checkpoints in each trial show how depth changes over time. The sample standard deviation is calculated across the five trials at a given size.

Python was used to aggregate the captured measurements, and Matplotlib produced the PNG figures in `results/analyst`. The C++ program `bench/analyst_study.cpp` provides a separate reproduction path that writes CSV summaries and SVG plots. The raw measurements and the summary used in the report are retained together so that each reported value can be checked.

### Cache profiling

DynamoRIO was tried on Windows. The tracer failed to produce usable cache counters, so the report makes no measured cache-miss claim. The analyst guide includes a Linux Cachegrind procedure for collecting those measurements separately.

### Project records

- Build and reproduction commands are in [the analyst guide](bench/ANALYST_GUIDE.md).
- The cost model and rebuild-frequency prediction are in [D6](docs/derivations/D6.md).
- The measured results and their interpretation are in [the report](REPORT.md).
- The adversary module has its own [tools record](adversary/TOOLS.md).