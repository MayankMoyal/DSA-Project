# Live churn dashboard

From the repository root:

```powershell
python scripts/run_all.py --build-only
python frontend/serve.py
```

Open http://127.0.0.1:8766. Select population, workload, pair count and seed, then run. The dashboard streams validated C++ simulator checkpoints, displays depth/height/rebuild metrics, and exports CSV. Stop cancels the request and terminates the subprocess at the next checkpoint. Small runs may finish immediately. Full-scale runs use 1,000/2,000/4,000 keys and n squared pairs. The chart shows aggregate metrics, not individual tree nodes.

The small-tree panel uses snapshots from tree_demo, linked to the same C++ tree implementations. Next/Back/Play/Reset show deletion and reinsertion of 4, then 5 and 6. Sorted insertion of keys 1–15 triggers Scapegoat rebuilds. Each frame validates the live set and invariants. Animated movement interpolates between complete operation states; it does not expose individual internal rotations.
