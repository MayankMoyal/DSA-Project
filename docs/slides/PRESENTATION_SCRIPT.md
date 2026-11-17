# Revised presentation script: 12 slides, rubric-aligned rehearsal

Avinash and Krish have expanded segments, with a separate proof slide each. Times include pointing and handovers and are rehearsal targets.

| Person | Slides | Part | Rehearsal budget |
| --- | --- | --- | --- |
| Mallhar | 1–2, 12 | Problem, interface, D3 and demo narration | 1:40 + demo |
| Avinash | 3–4 | Hibbard design and D1 proof | 2:30 |
| Krish | 5–6 | Scapegoat design and D2 proof | 3:00 |
| Sai | 7 | Oracle design and D4 | 1:20 |
| Mayank | 8 | Adversary, D5 and measured effects | 1:30 |
| Aditya | 9–11 | D6, model gaps, depth and latency | 2:00 |

Prepared explanation budgets total 12 minutes, leaving 3 minutes for demo and handovers within a 15-minute slot. This is a compressed rehearsal allocation, not proof that the live run fits. Time the full-scale run on the presentation machine. If it exceeds the remaining time, agree the demo arrangement with the instructor rather than reducing input sizes.

**Unresolved brief conflict:** six individual segments of 3–4 minutes require 18–24 minutes before demo time. No allocation can simultaneously meet that requirement and the 15-minute team limit. Confirm which timing rule the instructor intends. For the longer interpretation, rehearse each member's proof and module explanation for at least 3 minutes, plus separately timed demo.

Order: Mallhar → Avinash → Krish → Sai → Mayank → Aditya → Mallhar.
The following scripts are full rehearsal wording; shorten spoken transitions to meet an agreed allocation without skipping proof steps.

Read the quoted text aloud. Bracketed text gives actions. Pause between proof steps so the audience can follow the equations.

## Mallhar — slides 1–2
### Slide 1: problem statement and motivation

“Good morning. We are Team Underqualified. Our project asks how search trees behave under long-running churn: repeated deletions and insertions while the number of live keys stays fixed.

A correct index may still become deeper and slower to search. We isolate deletion bias by comparing successor and randomized replacement, then compare array and DSW scapegoat rebuilding to measure what preventing degradation costs. We test one thousand, two thousand and four thousand live keys, with n squared delete-insert pairs at each size.”

### Slide 2: interface, invariants and D3

“I am Mallhar, the Architect. I chose a shared adapter so both implementations follow the same contract. The simulator generates one trace and replays it across all four configurations. Each pair deletes a live key and inserts an absent key, restoring the population to n.

The shared interface supports updates, membership, ordered traversal, metrics and validation. We check strict ordering and reachable counts, plus the scapegoat height and size bookkeeping. Trace hashes identify common inputs. Root depth is zero.

My D3 derivation concerns a conceptual B-tree. Minimum occupancy at height h gives at least two times t to the power h, minus one, keys. Rearranging gives the logarithmic height bound. Splits and merges preserve equal leaf depth, so the bound continues to hold under churn. We analyze this property without implementing a B-tree.

Avinash will explain our first implemented structure and prove its deletion correctness.”

## Avinash — slides 3–4
### Slide 3: Hibbard deletion
“I am Avinash, and I worked on Structure A, the iterative Hibbard BST.

I chose iterative updates to tolerate deep chains without recursive call-stack growth. Insertion and lookup follow a single search path. Deletion depends on the number of children. A leaf can simply be removed. A node with one child is replaced by its child subtree. A node with two children needs a replacement key.

We implement two policies for that replacement. The first always takes the inorder successor, the smallest key in the right subtree. The second uses a separate seeded random generator to choose the predecessor or successor.

[Point to the left diagram, then the right.]

Here the original root is four. We delete four, copy its successor five into the root, then reinsert four. The reinserted key takes a deeper position. The contents return to the original set, but the shape changes. This example increases total node depth from ten to eleven. It illustrates a possible effect, rather than proving every update increases depth.

Each operation costs O of h. Without balancing, h can grow linearly with n. Next I will prove that these shape changes still preserve correctness.”

### Slide 4: D1 proof
“Our proof must show two things: the surviving tree remains ordered, and it contains exactly the original keys except the requested key x.

For a leaf, removing its incoming link changes no surviving ordering relation. For one child, we reconnect the entire child subtree. Its keys already satisfied every bound imposed by ancestors of x, so reconnecting it preserves those bounds.

For two children, write L and R for the left and right key sets. Let s be the minimum key in R. Originally every key a in L satisfies a less than x, and x is less than s. Therefore every key in L is smaller than the new root s. Since s is the minimum of R and keys are unique, every remaining key in R is larger than s.

[Point to the splicing argument.]

