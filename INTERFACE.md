
# Tree Interface and Invariants

## Project 3: Search Trees Under Long-Running Churn

**Owner:** Architect  
**Status:** Initial interface proposal; pending integration with structure owners.

## 1. Objective

The application compares the effect of long-running insertion and deletion churn on:

- Structure A: Hibbard Binary Search Tree
- Structure B: Scapegoat Tree

Both structures must execute equivalent operation sequences so that their average node depths can be compared fairly.

## 2. Proposed Common Operations

| Operation | Description |
|---|---|
| `insert(key)` | Insert a key if absent |
| `erase(key)` | Delete a key if present |
| `contains(key)` | Check whether a key exists |
| `size()` | Return the number of keys |
| `averageDepth()` | Return the average depth of all nodes |

Proposed operation semantics:

- Keys are signed 64-bit integers.
- Duplicate keys are not stored.
- `insert` returns true only when a new key is inserted.
- `erase` returns true only when an existing key is removed.
- `contains` returns true only if the key exists.
- Root depth is defined as zero.
- Average depth is the sum of all node depths divided by the number of nodes.
- Average depth of an empty tree is defined as zero for application reporting.

These are target semantics. Existing structure implementations may require adapters or interface updates.

## 3. Structural Invariants

### Common BST Invariants

1. Every key in a node's left subtree is smaller than its key.
2. Every key in a node's right subtree is larger than its key.
3. The reported size equals the actual number of nodes.
4. An inorder traversal produces keys in strictly increasing order.

### Structure A

- Uses iterative BST operations.
- Supports always-successor Hibbard deletion.
- Supports randomized predecessor/successor deletion.
- Does not automatically rebalance the tree.

### Structure B

- Uses Scapegoat Tree balancing.
- Rebuilds an unbalanced ancestor after sufficiently deep insertion.
- Rebuilds the whole tree after sufficient deletion-driven shrinkage.
- Supports both array-based and Stout-Warren rebuilding.

## 4. Churn Simulator Contract

The simulator maintains a fixed tree size `n`.

For each churn pair:

1. Select a key currently present and delete it.
2. Select a key currently absent and insert it.
3. Verify that the final size is `n`.
4. Record average node depth at predetermined checkpoints.

Both tree structures must receive the same initial key set and equivalent operation sequence.

Random workloads must use recorded seeds so experiments are reproducible.

## 5. Required Experiments

| n | Minimum churn pairs |
|---|---|
| 1000 | 1,000,000 |
| 2000 | 4,000,000 |
| 4000 | 16,000,000 |

The results must allow comparison of:

- Always-successor Hibbard deletion
- Randomized predecessor/successor Hibbard deletion
- Scapegoat Tree with array rebuilding
- Scapegoat Tree with Stout-Warren rebuilding

The benchmark module will plot average depth against churn pairs, including logarithmic and square-root reference curves.

## 6. Integration Notes

The current implementations do not yet expose identical APIs.

Structure owners must agree on common insertion, deletion, and measurement semantics before integration.

The architect owns the application in `app/` and coordinates this interface with the structure, oracle, adversary, and benchmark owners.
