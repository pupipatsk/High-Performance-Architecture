import subprocess
import time
import statistics
from pathlib import Path
from typing import List, Optional
import random

# === CONFIGURATION ===
DOCKER_IMAGE = "power-plant-solver"
INPUT_DIR = Path("data/input")
OUTPUT_DIR = Path("data/output")
RUNS = 5
TEST_LIMIT = 15


def run_docker(input_file: str, output_file: str) -> bool:
    """Run Docker container and return success status."""
    command = [
        "docker",
        "run",
        "--rm",
        "-v",
        f"{INPUT_DIR.resolve()}:/input",
        "-v",
        f"{OUTPUT_DIR.resolve()}:/output",
        DOCKER_IMAGE,
        f"/input/{input_file}",
        f"/output/{output_file}",
    ]
    result = subprocess.run(
        command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
    )
    return result.returncode == 0


def run_single_file(input_path: Path) -> Optional[float]:
    """Run a single input file and return duration, or None if it fails."""
    output_path = input_path.with_suffix(".out").name
    start = time.perf_counter()
    success = run_docker(input_path.name, output_path)
    end = time.perf_counter()

    if not success:
        print(f"Error: Failed on {input_path.name}")
        return None
    return end - start


def run_single_set(files: List[Path]) -> float:
    """Run a single full benchmark set. Return total duration or -1 on failure."""
    start = time.perf_counter()
    for file in files:
        output_name = file.with_suffix(".out").name
        if not run_docker(file.name, output_name):
            print(f"Error: Failed on {file.name}")
            return -1
    end = time.perf_counter()
    return end - start


def sample_files(limit: int = TEST_LIMIT, seed: int = 42) -> List[Path]:
    """
    Randomly sample input files with a fixed seed to ensure reproducibility.
    Returns a list of Path objects up to the specified limit.
    """
    # v1
    # return sorted(INPUT_DIR.glob("*.txt"))[:limit]
    # v2
    all_files = list(INPUT_DIR.glob("*.txt"))
    random.Random(seed).shuffle(all_files)
    # print("Sampled files:", [file.name for file in all_files[:limit]])
    return all_files[:limit]


def benchmark_report(durations: List[float]) -> None:
    """Print timing statistics for all successful benchmark runs."""
    if not durations:
        print("No successful benchmark runs.")
        return

    average = statistics.mean(durations)
    fastest = min(durations)
    slowest = max(durations)
    std_dev = statistics.stdev(durations) if len(durations) > 1 else 0.0
    total = sum(durations)

    print(f"Average time       : {average:.3f} seconds")
    print(f"Total runs         : {len(durations)}")
    print(f"Total time         : {total:.3f} seconds")
    print(f"Fastest run        : {fastest:.3f} seconds")
    print(f"Slowest run        : {slowest:.3f} seconds")
    print(f"Standard deviation : {std_dev:.3f} seconds")


def benchmark_runs() -> None:
    """Run full benchmark set multiple times and report average duration."""
    files = sample_files()
    durations = []

    print(f"Benchmarking {len(files)} input files, {RUNS} full-set runs total...")

    for i in range(1, RUNS + 1):
        print(f"Run {i}")
        duration = run_single_set(files)
        if duration >= 0:
            durations.append(duration)
            print(f"  Time: {duration:.3f} seconds\n")
        else:
            print("  Run failed. Skipping timing.\n")

    benchmark_report(durations)


def main() -> None:
    print(f"🔧 Rebuilding Docker image: {DOCKER_IMAGE}...")
    result = subprocess.run(
        ["docker", "build", "-t", DOCKER_IMAGE, "."], capture_output=True
    )
    if result.returncode != 0:
        print("❌ Docker build failed!")
        print(result.stderr.decode())
        return
    print("✅ Docker build complete.\n")

    benchmark_runs()


if __name__ == "__main__":
    main()
