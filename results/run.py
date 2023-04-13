import argparse
import multiprocessing
import json
import os
import re
import shutil
import subprocess
import tempfile
from contextlib import contextmanager
from pathlib import Path
from typing import Generator, List

@contextmanager
def temporary_file() -> Generator[Path, None, None]:
    fd, name = tempfile.mkstemp()
    os.close(fd)

    path = Path(name)

    try:
        yield path
    finally:
        path.unlink()

@contextmanager
def temporary_directory() -> Generator[Path, None, None]:
    path = Path(tempfile.mkdtemp())

    try:
        yield path
    finally:
        shutil.rmtree(path)

def update_overview() -> None:
    scores_by_solver = {}
    outputs_by_solver = {}
    results_root = Path(__file__).parent / "out"

    for directory in results_root.iterdir():
        scores_by_seed = {}
        outputs_by_seed = {}

        for file in directory.iterdir():
            if file.name.endswith(".txt"):
                scores_by_seed[file.stem] = int(file.read_text(encoding="utf-8").strip())
                outputs_by_seed[file.stem] = (file.parent / f"{file.stem}.out").read_text(encoding="utf-8")

        scores_by_solver[directory.name] = scores_by_seed
        outputs_by_solver[directory.name] = outputs_by_seed

    overview_template_file = Path(__file__).parent / "overview.tmpl.html"
    overview_file = Path(__file__).parent / "overview.html"

    overview = overview_template_file.read_text(encoding="utf-8")
    overview = overview.replace("/* scores_by_solver */{}", json.dumps(scores_by_solver))
    overview = overview.replace("/* outputs_by_solver */{}", json.dumps(outputs_by_solver))

    with overview_file.open("w+", encoding="utf-8") as file:
        file.write(overview)

    print(f"Overview: file://{overview_file}")

def generate_seed(seed: int) -> None:
    seed_file = Path(__file__).parent / "in" / f"{seed}.txt"
    if seed_file.is_file():
        return

    if not seed_file.parent.is_dir():
        seed_file.parent.mkdir(parents=True)

    with temporary_file() as tmp_file, temporary_directory() as tmp_directory:
        tmp_file.write_text(f"{seed}\n", encoding="utf-8")

        process = subprocess.run([str(Path(__file__).parent / "gen"), str(tmp_file)],
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT,
                                 cwd=str(tmp_directory))

        if process.returncode != 0:
            raise RuntimeError(f"Generator exited with status code {process.returncode} for seed {seed}:\n{process.stdout.decode('utf-8').strip()}")

        seed_file.write_text((tmp_directory / "in" / "0000.txt").read_text(encoding="utf-8"), encoding="utf-8")

def run_seed(solver: Path, seed: int, results_directory: Path) -> int:
    with (results_directory / f"{seed}.out").open("w+", encoding="utf-8") as out_file, \
         (results_directory / f"{seed}.err").open("w+", encoding="utf-8") as err_file:
        input = (Path(__file__).parent / "in" / f"{seed}.txt").read_text(encoding="utf-8").encode("utf-8")

        try:
            solver_process = subprocess.run([str(solver)], input=input, stdout=out_file, stderr=err_file, timeout=2)
        except subprocess.TimeoutExpired:
            raise RuntimeError(f"Solver timed out for seed {seed}")

        if solver_process.returncode != 0:
            raise RuntimeError(f"Solver exited with status code {solver_process.returncode} for seed {seed}")

    with temporary_directory() as tmp_directory:
        vis_process = subprocess.run([
            str(Path(__file__).parent / "vis"),
            str(Path(__file__).parent / "in" / f"{seed}.txt"),
            str((results_directory / f"{seed}.out"))
        ], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=tmp_directory)

        if vis_process.returncode != 0:
            raise RuntimeError(f"Visualizer exited with status code {solver_process.returncode} for seed {seed}:\n{vis_process.stdout.decode('utf-8').strip()}")

        return int(vis_process.stdout.decode("utf-8").split("Score = ")[1])

def run(solver: Path, seeds: List[int], results_directory: Path) -> None:
    if not results_directory.is_dir():
        results_directory.mkdir(parents=True)

    with multiprocessing.Pool(multiprocessing.cpu_count() - 2) as pool:
        pool.map(generate_seed, seeds)

        scores = pool.starmap(run_seed, [(solver, seed, results_directory) for seed in seeds])

        for seed, score in zip(seeds, scores):
            print(f"{seed}: {score:,.0f}")

            with (results_directory / f"{seed}.txt").open("w+", encoding="utf-8") as score_file:
                score_file.write(str(score))

        if len(seeds) > 0:
            print(f"Total score: {sum(scores):,.0f}")

def main() -> None:
    parser = argparse.ArgumentParser(description="Run a solver.")
    parser.add_argument("solver", type=str, help="the solver to run")
    parser.add_argument("--seed", type=int, help="the seed to run (defaults to 1-100)")

    args = parser.parse_args()

    solver = Path(__file__).parent.parent / "cmake-build-release" / args.solver
    if not solver.is_file():
        raise RuntimeError(f"Solver not found, {solver} is not a file")

    results_directory = Path(__file__).parent / "out" / args.solver

    if args.seed is None:
        run(solver, list(range(1, 101)), results_directory)
    else:
        run(solver, [args.seed], results_directory)

    update_overview()

if __name__ == "__main__":
    main()
