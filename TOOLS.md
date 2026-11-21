## Analyst â€” Aditya

### Tools and assistance

I used ChatGPT and Codex to help develop the C++ benchmark and study runner.

### C++ implementation and build

The analyst benchmark uses the team's Hibbard BST from `src/structA`, scapegoat tree from `src/structB`, and seeded workload generators from `adversary`. It exercises the two Hibbard deletion policies and the two scapegoat rebuilding methods through their existing interfaces.

The measured Windows build used Zig 0.14.1 as a C++17 compiler with `-O2` and `-DNDEBUG`. The benchmark uses `std::chrono::steady_clock` to time blocks of tree updates. Each deleteâ€“insert pair contains two operations, so the reported time per operation is the elapsed update time divided by twice the number of completed pairs.

### Measurement and correctness checks

Each trial starts all four strategies with the same insertion order and replays the same sequence of deleteâ€“insert pairs. The raw CSV records the seed and trace hash so that a reviewer can identify the workload used in each comparison.

The benchmark resets the scapegoat rebuild counters after constructing the initial tree. This matters because initial insertions can trigger rebuilds before the first churn pair. At each checkpoint, the benchmark compares the ordered tree contents with the expected live keys and checks the structure's invariants. These checks and the depth scans run outside the timed update block.

### Statistics and figures

The study covers 1,000, 2,000, and 4,000 live keys, with five seeded trials at each size and nÂ² pairs per trial. The 100 checkpoints in each trial show how depth changes over time. The sample standard deviation is calculated across the five trials at a given size.

Python was used to aggregate the captured measurements, and Matplotlib produced the PNG figures in `results/figures/analyst/`. The C++ program `bench/analyst_study.cpp` provides a separate reproduction path that writes CSV summaries and SVG plots. The raw measurements and the summary used in the report are retained together so that each reported value can be checked.

### Cache profiling

DynamoRIO was tried on Windows. The tracer failed to produce usable cache counters, so the report makes no measured cache-miss claim. The analyst guide includes a Linux Cachegrind procedure for collecting those measurements separately.

### Project records

- Build and reproduction commands are in [the analyst guide](bench/ANALYST_GUIDE.md).
- The cost model and rebuild-frequency prediction are in [D6](docs/derivations/D6.md).
- The measured results and their interpretation are in [the report](REPORT.md).
- The adversary module has its own [tools record](adversary/TOOLS.md).
# Build and verification tools

The project uses a C++17 compiler for the tree implementations, generators and
benchmarks, and Python 3 for verification and experiment orchestration.

See [TESTING.md](TESTING.md) for test commands and
[adversary/TOOLS.md](adversary/TOOLS.md) for the adversary module's build,
measurement and document-generation tools. Recorded experiment evidence is
under `results/metadata/adversary/`.

## Structure B Owner â€” Scapegoat Tree

OpenAI ChatGPT assisted in creating temporary C++ test files used to test the Scapegoat Tree implementation during development. It was also used to format and organize the source code for improved readability and consistency.

The temporary test files were used to check the behaviour of the Scapegoat Tree operations, including insertion, deletion and subtree rebuilding. The source code and generated tests were reviewed and adapted to match the final implementation.

## Bahiram Sai Chhagan (mc250041007) — Oracle and D4

OpenAI ChatGPT (Codex) generated the initial Oracle C++ source, test and reduction logic, CMD scripts, proof draft, presentation material and documentation from the two course briefs and inspected repository source. The generated implementation was adapted to the repository's bool/void insert difference, four concrete configurations, existing adversary generator and root-zero depth convention. The assistant compiled and exercised the C++ using GCC on Linux, including a disposable mutation, and checked the proof against exhaustive permutations through eight keys. Git was used to inspect the source, and ReportLab to typeset the D4 PDF. I ran the Windows verification on 17 November 2026 using GCC 16.2.0 (WinLibs, C++17) and Python 3.13.17 for the fault demonstration.

One prompt I used to prepare for the presentation was “What exactly is the D4 about why are we doing it, and a script for presentation”.

Sai's subsequent edits, tests and review responses: I compiled and ran the generated C++ on Windows, verified all three full-scale churn runs and the D4 permutation checks, and reproduced the injected size bug with the two-operation trace.

## Mayank Moyal (250041027) — Adversary and D5

| Tool | How it was used |
| --- | --- |
| MSYS2 g++ 16.2.0 (C++17, `-O2`) | Compiled the seeded workload generators, trace validator and rebuild benchmark. The C++ benchmark measured elapsed time with `std::chrono::steady_clock`. [Recorded build environment](results/metadata/adversary/machine.json) |
| Python 3 | Ran the generators and validators, coordinated seeded trials and calculated means and sample standard deviations across trials. [Verification script](adversary/verify.py) · [Experiment runner](adversary/reproduce.py) |
| Matplotlib, ReportLab and PyMuPDF | Matplotlib plotted the experiment results. ReportLab generated PDFs for the recorded study, and PyMuPDF rendered and checked them. [PDF verification record](results/metadata/adversary/pdf-verification.json) |
| Git | Tracked changes to the generators, validation code and D5. The experiment records identify the source snapshot used for measurement. [Source record](results/metadata/adversary/source-before-measurement.json) |

### Workload design and verification

The design drew on work by Eppinger, Culberson and Munro, Galperin and Rivest, and Stout and Warren. D5 explains the distinction between exact-fit deletion and reinsertion and the project's shallow-target heuristic. Verification led to a correction in target selection so the generator only searches depths 0 through 3 before falling back to uniform deletion. [D5 derivation](docs/derivations/D5.md) · [Workload contract](adversary/CONTRACT.md)

From the repository root, run `python adversary/verify.py` to check deterministic replay, valid delete–insert pairs, insertion permutations and rejection of malformed traces. Run `python adversary/reproduce.py` to repeat those checks and measure both scapegoat rebuild methods at 1,000, 2,000 and 4,000 keys with five seeded trials per size.

The published churn measurements used a separate reference checkout that included changes to the draft tree implementations, including perfect-leaves rebuilding. Repeating that study requires passing the matching checkout to `reproduce.py` with `--reference-root`. [Measured results](adversary/RESULTS.md) · [Verification and integration limits](adversary/VERIFICATION.md)

AI tools assisted with some of the adversary work, including preparation of this tools disclosure.
