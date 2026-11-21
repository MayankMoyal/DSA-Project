
## Aditya (250001005) — Analyst and D6

| Tool | How it was used |
| --- | --- |
| MSYS2 GCC 16.2.0, CMake (C++17) | Compiled and tested the C++ benchmark programs. Earlier recorded Windows measurements used Zig 0.14.1 with `-O2` and `-DNDEBUG`. |
| C++ `std::chrono` | Measured tree update times for Hibbard BST and Scapegoat Tree implementations. |
| Python 3 | Ran benchmark experiments, processed CSV files, and calculated means and sample standard deviations. |
| Matplotlib | Generated comparison graphs for four tree strategies across 1,000, 2,000 and 4,000 nodes, including the alpha = 0.55 study. |
| Git and GitHub | Managed Analyst contributions, commits, pull requests and team integration. |
| ChatGPT and Codex | Assisted with benchmark development, code simplification, experiment scripts, graph generation and documentation. The code was compiled and checked using repository tests and recorded results. |

### Benchmark and Verification

The Analyst module compares two Hibbard deletion policies and two Scapegoat rebuilding methods using identical seeded workloads.

Experiments record average depth, execution time, and rebuilding statistics. The alpha = 0.55 study includes five trials per population and compares all four strategies.

The benchmark and results are available in [bench/](bench/), [results/](results/), and [REPORT.md](REPORT.md). The mathematical analysis is documented in [D6](docs/derivations/D6.md)

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

## Avinash Bhadane (250041006): Structure A

| Tool and version or date | Used by | Used for | Files affected | What we changed or verified |
| --- | --- | --- | --- | --- |
| OpenAI ChatGPT with Codex, 21 Nov 2026 | Avinash Bhadane | Compiling and running the Structure A test suite and the full-scale churn verifier; checking README and TOOLS.md against the course handouts | `src/tests/test_struct_A.cpp`, `src/tests/test_struct_A_churn.cpp`, `README.md`, `TOOLS.md` (reviewed only; `src/structA/hibbard_bst.cpp` was read, not edited) | Both Hibbard policies passed 21,000,000 delete-insert pairs per policy at all three required sizes. Suggestions were returned in chat only. All code and documentation edits were made manually by me. |

## Krish Zinzuvadiya (250041022) — Structure B

| Tool and version or date | Used by | Used for | Files affected | What we changed or verified |
| --- | --- | --- | --- | --- |
| OpenAI ChatGPT, 21 Nov 2026 | Krish Zinzuvadiya  | Creating temporary C++ test files for the Scapegoat Tree implementation and formatting the source code for readability and consistency | `scapegoat_tree.h`, `scapegoat_tree.cpp`, temporary C++ test files | ChatGPT assisted in creating temporary tests for insertion, deletion and subtree rebuilding. The generated tests and source-code suggestions were reviewed and adapted to match the final implementation. All final changes were reviewed by me. |

## Mallhar Totey (mc250041026) - Architect and D3

| Tool and version | Used by | Used for | Files affected | What was changed or verified |
| --- | --- | --- | --- | --- |
| OpenAI Codex, 17 November 2026 | Mallhar Totey, Architect | Assisted with simulator replay and build integration, tests, documentation and D3 | `app/`, `include/`, `tests/`, `scripts/`, `CMakeLists.txt`, `INTERFACE.md`, `README.md`, `docs/derivations/D3*` | Preserved the existing generators and tree APIs. Checked the CLI on all four workloads and ran the three required full-scale random traces. |
| GCC 16.1.0, CMake 4.3.3 and Python 3.11.9 | Architect | Built C++17 executables, ran suites and checked CSV output | Build output and generated CSVs | Verified checkpoint schedule, live size, trace hashes and invalid CLI input. |
| Python ReportLab and Poppler | Architect | Generated and rendered D3 | `docs/derivations/D3_250041026.pdf` | Compared the rendered proof with its source. |


## Bahiram Sai Chhagan (mc250041007) — Oracle and D4

| Tool | How it was used |
| --- | --- |
| OpenAI ChatGPT (Codex) | Generated the initial Oracle code, tests, trace reducer, CMD scripts, D4 proof draft, slides and documentation. Ran Linux GCC checks on the code, the injected fault and all insertion orders through eight keys. |
| WinLibs GCC 16.2.0 (C++17) | On 17 November 2026, I compiled and tested all four tree variants on Windows, verified all three full-scale churn runs and checked the D4 formula against every insertion order through eight keys. |
| Python 3.13.17 | I used the Windows fault demo to reproduce the size error with two operations: `I 17`, then `D 17`. |
| Git | Inspected the repository source. |
| ReportLab | Typeset the D4 proof PDF. |

One prompt I used to prepare for the presentation was “What exactly is the D4 about why are we doing it, and a script for presentation”.

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
