from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]

def verify():
    output=ROOT/'adversary/.work'
    output.mkdir(parents=True,exist_ok=True)
    names=['foundation','generate_baseline','generate_degradation','generate_insertions','tests','validate_trace']
    for name in names:
        subprocess.run(['g++','-std=c++17','-O2','-Wall','-Wextra','-Wpedantic',
                        str(ROOT/f'adversary/{name}.cpp'),'-o',str(output/(name+'.exe'))],check=True)
    subprocess.run([str(output/'tests.exe')],check=True)
    def call(name,args,success=True):
        run=subprocess.run([str(output/(name+'.exe')),*map(str,args)],capture_output=True,text=True)
        if (run.returncode==0)!=success:
            raise RuntimeError(f'{name} unexpected exit {run.returncode}: {run.stderr}')
        return run.stdout
    for mode in ['random','exact','shallow_exact','shallow_fresh']:
        name='generate_baseline' if mode in ['random','exact'] else 'generate_degradation'
        data=call(name,[31,10000,42,mode])
        assert data==call(name,[31,10000,42,mode])
        with tempfile.TemporaryDirectory(dir=output) as temp:
            path=Path(temp)/'trace.txt'; path.write_text(data)
            print(call('validate_trace',[31,10000,path]).strip())
            path.write_text(data+'I 999\n')
            call('validate_trace',[31,10000,path],False)
            path.write_text('I 1\nD 1\nI 1\n')
            call('validate_trace',[31,2,path],False)
    for name,args in [('foundation',[0,42]),('generate_baseline',[10,1,42,'bad']),
                      ('generate_degradation',[10,1,42,'bad']),('generate_insertions',[10,42,'bad'])]:
        call(name,args,False)
    print('Standalone generator, CLI, replay validation, reproducibility and malformed-trace checks passed')

if __name__=='__main__': verify()
