# Recorded measurements

- `raw/adversary/`: original 60 reference churn runs and 30 insertion runs;
  `interrupted/` holds the incomplete run, excluded from plots and summaries.
- `raw/analyst/`: integrated four-strategy study, 15 trials in one checkpoint CSV.
- `raw/analyst-replay/`: separately recorded rerun and its final temporary trial.
- `raw/smoke/`: exploratory short runs, not five-trial evidence.
- `summary/`: submitted aggregates kept with their originating study.
- `figures/`: plots grouped by study; regenerate with `python scripts/plot_results.py`.
- `metadata/adversary/`: original manifests, source hashes and machine records.

The migration preserved recorded CSV bytes and their seeds. Historical manifests
retain their original paths and source revisions; `metadata/path-migration.json`
maps those repository paths to the current layout. They describe the original
measurement snapshot, not a new measurement of this restructured checkout.

`python scripts/run_all.py` runs new experiments in timestamped `raw/run-*`
directories. Reproducing historical reference timings requires the separately
recorded reference implementation; rebuilding figures from the stored raw CSVs
does not require it. Timing values are not expected to be identical on a new run.
