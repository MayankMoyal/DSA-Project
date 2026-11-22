# Final submission checklist

Repository restructuring completed locally on 17 November 2026 for Team Underqualified.
This list distinguishes available artifacts from items still needing team action.

## Available

- Required module folders, shared interface, build configuration and run_all.sh.
- README sections in the supplied order, team table, concrete CLI example and seeds.
- REPORT.md organized into the 13 requested sections and matching REPORT.pdf export.
- Six derivation PDFs named D<number>_<roll>.pdf; original authored contents retained.
- Raw measurements, summaries, figures and source/machine metadata in results/.
- scripts/plot_results.py regenerates all 19 recorded figures from raw CSVs.
- Tracked build executables/debug symbols removed; ignore rules prevent reintroduction.
- Final 12-slide presentation supplied as [PowerPoint](slides/slides.pptx) and [PDF](slides/slides.pdf).

## Before final tagging

- Confirm all names, roll numbers and account names in the team table.
- Authors must check each derivation is one to two pages, includes roll number,
  and identifies the code/measurement use. Renaming does not repair PDF contents.
- Each member must complete their own tool versions, dates and representative
  prompts in TOOLS.md; no missing records have been invented.
- Check the instructor's hidden-test input/output specification and update the
  interface if necessary. Current README documents the implemented CLI only.
- Expand and review the concise structured report against the 12-20-page content guidance, then regenerate REPORT.pdf. Do not pad pages to meet a length target.
- D5 currently has 3 pages and D6 has 4; their authors must shorten them to the requested 1-2 pages.
- Rehearse from a fresh clone of the final tagged commit, with documented dependencies.
- Confirm private repository access, instructor access, release tag and submission
  details with the team. No visibility, collaborator or tag changes were made here.
- Retain honest prediction chronology and reference implementation labels.

Historical manifests retain original paths. See results/metadata/path-migration.json.
The report's historical benchmark timings have not been remeasured during this
layout change. New smoke runs are verification, not replacement performance data.
