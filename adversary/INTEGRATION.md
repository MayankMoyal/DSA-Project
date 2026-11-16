# Integration support for teammates

This contribution contains adversary/ and docs/derivations/D5.md and D5.pdf. The generators use the existing tree APIs.

Generators are header-based and can be consumed directly by the architect's C++ simulator. `initialKeys(n, seed)` returns the initial insertion order. `Baseline(n, seed, "random"|"exact")` and `Degradation(n, seed, "shallow_exact"|"shallow_fresh")` each expose `next()` returning {removed, inserted}. `insertionKeys` returns a unique insertion permutation.

`ShapeModel` is an internal fixed-population generation helper: after construction it is updated only by a deletion followed by an insertion. Consume the generator classes rather than treating that helper as a general-purpose tree API.

Use `Validator` outside timing to reject absent deletion, duplicate insertion and population drift. It uses std::set only for validation/data generation, not as either studied structure. `replayPair` accepts the current void-returning Scapegoat insertion and the bool-returning Hibbard insertion, and checks the deletion result and final size. The caller must validate that the inserted key was absent; do not treat the helper alone as a complete oracle.

Integration checklist:

- Structure A on main supports erase, deletion policies, metrics, inorder traversal and validation. Use `metrics().average_depth()` for average depth.
- The Scapegoat interface already supports replay and counters. `measure_rebuilds.cpp` builds with that implementation without edits. Its basic Stout-Warren shape and the reference perfect-leaves shape are measured separately; their event counts need not match.
- Retain delete-then-insert pair semantics and supply the same trace to every strategy. Sampling depth at checkpoints avoids an O(n) scan per operation.
- Compare to a sorted-array oracle, audit structure invariants, retain failing traces and shrink them. Generator tests are not a substitute for the million-operation differential harness.
- Exclude initialization, generator work, oracle work and audits from structure timing, use five runs and record seeds and machine details. Rebuild count, rebuilt-node work, rotations and time are separate columns.

The separate reference project demonstrates these connections. Changes to other modules in that project still need review by their owners before adoption.

Compatibility review (16 November 2026): `mayank/adversary` at `df8e899` passed integration checks against `main` at `6633c5c`. The earlier Structure A duplicate-definition issue was fixed in `1a59bb1`. Current APIs include `metrics().average_depth()`, `inorder()` and `validate()`. The published timing data still describes the recorded reference snapshot, not a new performance study of main.
