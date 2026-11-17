# Build and verification tools

The project uses a C++17 compiler for the tree implementations, generators and
benchmarks, and Python 3 for verification and experiment orchestration.

See [TESTING.md](TESTING.md) for test commands and
[adversary/TOOLS.md](adversary/TOOLS.md) for the adversary module's build,
measurement and document-generation tools. Recorded experiment evidence is
under `adversary/results/`.

## Structure B Owner — Scapegoat Tree

OpenAI ChatGPT assisted in creating temporary C++ test files used to test the Scapegoat Tree implementation during development. It was also used to format and organize the source code for improved readability and consistency.

The temporary test files were used to check the behaviour of the Scapegoat Tree operations, including insertion, deletion and subtree rebuilding. The source code and generated tests were reviewed and adapted to match the final implementation.