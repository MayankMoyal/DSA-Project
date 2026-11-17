# Verification against the project checkout

Base: MayankMoyal/DSA-Project main at `2dcde349c9d4214c03fe67d7ee068d3a3261bb6f`. C++17, Debian g++ 14.2.0, -O2 -Wall -Wextra -Wpedantic. These are the assistant's reproduction results using the actual tree .cpp files. Run the CMD equivalents in your own checkout and identify your compiler in your PR. No elapsed times are used as benchmarks.

| Run | Seed | Checked result |
|---|---:|---|
| Array fixture and per-step four-variant invariant fixture | policy 253 | Both passed |
| Mixed I/D/C, 1,000,000 operations per variant | 20261117 | Four passed |
| Ordinary churn, 1000 keys, 1,000,000 pairs | 42 | 2,001,000 operations per variant, passed |
| Ordinary churn, 2000 keys, 4,000,000 pairs | 42 | 8,002,000 operations per variant, passed |
| Ordinary churn, 4000 keys, 16,000,000 pairs | 42 | 32,004,000 operations per variant, passed |
| Exact-fit churn, 1000 keys, 10,000 pairs | 42 | 21,000 operations per variant, passed |
| Mayank's shallow_fresh generator, n=100, 1000 pairs | 42 | Validated hash 14871526626883805022; 2100 operations passed |
| Five insertion-only patterns from Mayank’s generator, n=1000 each | 42 | ascending, descending, shuffle, extremes and blocks all passed |
| Malformed trace records | deterministic | Unknown op, excess column, invalid decimal and key overflow each returned 2 |
| D4 exhaustive permutation check | deterministic | All n=1…8 agreed within 1e-10 |
| Healthy shrink fixture | policy 253 | Eight operations passed; reducer rejected passing input |
| Disposable Structure B size fault | policy 253 | B-array/size at operation 8, reduced to 2 and reproduced at operation 2 |

Each churn operation count includes n initial inserts. The mixed campaign and full campaigns use separate fresh trees. Generated stream parameters and the fixed policy seed reproduce the run. The full auditor runs on operations 1–64, at 4096-operation intervals, and after the last operation. No upstream defect was observed in these checks. The course examiner's hidden inputs are separate.

Run `oracle\run.cmd full` for the main results. The exact-fit, targeted-generator and injected-fault commands are in [the Oracle guide](README.md). The Windows scripts are supplied for local execution; the recorded runs used the equivalent native compiler commands on Linux.
Sai's Windows verification on 17 November 2026: All four variants passed 1,000,000 mixed operations and churn runs of 2,001,000, 8,002,000 and 32,004,000 operations per variant. D4 matched for n=1 through n=8. The fault demo reduced eight operations to insert 17 and delete 17.
