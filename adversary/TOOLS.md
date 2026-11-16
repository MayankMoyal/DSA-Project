# Tool disclosure for Mayank's contribution

ChatGPT/Codex generated the workload contract, SplitMix64 sampling utilities, baseline/targeted/insertion generators, shape model used for data generation, validation tests, integration and reproduction helpers, measurement driver, documentation, D5 draft and result-analysis material during the 16 November 2026 session.

Original papers by Eppinger, Culberson-Munro, Galperin-Rivest and Stout-Warren were consulted. g++ compiled the programs; C++ runs produced the raw measurements. Python orchestrated experiments and computed means/sample SD; Matplotlib generated figures; ReportLab generated PDFs; PyMuPDF rendered and checked the PDFs. Git/GitHub were used for the authorized delivery: two initial commits followed by one unified commit for the four remaining groups, as Mayank requested.

Verification corrected target selection to enforce the documented depth limit. The separate reference version also changes teammates' draft modules, including perfect-leaves rebuilding. Those reference changes are not published here, are not attributed to teammates and require their review before adoption.

Mayank requested the contribution but has not yet reviewed or explained every generated line. He should update this disclosure with his actual subsequent edits and review. No manual authorship, teammate approval, exhaustive correctness, cache effect or global adversarial optimality is claimed. The publication schedule must not be presented as independent development history.
