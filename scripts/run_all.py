"""Build, verify and reproduce the DSA project experiments."""
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
    # subprocess runs another program (CMake, C++ executable, Python, etc.).
    arguments = []
    for argument in command:
        arguments.append(str(argument))
    print("+", subprocess.list2cmdline(arguments), flush=True)
    subprocess.run(arguments, cwd=cwd, stdout=stdout, check=True)


def save_csv(path, *command):
    # Redirect a program's standard output into a new CSV file.
    with path.open("wb") as output:
        run(*command, stdout=output)


def build():
    options = ["-DCMAKE_BUILD_TYPE=Release"]
    cache_exists = (BUILD / "CMakeCache.txt").exists()
    if not cache_exists and os.name == "nt":
        options.extend(["-G", "MinGW Makefiles"])
    run("cmake", "-S", ROOT, "-B", BUILD, *options)
    run("cmake", "--build", BUILD, "--parallel")


def executable(name):
    if os.name == "nt":
        name = name + ".exe"
    return BUILD / "bin" / name


def run_simulator(raw_dir, quick):
    if quick:
        sizes = [100]
    else:
        sizes = [1000, 2000, 4000]
    for n in sizes:
        pairs = n * n
        checkpoint = pairs // 100
        output = raw_dir / f"churn-{n}.csv"
        run_program = executable("churn_simulator")
        save_csv(output, run_program, n, pairs, 42, "random", checkpoint)


def run_adversary(raw_dir):
    # Reuse the teammate's existing insertion experiments.
    run(sys.executable, ROOT / "adversary" / "reproduce.py")
    work_dir = ROOT / "adversary" / ".work"
    for csv_path in sorted(work_dir.glob("insertions-upstream-*.csv")):
        shutil.copy2(csv_path, raw_dir / csv_path.name)


def run_analyst(raw_dir, run_id, quick):
    source = ROOT / "bench" / "churn_benchmark.cpp"
    if not source.is_file():
        print("Analyst churn benchmark and figures pending team integration.")
        return
    run(executable("predict_rate"))
    if quick:
        save_csv(raw_dir / "analyst-quick.csv", executable("churn_benchmark"), "--quick")
        return

    # The complete five-trial study produces raw.csv, summary.csv and graphs.
    study_dir = raw_dir / "analyst"
    program = executable("analyst_study")
    run(program, study_dir, cwd=program.parent)

    # Remove only the temporary copy and trial CSV from this new study folder.
    copied_benchmark = study_dir / executable("churn_benchmark").name
    copied_benchmark.unlink()
    (study_dir / "trial.csv").unlink()

    figures = ROOT / "results" / "figures" / run_id
    figures.mkdir(parents=True)
    for figure in sorted(study_dir.glob("depth-*.svg")):
        shutil.move(figure, figures / figure.name)
    print(f"Analyst figures: {figures}")


def experiments(quick):
    # Each new study gets a new directory: preserve the submitted CSVs.
    run_id = datetime.now().strftime("run-%Y%m%d-%H%M%S-%f")
    raw_dir = ROOT / "results" / "raw" / run_id
    raw_dir.mkdir(parents=True)
    save_csv(raw_dir / "bst-baseline.csv", executable("bst_baseline"))
    run_simulator(raw_dir, quick)
    run_adversary(raw_dir)
    run_analyst(raw_dir, run_id, quick)
    print(f"Experiment CSVs: {raw_dir}")


def plot_recorded_results():
    run(sys.executable, ROOT / "scripts" / "plot_results.py")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    actions = parser.add_mutually_exclusive_group()
    actions.add_argument("--build-only", action="store_true")
    actions.add_argument("--tests-only", action="store_true")
    actions.add_argument("--figures-only", action="store_true",
                         help="Regenerate recorded figures without building or measuring.")
    actions.add_argument("--quick", action="store_true",
                         help="Use a smaller churn demonstration.")
    args = parser.parse_args()

    if args.figures_only:
        plot_recorded_results()
        return
    build()
    if args.build_only:
        return
    run("ctest", "--output-on-failure", cwd=BUILD)
    if args.tests_only:
        return
    experiments(args.quick)
    plot_recorded_results()


if __name__ == "__main__":
    try:
        main()
    except (OSError, subprocess.CalledProcessError) as error:
        sys.exit(f"Project run failed: {error}")
