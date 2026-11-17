from pathlib import Path
import argparse
import json
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description='Verify the adversary module and optionally rerun the separate reference project.')
    parser.add_argument('--reference-root',type=Path)
    parser.add_argument('--project-root',type=Path,default=ROOT,help='Complete team checkout containing src/structB')
    args=parser.parse_args()
    subprocess.run([sys.executable,str(ROOT/'adversary/verify.py')],check=True)
    structure=args.project_root.resolve()/'src/structB'
    if not (structure/'scapegoat_tree.cpp').is_file():
        parser.error('Insertion measurements require --project-root pointing to the complete team checkout. Standalone verification has passed.')
    work=ROOT/'adversary/.work'
    exe=work/'measure_rebuilds.exe'
    subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Wpedantic',
                    '-I'+str(structure),str(structure/'scapegoat_tree.cpp'),
                    str(ROOT/'adversary/measure_rebuilds.cpp'),'-o',str(exe)],check=True)
    raw=[]
    for n in [1000,2000,4000]:
        for trial in range(1,6):
            seed=202611160+n*31+trial
            path=work/f'insertions-upstream-{n}-{trial}.csv'
            with path.open('w') as output:
                subprocess.run([str(exe),str(n),str(trial),str(seed),'0.7'],stdout=output,check=True)
            raw.append(str(path))
    print('Reproduced teammate-current Scapegoat insertion experiments:',len(raw),'runs')
    if args.reference_root:
        reference=args.reference_root.resolve()
        if not (reference/'scripts/run_all.py').is_file():
            raise RuntimeError('reference root must name the separate complete general-folder project')
        subprocess.run([sys.executable,str(reference/'scripts/run_all.py'),'--full'],cwd=reference,check=True)
    else:
        print('Full churn measurements used the separately staged reference project; pass --reference-root to rerun it.')
        print('Published churn timings use the recorded reference snapshot. See INTEGRATION.md for compatibility with main.')

if __name__=='__main__': main()