We also need to remove the old successor node safely. It cannot have a left child, because that child would contain a smaller key in R. Its possible right child already satisfies the bounds at that position, so we can splice that child into its place.

Finally, the new contents are L, the copied key s, and R without the old occurrence of s. This simplifies to L union R: exactly the original subtree with x removed. The replacement also stays within inherited ancestor bounds because s came from that subtree.

The predecessor case is symmetric. Since both choices are correct, every realized sequence of random choices also preserves order. Krish will now explain how Structure B controls height.”

## Krish — slides 5–6
### Slide 5: scapegoat operations and rebuilding
“I am Krish, and I worked on the scapegoat tree.

The balance parameter alpha is zero point seven. We keep the current size n and q, the maximum size since the latest whole-tree rebuild.

After inserting, we compare the new node’s depth with the logarithmic threshold based on q. If it exceeds the threshold, we walk upward to find an ancestor with a child larger than alpha times that ancestor’s subtree. We rebuild the selected subtree. If deletion makes n less than alpha times q, we rebuild the whole tree and reset q.

The array method offers straightforward median construction, while DSW permits in-place reconstruction. Comparing both separates the rebuild strategy from the trigger policy. Our array method collects nodes in inorder and recursively selects medians. DSW uses rotations to form a vine, then compresses it into a complete tree.

[Point along the vine and then to the rebuilt tree.]

Both methods preserve the inorder keys and take O of s time for s rebuilt nodes, although their resulting shapes can differ. The diagram illustrates rebuilding, rather than the insertion threshold for this small example.

The algorithm depends on an important claim: when insertion is too deep, there really is a scapegoat to find. My D2 proof establishes that claim.”

### Slide 6: D2 theorem and proof
“The theorem is: if a new leaf has depth d greater than log base one over alpha of n, some proper ancestor is not alpha-weight-balanced. Here n includes the newly inserted node, and alpha lies strictly between one half and one.

I will prove this by contradiction. Assume the insertion is too deep, but every proper ancestor on its path is balanced.

[Point to step one.]

Label the path from the root to the new leaf v zero through v d. Because each ancestor is balanced, the next child on the path has size at most alpha times its parent’s size.

[Point to step two and pause.]

Apply that inequality repeatedly. After one edge, size is at most alpha n. After two edges, it is at most alpha squared n. At depth d, it is at most alpha to the power d times n.

But the new node is a leaf, so its subtree size is exactly one. Therefore one is at most alpha to the power d times n.

[Point to step three.]

Rearranging gives one over alpha, raised to d, at most n. Because one over alpha is greater than one, taking logarithms preserves the inequality. We obtain d at most log base one over alpha of n.

This contradicts our starting assumption that d exceeds that value. Therefore at least one ancestor must be unbalanced: a scapegoat exists.

[Point to the implementation connection.]

Our code uses q, which is at least n. Its logarithmic threshold is therefore at least the threshold in the theorem. An integer depth beyond the floor of that threshold also exceeds the threshold itself. So whenever the implementation triggers its ancestor search, the theorem applies and guarantees a violating ancestor.

This proves that the search for a scapegoat is well defined. Aditya will later explain the amortized cost of rebuilding. First, Sai will cover correctness testing.”

## Sai — slide 7
“I am Sai, and I worked on the independent sorted-array oracle and failure reducer.

The sorted array keeps the reference independent of either tree algorithm. We compare tree updates, membership and size with the reference. Structural audits additionally check complete inorder contents, ordering and applicable bookkeeping.

Recorded testing passed one million mixed operations with workload seed 20261117. Full churn checks also passed one, four and sixteen million pairs at the required populations. Full structural audits run for the first sixty-four operations, every 4096th operation and the final state.

An intentionally incorrect size update in a disposable build reduced from eight operations to inserting seventeen and deleting seventeen. This demonstrates the reducer.

My D4 derivation gives a random-insertion baseline. Conditioning on the uniformly distributed root rank yields the recurrence shown. Solving it gives average depth two times one plus one over n, times the harmonic number, minus four. Its leading term is two natural log n. Exhaustive checks of insertion permutations through eight keys agree.

The baseline describes initialization; churn needs its own measurements. Mayank will explain the workloads.”

## Mayank — slide 8
“I am Mayank, and I worked on the adversarial generators and D5.

Ordinary churn deletes a live key and inserts a sampled absent key. Exact fit reinserts the same value that was deleted. Culberson and Munro’s square-root theorem concerns restricted exact fit with uniform deletion. Eppinger provides empirical motivation for ordinary churn.

Our targeted schedules prefer shallow two-child nodes in a separate successor-tree model. Shallow exact reinserts the same key; shallow fresh inserts an absent key. We generate the trace once for fair replay across configurations.

