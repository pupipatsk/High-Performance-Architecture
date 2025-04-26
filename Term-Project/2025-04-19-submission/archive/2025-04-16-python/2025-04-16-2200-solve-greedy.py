import sys
from typing import Tuple, Dict, Set


def read_input(path: str) -> Tuple[int, Dict[int, Set[int]]]:
    """
    Reads the input graph file and returns the number of nodes and the adjacency list.
    """
    with open(path, "r") as f:
        lines = f.read().strip().splitlines()

    n = int(lines[0])  # number of cities
    graph = {i: set() for i in range(n)}

    for line in lines[2:]:
        a, b = map(int, line.strip().split())
        graph[a].add(b)
        graph[b].add(a)

    return n, graph


def greedy_power_plant_placement(n: int, graph: Dict[int, Set[int]]) -> str:
    """
    Greedy algorithm to place power plants such that all cities are powered.
    Returns a binary string where '1' means a plant is placed.
    """
    covered = set()
    power_plants = set()

    while len(covered) < n:
        best_city = -1
        max_new_cover = -1

        for city in range(n):
            if city in power_plants:
                continue

            reach = {city} | graph[city]
            new_cover = len(reach - covered)

            if new_cover > max_new_cover:
                max_new_cover = new_cover
                best_city = city

        power_plants.add(best_city)
        covered |= {best_city} | graph[best_city]

    output = ["0"] * n
    for city in power_plants:
        output[city] = "1"

    return "".join(output)


def write_output(path: str, result: str) -> None:
    """
    Writes the result to the output file.
    """
    with open(path, "w") as f:
        f.write(result + "\n")


def main() -> None:
    if len(sys.argv) != 3:
        print("Usage: python3 solve.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    n, graph = read_input(input_file)
    result = greedy_power_plant_placement(n, graph)
    write_output(output_file, result)


if __name__ == "__main__":
    main()
