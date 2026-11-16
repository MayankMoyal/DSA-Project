# Verification evidence and integration limits

The full reference validation passed one million seeded operations across both Hibbard deletion policies and both Scapegoat rebuild methods. The sorted-array oracle, node counts, order, depth and external shrink-state checks run after every differential operation. Independent fixtures check the seven-node directional example, a 12000-node chain, strict rebuild thresholds, invalid alpha values, signed key boundaries and perfect rebuilding near powers of two through 4097.

Standalone adversary verification passed 200000 generated pairs, deterministic replay, all five insertion permutations, invalid inputs and malformed traces. The shrinking test uses a deliberate dropped-deletion fault and produces a verified two-operation 1-minimal trace; no actual tree defect is being claimed by that fixture.

The dataset audit passed all 90 raw files: 60 churn runs and 30 insertion runs. There are 420 million generated pairs and 1.68 billion strategy-pair replays. Every timing group has five runs, with seeds and sample spread recorded. Initial orders match across strategies and candidate workloads. Source hashes, raw-file hashes and the resumed trial's deterministic prefix were checked. The interrupted partial run is retained and excluded from aggregates.

All eight rendered pages of D1-D6 and all ten figures were visually inspected. The PDF verifier initially rejected a line-wrapped authorship label; whitespace normalization corrected that verifier issue, and the affected check passed afterward. Other derivations and implementations remain clearly labeled reference drafts for their owners' review.

These records describe the original measured snapshot; later wording edits do not replace its source hashes. See results/validation-manifest.json, evidence-audit.json, pdf-verification.json and pdf-visual-review.json for recorded evidence. This is tested evidence, not exhaustive correctness or hidden-test certification. No cache claims or globally optimal adversarial schedules are asserted.

Only adversary files and D5 are included in this contribution. Full churn timing uses the separate reference project. PR review, final integration, the tagged release, presentation and viva remain part of the team submission.

Compatibility review (16 November 2026): `mayank/adversary` at `df8e899` passed integration checks against `main` at `6633c5c`. The earlier Structure A duplicate-definition issue was fixed in `1a59bb1`. Current APIs include `metrics().average_depth()`, `inorder()` and `validate()`. The published timing data still describes the recorded reference snapshot, not a new performance study of main.
