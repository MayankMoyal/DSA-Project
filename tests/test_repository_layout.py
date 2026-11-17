"""Check submission paths, document links and recorded CSV integrity."""
import hashlib
import json
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1]


class RepositoryLayout(unittest.TestCase):
    def test_required_paths(self):
        for path in ('README.md','REPORT.md','REPORT.pdf','INTERFACE.md','TOOLS.md',
                     'CMakeLists.txt','scripts/run_all.sh','scripts/plot_results.py',
                     'src/structA','src/structB','include','app','oracle','adversary',
                     'bench','tests','results/raw','results/figures','docs/slides'):
            self.assertTrue((ROOT/path).exists(),path)
        rolls=('250041006','250041022','250041026','250041007','250041027','250001005')
        for number,roll in enumerate(rolls,1):
            self.assertTrue((ROOT/f'docs/derivations/D{number}_{roll}.pdf').is_file())

    def test_markdown_links(self):
        for path in ROOT.rglob('*.md'):
            if any(p in ('.git','.build','.work','build') for p in path.relative_to(ROOT).parts):
                continue
            for link in re.findall(r'\]\(([^)]+)\)',path.read_text(encoding='utf-8-sig')):
                if '://' in link or link.startswith('#'):
                    continue
                self.assertTrue((path.parent/link.split('#')[0]).exists(),f'{path}: {link}')

    def test_recorded_raw_hashes(self):
        manifest=json.loads((ROOT/'results/metadata/adversary/experiment-manifest.json').read_text())
        checked=0
        for run in manifest['runs']:
            if 'sha256' not in run:
                continue
            name=run['file'].replace('\\','/').split('/')[-1]
            actual=hashlib.sha256((ROOT/'results/raw/adversary'/name).read_bytes()).hexdigest()
            self.assertEqual(actual,run['sha256'],name)
            checked+=1
        self.assertEqual(checked,90)


if __name__=='__main__':
    unittest.main()
