# Adversary verification

Run `python adversary/verify.py` from the repository root with Python 3 and
`g++` available. This checks deterministic generation, 200000 churn pairs,
insertion patterns, CLI validation and malformed traces.

The optional `adversary/reproduce.py` insertion benchmark needs the teammate's
`src/structB` implementation. Pass `--project-root` pointing to a complete
checkout of main. Teammate implementations are intentionally not included here.
Historical full churn reruns additionally require `--reference-root` pointing
to the recorded reference project. See `adversary/RESULTS.md` for provenance.
