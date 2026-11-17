
## Sai — Oracle and D4

OpenAI ChatGPT (Codex) generated the initial Oracle C++ source, test and reduction logic, CMD scripts, proof draft, presentation material and documentation from the two course briefs and inspected repository source. The generated implementation was adapted to the repository's bool/void insert difference, four concrete configurations, existing adversary generator and root-zero depth convention. The assistant compiled and exercised the C++ using GCC on Linux, including a disposable mutation, and checked the proof against exhaustive permutations through eight keys. Git was used to inspect the source, and ReportLab to typeset the D4 PDF.

Sai's subsequent edits, tests and review responses: I compiled and ran the generated C++ on Windows, verified all three full-scale churn runs and the D4 permutation checks, and reproduced the injected size bug with the two-operation trace..
