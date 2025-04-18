import sys
import random
from typing import Dict, Set, Tuple
from concurrent.futures import ProcessPoolExecutor


def read_input(path: str) -> Tuple[int, Dict[int, Set[int]]]:
    with open(path, "r") as file:
        lines = file.read().splitlines()

    n = int(lines[0])
    graph = {i: set() for i in range(n)}
    for line in lines[2:]:
        a, b = map(int, line.split())
        graph[a].add(b)
        graph[b].add(a)
    return n, graph


def compute_reach_sets(graph: Dict[int, Set[int]]) -> Dict[int, Set[int]]:
    return {city: {city} | neighbors for city, neighbors in graph.items()}


def greedy_dominating_set(
    n: int, reach: Dict[int, Set[int]], seed: int = 42
) -> Set[int]:
    covered = set()
    power_plants = set()
    rng = random.Random(seed)
    candidates = list(range(n))

    while len(covered) < n:
        rng.shuffle(candidates)
        best_city = max(
            (c for c in candidates if c not in power_plants),
            key=lambda c: len(reach[c] - covered),
            default=None,
        )
        if best_city is None:
            break
        power_plants.add(best_city)
        covered.update(reach[best_city])
    return power_plants


def is_valid_dominating_set(
    n: int, reach: Dict[int, Set[int]], plants: Set[int]
) -> bool:
    covered = set()
    for p in plants:
        covered.update(reach[p])
    return len(covered) == n


def improve_solution(n: int, reach: Dict[int, Set[int]], plants: Set[int]) -> Set[int]:
    improved = set(plants)
    for city in sorted(plants):
        test_set = improved - {city}
        if is_valid_dominating_set(n, reach, test_set):
            improved = test_set
    return improved


def power_plant_string(n: int, plants: Set[int]) -> str:
    output = ["0"] * n
    for city in plants:
        output[city] = "1"
    return "".join(output)


def parallel_strategy(seed: int, n: int, reach: Dict[int, Set[int]]) -> Set[int]:
    initial = greedy_dominating_set(n, reach, seed)
    return improve_solution(n, reach, initial)


def solve_parallel(n: int, graph: Dict[int, Set[int]], runs: int = 4) -> str:
    reach = compute_reach_sets(graph)
    seeds = [42 + i for i in range(runs)]

    with ProcessPoolExecutor() as executor:
        futures = [executor.submit(parallel_strategy, seed, n, reach) for seed in seeds]
        results = [f.result() for f in futures]

    # Select the solution with the fewest power plants
    best = min(results, key=len)
    return power_plant_string(n, best)


def write_output(path: str, result: str) -> None:
    with open(path, "w") as file:
        file.write(result + "\n")


def main():
    if len(sys.argv) != 3:
        print("Usage: python3 solve.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    n, graph = read_input(input_file)
    result = solve_parallel(n, graph, runs=4)  # Run 4 strategies in parallel
    write_output(output_file, result)


if __name__ == "__main__":
    main()
