# Tools used for adversarial workloads and D5

**Mayank Moyal · Adversary · Project 3**

| Tool | How it was used |
| --- | --- |
| MSYS2 g++ 16.2.0 (C++17, `-O2`) | Compiled the seeded workload generators, trace validator and rebuild benchmark. The C++ benchmark measured elapsed time with `std::chrono::steady_clock`. [Recorded build environment](../results/metadata/adversary/machine.json) |
| Python 3 | Ran the generators and validators, coordinated seeded trials and calculated means and sample standard deviations across trials. [Verification script](verify.py) · [Experiment runner](reproduce.py) |
| Matplotlib, ReportLab and PyMuPDF | Matplotlib plotted the experiment results. ReportLab generated PDFs for the recorded study, and PyMuPDF rendered and checked them. [PDF verification record](../results/metadata/adversary/pdf-verification.json) |
| Git | Tracked changes to the generators, validation code and D5. The experiment records identify the source snapshot used for measurement. [Source record](../results/metadata/adversary/source-before-measurement.json) |

## Workload design and verification

The design drew on work by Eppinger, Culberson and Munro, Galperin and Rivest, and Stout and Warren. D5 explains the distinction between exact-fit deletion and reinsertion and the project's shallow-target heuristic. Verification led to a correction in target selection so the generator only searches depths 0 through 3 before falling back to uniform deletion. [D5 derivation](../docs/derivations/D5.md) · [Workload contract](CONTRACT.md)

From the repository root, run `python adversary/verify.py` to check deterministic replay, valid delete–insert pairs, insertion permutations and rejection of malformed traces. Run `python adversary/reproduce.py` to repeat those checks and measure both scapegoat rebuild methods at 1,000, 2,000 and 4,000 keys with five seeded trials per size.

The published churn measurements used a separate reference checkout that included changes to the draft tree implementations, including perfect-leaves rebuilding. Repeating that study requires passing the matching checkout to `reproduce.py` with `--reference-root`. [Measured results](RESULTS.md) · [Verification and integration limits](VERIFICATION.md)

AI tools assisted with some of the adversary work, including preparation of this tools disclosure.
