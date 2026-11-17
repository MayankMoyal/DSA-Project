# Tools used

| Tool and version | Used by | Used for | Files affected | What was changed or verified |
| --- | --- | --- | --- | --- |
| OpenAI Codex, 17 November 2026 | Mallhar Totey, Architect | Assisted with simulator replay and build integration, tests, documentation and D3 | `app/`, `include/`, `tests/`, `scripts/`, `CMakeLists.txt`, `INTERFACE.md`, `README.md`, `docs/derivations/D3*` | Preserved the existing generators and tree APIs. Checked the CLI on all four workloads and ran the three required full-scale random traces. |
| GCC 16.1.0, CMake 4.3.3 and Python 3.11.9 | Architect | Built C++17 executables, ran suites and checked CSV output | Build output and generated CSVs | Verified checkpoint schedule, live size, trace hashes and invalid CLI input. |
| Python ReportLab and Poppler | Architect | Generated and rendered D3 | `docs/derivations/D3_250041026.pdf` | Compared the rendered proof with its source. |

The Adversary's module-specific disclosure is in [adversary/TOOLS.md](adversary/TOOLS.md).

## Representative prompt

17 November 2026, OpenAI Codex:

> I need you to now follow the structure as given in the above documents and everything and complete this implementation, my role is architect
