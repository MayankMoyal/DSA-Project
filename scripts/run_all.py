"""Build and verify the checkout and reproduce its runnable experiments."""

import argparse
from datetime import datetime
import os
from pathlib import Path
import shutil
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / ".build"


def run(*command, cwd=ROOT, stdout=None):
    print("+", subprocess.list2cmdline([str(arg) for arg in command]), flush=True)
    subprocess.run(command, cwd=cwd, stdout=stdout, check=True)


def save_csv(path, *command):
    with path.open("wb") as output:
        run(*command, stdout=output)


def build():
    options = ["-DCMAKE_BUILD_TYPE=Release"]
    if not (BUILD / "CMakeCache.txt").exists() and os.name == "nt":
        options.extend(("-G", "MinGW Makefiles"))
    run("cmake", "-S", ROOT, "-B", BUILD, *options)
    run("cmake", "--build", BUILD, "--parallel")


def executable(name):
    return BUILD / "bin" / (name + (".exe" if os.name == "nt" else ""))


def experiments(quick):
    run_id = datetime.now().strftime("run-%Y%m%d-%H%M%S-%f")
    raw = ROOT / "results" / "raw" / run_id
    raw.mkdir(parents=True)
    save_csv(raw / "bst-baseline.csv", executable("bst_baseline"))

    for n in ((100,) if quick else (1000, 2000, 4000)):
        pairs = n * n
        save_csv(raw / f"churn-{n}.csv", executable("churn_simulator"),
                 str(n), str(pairs), "42", "random", str(pairs // 100))

    run(sys.executable, ROOT / "adversary" / "reproduce.py")
    for csv in sorted((ROOT / "adversary" / ".work").glob("insertions-upstream-*.csv")):
        shutil.copy2(csv, raw / csv.name)

    if (ROOT / "bench" / "churn_benchmark.cpp").is_file():
        run(executable("predict_rate"))
        if quick:
            save_csv(raw / "analyst-quick.csv", executable("churn_benchmark"), "--quick")
        else:
            study = raw / "analyst"
            run(executable("analyst_study"), study, cwd=executable("analyst_study").parent)
            (study / executable("churn_benchmark").name).unlink()
            (study / "trial.csv").unlink()
            figures = ROOT / "results" / "figures" / run_id
            figures.mkdir(parents=True)
            for figure in sorted(study.glob("depth-*.svg")):
                shutil.move(figure, figures / figure.name)
            print(f"Analyst figures: {figures}")
    else:
        print("Analyst churn benchmark and figures pending team integration.")
    print(f"Experiment CSVs: {raw}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    actions = parser.add_mutually_exclusive_group()
    actions.add_argument("--build-only", action="store_true")
    actions.add_argument("--tests-only", action="store_true")
    actions.add_argument("--quick", action="store_true", help="Use a smaller churn demonstration.")
    args = parser.parse_args()

    build()
    if args.build_only:
        return
    run("ctest", "--output-on-failure", cwd=BUILD)
    if not args.tests_only:
        experiments(args.quick)


if __name__ == "__main__":
    try:
        main()
    except (OSError, subprocess.CalledProcessError) as error:
        sys.exit(f"Project run failed: {error}")
