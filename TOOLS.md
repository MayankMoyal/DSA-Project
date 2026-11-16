# Analyst tooling

Codex assisted with the benchmark, D6 sketch, rebuild-frequency model, C++ study runner, result checks and documentation. The analyst harness reuses the team trees and generators, resets counters after initial construction and checks contents outside the timed blocks. Zig 0.14.1 compiled the captured benchmark. Python aggregated its measured CSV and Matplotlib generated the submitted PNGs. The C++ runner produces its own summary and SVGs. DynamoRIO on Windows produced no usable cache counters. See [the adversary tooling record](adversary/TOOLS.md) for that module.
