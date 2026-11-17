# Mayank Moyal — Adversary contribution

CS 253 / MA 253, Project 3, Team Underqualified. Roll number 250041027.

This branch contains Mayank's adversarial workload generators, validation,
recorded adversary measurements, D5 derivation, and the updated team slides.
The complete integrated project remains on `main`.

- `adversary/`: generators, standalone verification, measurement drivers and records.
- `docs/derivations/D5_250041027.pdf`: D5 derivation, with Markdown source beside it.
- `results/{raw,summary,figures,metadata}/adversary/`: recorded evidence.
- `docs/slides/`: latest 12-slide PowerPoint, PDF and speaking script.

Run standalone checks with `python adversary/verify.py` (Python 3 and g++ C++17).
See `TESTING.md` for optional measurement integration.

The slides give Avinash and Krish expanded proof sections, omit the live demo,
and end with Thank You.

This contribution-only branch is for individual review. Its scope-cleanup
commit removes team files from this branch and should not be merged wholesale
into main. To update main's presentation, copy or cherry-pick only the slides.
