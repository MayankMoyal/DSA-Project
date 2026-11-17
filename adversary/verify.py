from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
MODES = ("random", "exact", "shallow_exact", "shallow_fresh")


def verify():
    output = ROOT / "adversary/.work"
    output.mkdir(parents=True, exist_ok=True)
    names = (
        "foundation", "generate_baseline", "generate_degradation",
        "generate_insertions", "tests", "validate_trace",
    )
    for name in names:
        subprocess.run(
            ["g++", "-std=c++17", "-O2", "-Wall", "-Wextra", "-Wpedantic",
             str(ROOT / f"adversary/{name}.cpp"), "-o", str(output / f"{name}.exe")],
            check=True,
        )
    subprocess.run([str(output / "tests.exe")], check=True)

    def call(name, args, success=True, error_label=None):
        run = subprocess.run(
            [str(output / f"{name}.exe"), *map(str, args)],
            capture_output=True, text=True, timeout=10,
        )
        if (run.returncode == 0) != success:
            raise RuntimeError(f"{name} unexpected exit {run.returncode}: {run.stderr}")
        if error_label is not None and (run.stdout or error_label not in run.stderr):
            raise RuntimeError(f"{name} invalid-argument output: {args!r}: {run!r}")
        return run.stdout

    for mode in MODES:
        name = "generate_baseline" if mode in MODES[:2] else "generate_degradation"
        data = call(name, [31, 10000, 42, mode])
        if data != call(name, [31, 10000, 42, mode]):
            raise RuntimeError(f"{mode} trace is not reproducible")
        with tempfile.TemporaryDirectory(dir=output) as temp:
            path = Path(temp) / "trace.txt"
            path.write_text(data, encoding="ascii")
            print(call("validate_trace", [31, 10000, path]).strip())
            path.write_text(data + "I 999\n", encoding="ascii")
            call("validate_trace", [31, 10000, path], False)
            path.write_text("I 1\nD 1\nI 1\n", encoding="ascii")
            call("validate_trace", [31, 2, path], False)

    for name, args in (
        ("foundation", [0, 42]),
        ("generate_baseline", [10, 1, 42, "bad"]),
        ("generate_degradation", [10, 1, 42, "bad"]),
        ("generate_insertions", [10, 42, "bad"]),
    ):
        call(name, args, False)

    with tempfile.TemporaryDirectory(dir=output) as temp:
        trace = Path(temp) / "setup.trace"
        trace.write_text(call("foundation", [3, 0]), encoding="ascii")
        tools = (
            ("foundation", [3, 0], ((0, "n"), (1, "seed"))),
            ("generate_baseline", [3, 0, 0, "random"], ((0, "n"), (1, "pairs"), (2, "seed"))),
            ("generate_degradation", [3, 0, 0, "shallow_exact"], ((0, "n"), (1, "pairs"), (2, "seed"))),
            ("generate_insertions", [3, 0, "shuffle"], ((0, "n"), (1, "seed"))),
            ("validate_trace", [3, 0, trace], ((0, "n"), (1, "pairs"))),
        )
        invalid = ("", "-1", "+1", " 1", "1 ", "1junk", "1.5", "1\t", "18446744073709551616")
        for name, args, positions in tools:
            expected = call(name, args)
            for index, label in positions:
                for value in invalid + (("0", "1000001") if label == "n" else ()):
                    bad = list(args)
                    bad[index] = value
                    call(name, bad, False, error_label=label)
                padded = list(args)
                padded[index] = "000" + str(args[index])
                if call(name, padded) != expected:
                    raise RuntimeError(f"{name} leading zeroes changed output")
                if label == "seed":
                    valid = list(args)
                    valid[index] = "18446744073709551615"
                    call(name, valid)
        for mode in MODES:
            tool = "generate_baseline" if mode in MODES[:2] else "generate_degradation"
            for pairs in (0, 1):
                trace.write_text(
                    call(tool, [3, pairs, "18446744073709551615", mode]),
                    encoding="ascii",
                )
                call("validate_trace", [3, pairs, trace])
    print("Strict numeric CLI checks passed: malformed values, population bounds, "
          "leading zeroes, zero pairs and full-width seeds")
    print("Standalone generator, CLI, replay validation, reproducibility "
          "and malformed-trace checks passed")


if __name__ == "__main__":
    verify()
