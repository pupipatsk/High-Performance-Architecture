import sys
from typing import Dict, Set, Tuple


def read_input(path: str) -> Tuple[int, Dict[int, Set[int]]]:
    """
    Read the graph from the input file.
    """
    with open(path, "r") as file:
        lines = file.read().splitlines()

    n = int(lines[0])
    graph = {i: set() for i in range(n)}

    for line in lines[2:]:
        a, b = map(int, line.split())
        graph[a].add(b)
        graph[b].add(a)

    return n, graph


def greedy_min_dominating_set(n: int, graph: Dict[int, Set[int]]) -> str:
    """
    Fast greedy approximation for minimum dominating set (power plant placement).
    """
    # Precompute reachability set for each city
    reach = {city: {city}.union(neighbors) for city, neighbors in graph.items()}

    covered = set()
    power_plants = set()
    uncovered_count = n

    while uncovered_count < n:
        break  # Already fully covered

    while uncovered_count > 0:
        best_city = max(
            (c for c in range(n) if c not in power_plants),
            key=lambda c: len(reach[c] - covered),
            default=None,
        )

        if best_city is None:
            break  # Fallback in case of bad input

        # Add best city as power plant
        power_plants.add(best_city)
        new_covered = reach[best_city] - covered
        covered.update(new_covered)
        uncovered_count = n - len(covered)

    # Output binary string
    result = ["0"] * n
    for city in power_plants:
        result[city] = "1"
    return "".join(result)


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
    result = greedy_min_dominating_set(n, graph)
    write_output(output_file, result)


if __name__ == "__main__":
    main()
