# Sai's Oracle for search trees under churn

Run from the repository root in Windows Command Prompt with a C++17 g++ on PATH. The two studied trees and Mayank's workload generator come from the checkout. The Oracle compiles against them directly. Its signed 64-bit keys have set semantics.

```bat
oracle\run.cmd
oracle\run.cmd full
```

The first command builds the executables in `oracle\.build`, runs the array and invariant fixtures, enumerates the small D4 cases, runs one million seeded operations and replays a short fixture. The second also runs n² ordinary delete–insert pairs at each of the three project sizes. Run either command from CMD. In a parent batch file use `call oracle\run.cmd full`.

## Operations and audits

The reference is a sorted `std::vector<long long>`. Binary search costs O(log n) comparisons. Insertion and deletion can shift O(n) elements. Storage is O(n). There are no child links, rotations, rebuild decisions or deletion-policy choices in the model. A bug in one of those tree algorithms cannot be reproduced by sharing its implementation with the reference.

| Operation | Expected behavior | Checked after each operation |
|---|---|---|
| I k | Insert only when absent | Both A insert returns, all four sizes and post-insert membership |
| D k | Erase only when present | All four erase returns, sizes and post-erase membership |
| C k | Return whether k is present | All four lookup answers and sizes |

B's public insert returns void. A duplicate must leave the size unchanged and the key present. At a full audit the complete inorder vector must equal the reference. This also catches an unrelated key lost on a different path. A's validate() checks inherited strict bounds and reachable count. B's checkInvariants() checks BST bounds, count, max_n, the empty state and its logarithmic height bound; the Oracle also checks n ≥ α·max_n after an operation. α = 0.7. A scapegoat tree can have a locally heavy child between rebuilds. Requiring every node to be weight-balanced after every operation would incorrectly reject valid trees.

The mixed stream draws I/D/C uniformly with the repository's deterministic bounded sampler, on keys −1024 through 1024 inclusive. This small domain creates duplicate inserts, missing deletes, hits and misses. A separate fixture covers signed extremes, root deletion, repeated updates and ordered insertion/deletion. The same operation reaches each strategy. The randomized A instance starts with policy seed 253 on every replay. All variants may have different depths for the same set.

The runner checks the first 64 operations in full, then every 4096th and the final state. Each full audit costs O(n) per tree. The small invariant fixture audits every operation. The million-operation test exercises the API; the separate churn campaigns use n = 1000, 2000, 4000 and n² *pairs*, plus n initial inserts. Timing the Oracle would include verification and array shifts, so use the Analyst's harness for performance measurements.

## Individual runner commands

```bat
oracle\.build\oracle.exe mixed 20261117 1000000
oracle\.build\oracle.exe churn 1000 1000000 42 random
oracle\.build\oracle.exe churn 1000 10000 42 exact
oracle\.build\oracle.exe replay oracle\shrink_fixture.txt
```

Replay accepts one uppercase operation and one decimal key per line, blank lines and full-line # comments. An empty trace audits all four empty structures. Inputs with extra columns, unknown operations and out-of-range keys fail. General set replay permits absent deletes and duplicate inserts. To verify Mayank's *fixed-population workload contract* as well as the set results, run his validator first. His generator can also produce a targeted trace for the exact same four trees:

```bat
g++ -std=c++17 -O2 adversary\generate_degradation.cpp -o "oracle\.build\degrade.exe"
g++ -std=c++17 -O2 adversary\validate_trace.cpp -o "oracle\.build\validate.exe"
oracle\.build\degrade.exe 100 1000 42 shallow_fresh > "oracle\.build\attack.txt"
oracle\.build\validate.exe 100 1000 "oracle\.build\attack.txt" && oracle\.build\oracle.exe replay "oracle\.build\attack.txt"
```

Expected: validator hash 14871526626883805022, 2100 replay operations, four passing variants. Before testing a changed generator, recheck its contract.

## Reproducing a failure

The differential runner returns 0 on pass, 1 on a logical mismatch and 2 on an input or I/O error. A mismatch reports variant/check and the 1-based operation number. It writes the prefix under a new oracle-failure-N directory in the working directory. Generated streams are regenerated with their original seeds to save the prefix without retaining the whole stream in RAM. Keep the same binary, revision, alpha and policy seed. With the directory printed by the failing runner, use:

```bat
oracle\.build\shrink.exe oracle-failure-1\prefix.txt oracle-failure-1\minimal.txt
oracle\.build\oracle.exe replay oracle-failure-1\minimal.txt
```

The shrinker uses fresh trees for every candidate. It removes chunks then individual operations, retaining the original variant/check signature. It restarts after each accepted deletion. The final trace is **1-minimal**: removing any single operation loses that signature. A shorter trace reachable through a different path can exist. A minimal set-API regression may cease to be a fixed-size churn file. The reducer stores one failure in RAM and can take many replays. A passing trace is rejected. A process crash or hang requires a debugger or an external process runner; this reducer handles returned logical mismatches.

## Fault demonstration

```bat
oracle\fault-demo.cmd
```

This optional command uses the Python 3 launcher (py -3) to make one temporary source copy in an ignored build directory. That copy skips decrementing B's size when deleting 17. The eight-operation fixture first fails B-array/size at operation 8. The reducer returns I 17, D 17. Both operations are necessary: deleting from an empty tree never reaches the faulty decrement. The demonstrator expects both diagnostic runs to return 1 and returns 0 when the demonstration completes. The normal runner passes the same eight operations.

For results from the checked checkout, see VERIFICATION.md. The root README's combined command remains the team's integration point. The Architect can include `call oracle\run.cmd full` alongside the application and benchmarks there.
