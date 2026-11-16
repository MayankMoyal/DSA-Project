# Build and analysis tools

Original papers by Eppinger, Culberson-Munro, Galperin-Rivest and Stout-Warren were consulted. g++ compiled the programs; C++ runs produced the raw measurements. Python orchestrated experiments and computed means/sample SD; Matplotlib generated figures; ReportLab generated PDFs; PyMuPDF rendered and checked the PDFs. Git records the contribution history.

OpenAI Codex assisted with code, test scripts, documentation and reference experiment preparation. Subsequent verification corrected the depth-limited target selection and clarified the limits of the measured reference results. During PR preparation on 17 November 2026, Codex revised numeric argument validation, added regression coverage, formatted the touched code and updated this disclosure. See the root [TOOLS.md](../TOOLS.md) for the current review scope. Teammate review remains pending.

Verification corrected target selection to enforce the documented depth limit. The separate reference version also changes teammates' draft modules, including perfect-leaves rebuilding. Those reference changes are not published here, are not attributed to teammates and require their review before adoption.
