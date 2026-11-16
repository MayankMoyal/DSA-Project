# Mayank: adversarial inputs for Project 3

This module generates reproducible input traces. It does not replace either teammate's studied data structure. See CONTRACT.md for exact semantics, INTEGRATION.md for API expectations and TOOLS.md for AI disclosure.

## Build and use

From the repository root with g++ supporting C++17:

```powershell
g++ -std=c++17 -O2 adversary/generate_baseline.cpp -o baseline.exe
g++ -std=c++17 -O2 adversary/generate_degradation.cpp -o degradation.exe
g++ -std=c++17 -O2 adversary/generate_insertions.cpp -o insertions.exe
.\baseline.exe 1000 1000000 42 random > random.trace
.\baseline.exe 1000 1000000 42 exact > exact.trace
.\degradation.exe 1000 1000000 42 shallow_exact > attack.trace
.\degradation.exe 1000 1000000 42 shallow_fresh > fresh-attack.trace
.\insertions.exe 4000 42 ascending > insertions.trace
python adversary/verify.py
```

PowerShell 7 redirection preserves these ASCII traces. For older Windows PowerShell, use `| Set-Content -Encoding ascii path.trace` to avoid UTF-16 output. CLI arguments are n, number of churn pairs, workload seed and mode; insertion-only arguments are n, seed and pattern. Positive n is required; a zero-pair trace contains only setup. The validator takes n and pair count explicitly rather than trusting comments.

## Workloads

- random: uniform live deletion followed by uniform absent insertion in [0,16n). This is ordinary discrete churn, including possible immediate reinsertion.
- exact: uniform live deletion followed by reinserting that same key, connecting to the exact-fit model.
- shallow_exact: prefer a shallow two-child node in an independent always-successor shape model, then reinsert its key.
- shallow_fresh: the same target selection but uniform absent insertion.

The shape model scans depths 0..3 in left-to-right breadth-first order and falls back to uniform deletion. It uses direct one-child splicing and successor replacement for two children. Its indexed nodes are used only for data generation. Generate an adaptive trace once and replay it unchanged on every strategy.

Insertion patterns are ascending, descending, extremes (alternating minimum/maximum), blocks (ascending 32-key blocks with every second block reversed), and shuffle. Every pattern uses the same set [0,n), with no duplicates.

## Validation and measurement

```powershell
g++ -std=c++17 -O2 adversary/validate_trace.cpp -o validate_trace.exe
.\validate_trace.exe 1000 1000000 attack.trace
g++ -std=c++17 -O2 -Isrc/structB src/structB/scapegoat_tree.cpp adversary/measure_rebuilds.cpp -o measure_rebuilds.exe
.\measure_rebuilds.exe 4000 1 202735161 0.7
python adversary/reproduce.py
```

The measurement command tests all five insertion patterns with both current teammate rebuild modes. Keep each CSV; use at least five trials before reporting mean timing and spread. Rebuild frequency is event count divided by successful insertions. Report rebuilt nodes and rotations separately. Insertion-only timing includes building the tested tree because that is the experiment; churn timing excludes initialization.

The standalone verification checks 200000 generated pairs, all insertion permutations, deterministic streams, invalid inputs, boundary-key validation, CLI round trips and malformed traces. The separate integrated reference project additionally passed one million seeded operations with a sorted-array oracle and invariant/depth audits after every operation, and tested failure shrinking. The public generator tests do not claim to replace Sai's complete harness.

## Results and limits

Measured data, summaries, plots, source hashes and machine/compiler details are under adversary/results/ in the delivered contribution. D5 is under docs/derivations/. RESULTS.md explains the findings and differentiates unchanged teammate Scapegoat measurements from reference perfect-leaves measurements. Full churn measurements use the general-folder reference Structure A extension because the measured base snapshot of teammate A has no erase/depth methods. That extension and all other roles' drafts intentionally remain outside this contribution.

To regenerate the complete reference experiment when that separate directory is available:

```powershell
python adversary/reproduce.py --reference-root "C:/Users/Mayank Moyal/Desktop/dsa/reference-project"
```

No tested schedule is claimed globally optimal. The exact-fit square-root theorem does not prove an ordinary-churn bound or a bound for biased deletion. Cache behavior has not been profiled and is not claimed. Timings are unpinned wall-clock block measurements and can vary with scheduling and background load.

Delivery began with two commits. At Mayank's request, the four remaining planned groups were combined into one immediate commit. This publishes already verified AI-assisted work and does not establish independent manual development or fulfill peer review.


Latest branch review before publication: Avinash branch 34e1eea adds erase, DeletionPolicy, metrics(), inorder() and validate(). Its unmodified two-translation-unit test build fails with duplicate definitions of erase (defined outside the class in the header without inline). This is a separate current integration issue, not a failure of the measured reference snapshot. Map metrics().average_depth(), inorder() and validate() when adapting the newer API. Krish's branch is now krish/structure-b at the previously reviewed c16f7ef commit. No teammate code was transferred or changed.