Successor replacement raises a dividing key. Repeating this near large subtrees can extend paths. At four thousand keys, the historical reference experiment produced successor depth about nine hundred sixty-three under shallow exact, versus about twenty-four under ordinary churn. These are five-trial results from the recorded reference implementation.

Biased deletion falls outside the theorem’s uniform-selection assumption. These finite results therefore cannot establish its asymptotic law.

We also tested five insertion pattern families. Ascending order gave the highest tested array rebuild frequency, and descending order the highest tested DSW frequency. These maxima apply to our tested candidates and event-count metric.

Aditya will compare predictions with the integrated measurements.”

## Aditya — slides 9–11
### Slide 9: D6 and predictions

“I am Aditya, and I worked on measurement and D6.

Rebuilding s nodes costs O of s, but the update bound is amortized O of log n. Our potential stores credit at nodes whose larger child grows too large. A rebuild clears this imbalance, releasing credit to pay for its work. Ordinary updates add potential along a logarithmic path. One update can still cost linear time.

The random-tree approximation predicted local event rates around two to three percent per pair. Observed rates were around zero point three five to zero point five three percent. Rebuilding changes the shape distribution and helps explain the overprediction. Predicted and observed deletion-triggered whole-tree rebuilds were both zero.

The prediction record postdates earlier reference exploration, and we retain that chronology.”

### Slide 10: depth trajectories

“These integrated runs use five trials at each size, identical inputs and n squared pairs per trial. Lines show means, and shaded bands show one sample standard deviation.

Successor-only depth rises at each size. At four thousand keys it finishes at about twenty-four, compared with about thirteen for randomized Hibbard and fourteen and a half for the scapegoat methods. The logarithm and square-root lines are reference values. Three finite populations cannot establish an asymptotic law.”

### Slide 11: timing and conclusions

“At four thousand keys, randomized Hibbard has the lowest recorded mean time, about seventy nanoseconds per operation. Successor Hibbard takes about seventy-nine, array scapegoat eighty-six and DSW eighty-two. The table includes sample standard deviations.

Timing covers update blocks and excludes setup, generation, validation and depth scans. The machine and compiler are shown here. No usable cache profile was collected.

Our results show that deletion policy affects depth. Random replacement helps in these trials, while scapegoat rebuilding provides height and amortized update guarantees. Rebuild work and allocation also affect runtime, so depth alone cannot determine speed.

Our conclusion is that a tree can preserve its contents while its shape becomes expensive to search. The replacement and rebuilding policies determine how it responds to churn.”

## Slide 12 — Full-scale demonstration and closing

[Aditya hands over to Mallhar. Perform this demonstration only once an approved `final` tag exists. It was absent at the last check. Clone into a fresh directory and show the tagged commit identity.]

“We are using a fresh clone of the final release. We build it using the documented command, then run all four configurations at one thousand, two thousand and four thousand keys, for one, four and sixteen million pairs respectively.

[Run the PowerShell commands on slide 12. Show the final checkpoint at each size.]

The trace hashes match across the four configurations, live population returns to n at pair boundaries, and the last checkpoint reaches n squared pairs. Average depth and rebuild counters describe how the strategies behave on the same input.

Each member can now answer questions about their implementation and derivation. Thank you.”

This is a demonstration script, not a claim that a fresh tagged full-scale run has already occurred. Stored plots cannot replace the required live run.

## Rehearsal notes

- The handout gives 15 minutes per team and also 3–4 minutes per member. This script fits within the 15-minute team limit.
- The three plots on slide 10 retain their full source figures. Open the PDF at full size for detailed legend inspection.
- Slide 8 distinguishes historical reference churn from upstream insertion measurements. Slides 10–11 use the integrated analyst study.
- Prepared with OpenAI Codex assistance from the project source, derivations and recorded measurements. Each speaker should understand their proof and rehearse the allotted time.


## Individual question preparation

- Mallhar: Why hold n fixed? Explain the shared interface and derive n ≥ 2t^h − 1 from minimum occupancy.
- Avinash: Why does the successor have no left child? Explain inherited bounds and why random replacement has no worst-case logarithmic guarantee.
- Krish: Derive 1 ≤ α^d n. Why does q ≥ n make the trigger safe? Distinguish existence of a scapegoat from amortized cost.
- Sai: Why is the root rank uniform? Explain the recurrence and why a matching key set does not certify a good shape.
- Mayank: Which assumption does shallow targeting violate? Distinguish exact fit from fresh-key churn and global optimum from strongest tested candidate.
- Aditya: Why did the random-shape event forecast overpredict? Explain the potential argument, uncertainty and timing exclusions.

Answers must be given by the individual presenter. The slide deck provides evidence and proof steps but cannot guarantee the individual Q&A marks.
