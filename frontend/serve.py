"""Local live dashboard: python frontend/serve.py, then http://127.0.0.1:8766."""
import csv
import json
import subprocess
import tempfile
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlsplit

ROOT = Path(__file__).resolve().parents[1]
BINARY = ROOT / '.build/bin/churn_simulator.exe'
if not BINARY.exists():
    BINARY = ROOT / '.build/bin/churn_simulator'

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        url = urlsplit(self.path)
        if url.path == '/':
            body = (ROOT / 'frontend/index.html').read_bytes()
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.end_headers()
            self.wfile.write(body)
            return
        if url.path == '/demo':
            mode = parse_qs(url.query).get('mode', ['delete'])[0]
            if mode not in ('delete', 'rebuild'):
                self.send_error(400, 'Unknown demonstration')
                return
            binary = BINARY.with_name('tree_demo' + BINARY.suffix)
            try:
                result = subprocess.run([str(binary), mode], capture_output=True, text=True, check=True, timeout=10)
                body = json.dumps([json.loads(line) for line in result.stdout.splitlines()]).encode()
            except (OSError, subprocess.SubprocessError, ValueError):
                self.send_error(500, 'Build tree_demo before opening the demonstration')
                return
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(body)
            return
        if url.path != '/run':
            self.send_error(404)
            return
        try:
            query = parse_qs(url.query)
            n = int(query.get('n', ['1000'])[0])
            pairs = int(query.get('pairs', ['1000000'])[0])
            seed = int(query.get('seed', ['42'])[0])
            mode = query.get('mode', ['random'])[0]
            if not (1 <= n <= 4000 and 1 <= pairs <= 16000000 and 0 <= seed < 2**64):
                raise ValueError('Choose 1–4000 keys, 1–16000000 pairs and a uint64 seed.')
            if mode not in ('random', 'exact', 'shallow_exact', 'shallow_fresh'):
                raise ValueError('Unknown workload')
            if not BINARY.is_file():
                raise ValueError('Build first with python scripts/run_all.py --build-only')
        except (ValueError, IndexError) as error:
            self.send_error(400, str(error))
            return
        self.send_response(200)
        self.send_header('Content-Type', 'application/x-ndjson')
        self.send_header('Cache-Control', 'no-store')
        self.end_headers()
        process = None
        try:
            with tempfile.TemporaryFile(mode='w+t') as errors:
                process = subprocess.Popen([str(BINARY), str(n), str(pairs), str(seed), mode,
                    str(max(1, pairs // 200))], stdout=subprocess.PIPE, stderr=errors, text=True)
                for row in csv.DictReader(process.stdout):
                    self.wfile.write((json.dumps(row) + '\n').encode())
                    self.wfile.flush()
                code = process.wait()
                errors.seek(0)
                result = {'done': True} if code == 0 else {'error': errors.read() or 'Simulator failed'}
                self.wfile.write((json.dumps(result) + '\n').encode())
        except (BrokenPipeError, ConnectionResetError):
            pass
        finally:
            if process is not None:
                if process.poll() is None:
                    process.terminate()
                process.wait()
                process.stdout.close()

if __name__ == '__main__':
    print('Open http://127.0.0.1:8766', flush=True)
    ThreadingHTTPServer(('127.0.0.1', 8766), Handler).serve_forever()
