# Tree interface and invariants

Project 3 · Search Trees Under Long-Running Churn · Architect: Mallhar Totey

## Set operations

Both trees store unique signed 64-bit keys (`long long` on supported builds).
The interface below defines their common behavior. [The C++ adapter](include/tree_adapter.h)
normalizes insertion, measurement, ordered traversal and validation for the simulator.

| Operation | Contract | Hibbard BST | Scapegoat tree |
| --- | --- | --- | --- |
| Insert | Add a key only if absent and report whether size increased | `insert(key)` | `insert(key)` returns void. The adapter checks the size change. |
| Erase | Remove a key only if present and return whether it was removed | `erase(key)` | `erase(key)` |
| Find | Return whether a key is present | `contains(key)` | `contains(key)` |
| Size | Return the current live-key count | `size()` | `size()` |
| Depth and height | Return mean node depth and maximum node depth in edges | `metrics()` | `depthStats()` |
| Ordered keys | Return the live keys in strictly increasing order | `inorder()` | `toVector()` |
| Validate | Verify the structure's ordering, count and applicable balance bookkeeping | `validate()` | `checkInvariants()` |

The root has depth zero. Average depth is the sum of depths divided by the
node count. The empty tree reports average depth zero and height −1.

## Structural invariants

- Every left descendant is strictly smaller than its ancestor and every
  right descendant is strictly greater. All live keys are reachable from
  the root and their count equals `size()`.
- Structure A performs iterative BST updates and supports always-successor
  or randomized predecessor/successor replacement on two-child deletions.
- Structure B uses α = 0.7 in the simulator. A sufficiently deep insertion
  causes an ancestor subtree to be rebuilt. A deletion that drops the
  size below α times the recorded maximum triggers a whole-tree rebuild.
  Between rebuilds, the height bound uses that recorded maximum. A local
  α-weight imbalance can exist between rebuilds. Both rebuild methods
  preserve ordering and the live set.

## Churn simulator contract

The [adversary contract](adversary/CONTRACT.md) defines the deterministic
initial insertion order, the four workload generators and the FNV-1a trace
hash. For each pair, delete a live key, then insert a key absent after
deletion. This permits reinserting the same key in exact-fit workloads.

The simulator generates each pair once and applies it to all four trees.
The randomized deletion policy owns a separate RNG initialized from the
recorded seed. The simulator checks success and population after each update,
and compares each ordered traversal and structure check against the expected
live set at each checkpoint. Its membership flags use the finite generator
domain `[0, 16n)` and are separate from the studied trees. The standalone
sorted-array oracle audits the general signed-key interface when its module is
present.

The CSV samples pair zero, every requested interval and the final pair.
The trace hash includes setup and all updates through that checkpoint. All
four strategies share a hash for a given checkpoint. Depth and rebuild counts
depend on the strategy. Cumulative rebuild counters exclude initialization.

| Live keys | Minimum churn pairs per strategy |
| ---: | ---: |
| 1,000 | 1,000,000 |
| 2,000 | 4,000,000 |
| 4,000 | 16,000,000 |

Use the Analyst's timed benchmark for operation latency. The application
performs validation and CSV reporting as part of its run.
