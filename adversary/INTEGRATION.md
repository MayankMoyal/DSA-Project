# Integration support for teammates

The published contribution changes adversary/ and Mayank's D5 files only. It does not add the reference implementations of teammates' modules or alter their APIs.

Generators are header-based and can be consumed directly by the architect's C++ simulator. `initialKeys(n, seed)` returns the initial insertion order. `Baseline(n, seed, "random"|"exact")` and `Degradation(n, seed, "shallow_exact"|"shallow_fresh")` each expose `next()` returning {removed, inserted}. `insertionKeys` returns a unique insertion permutation.

`ShapeModel` is an internal fixed-population generation helper: after construction it is updated only by a deletion followed by an insertion. Consume the generator classes rather than treating that helper as a general-purpose tree API.

Use `Validator` outside timing to reject absent deletion, duplicate insertion and population drift. It uses std::set only for validation/data generation, not as either studied structure. `replayPair` accepts the current void-returning Scapegoat insertion and the proposed bool-returning Hibbard insertion, and checks the deletion result and final size. The caller must validate that the inserted key was absent; do not treat the helper alone as a complete oracle.

Current teammate integration requirements:

- The measured base snapshot lacks erase/policy/depth APIs. Avinash's newer 34e1eea branch adds these through erase and metrics; resolve its duplicate-definition build issue and adapt the metrics names before integration. This contribution does not silently replace his implementation.
- Krish's current Scapegoat interface already supports replay and counters. `measure_rebuilds.cpp` builds with that implementation without edits. Its basic Stout-Warren shape and the reference perfect-leaves shape are measured separately; their event counts need not match.
- Mallhar should retain delete-then-insert pair semantics and supply the same trace to every strategy. Sampling depth at checkpoints avoids an O(n) scan per operation.
- Sai should compare to a sorted-array oracle, audit structure invariants, retain failing traces and shrink them. Generator tests are not a substitute for his million-operation differential harness.
- Aditya should exclude initialization, generator work, oracle work and audits from structure timing, use five runs and record seeds and machine details. Rebuild count, rebuilt-node work, rotations and time are separate columns.

The general-folder reference project demonstrates these connections and supplies reviewable proposals for other roles. Its reference-only changes must be reviewed and adopted by their owners; they are not published as Mayank's work.


Latest branch review before publication: Avinash branch 34e1eea adds erase, DeletionPolicy, metrics(), inorder() and validate(). Its unmodified two-translation-unit test build fails with duplicate definitions of erase (defined outside the class in the header without inline). This is a separate current integration issue, not a failure of the measured reference snapshot. Map metrics().average_depth(), inorder() and validate() when adapting the newer API. Krish's branch is now krish/structure-b at the previously reviewed c16f7ef commit. No teammate code was transferred or changed.
