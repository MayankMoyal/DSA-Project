
## Oracle verification and D4 — Sai

A sorted vector gives an independent answer for signed 64-bit set membership, updates and cardinality. At every operation the harness compares each of the four strategies with that answer. Full audits compare the complete inorder sequence, check BST ordering and count, and check the scapegoat height and deletion-shrink conditions. Structure B exposes a void insertion method, so duplicate and new insertions are distinguished by post-state. The set can agree while tree depths differ.

At the tested main revision 2dcde34 the one-million-operation mixed run passed with workload seed 20261117 and randomized Hibbard policy seed 253. For ordinary churn, seed 42 passed 1, 4 and 16 million delete–insert pairs at n = 1000, 2000 and 4000 respectively across four variants. Dedicated fixtures covered boundary keys and no-op updates. In a disposable modified build, an incorrect size after erasing 17 reduced from eight operations to two. This was an intentional test of the harness. [Commands and evidence](oracle/VERIFICATION.md)

For random insertion, conditioning on the root rank gives expected internal path length P(n) = n−1 + (2/n) times the sum of P(k) for 0 ≤ k < n. The recurrence gives expected average depth 2(1+1/n)H(n)−4, asymptotically 2 ln n. Enumeration on the actual Hibbard implementation agrees for every size 1 through 8. This provides a baseline at initialization. [D4 proof by Sai](docs/derivations/D4.pdf)
