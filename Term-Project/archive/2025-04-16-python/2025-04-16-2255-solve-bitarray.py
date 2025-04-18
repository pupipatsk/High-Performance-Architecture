import sys
from typing import Dict, Tuple, List
from bitarray import bitarray


def read_input(path: str) -> Tuple[int, Dict[int, List[int]]]:
    with open(path, "r") as f:
        lines = f.read().splitlines()

    n = int(lines[0])
    graph = {i: [] for i in range(n)}
    for line in lines[2:]:
        a, b = map(int, line.split())
        graph[a].append(b)
        graph[b].append(a)
    return n, graph


def create_reach_sets(n: int, graph: Dict[int, List[int]]) -> List[bitarray]:
    """
    Precompute reach bitarrays: each city can power itself + its neighbors.
    """
    reach = [bitarray(n) for _ in range(n)]
    for i in range(n):
        bits = bitarray(n)
        bits.setall(0)
        bits[i] = 1
        for neighbor in graph[i]:
            bits[neighbor] = 1
        reach[i] = bits
    return reach


def greedy_cover(n: int, reach: List[bitarray]) -> List[int]:
    """
    Greedy algorithm: iteratively choose the city that covers the most uncovered.
    """
    covered = bitarray(n)
    covered.setall(0)
    solution = []

    while covered.count() < n:
        best_city = -1
        best_gain = -1
        for city in range(n):
            if city in solution:
                continue
            new_cover = (reach[city] & ~covered).count()
            if new_cover > best_gain:
                best_gain = new_cover
                best_city = city
        solution.append(best_city)
        covered |= reach[best_city]

    return solution


def prune_solution(n: int, reach: List[bitarray], solution: List[int]) -> List[int]:
    """
    Try removing each power plant, keeping the set valid.
    """
    solution = list(solution)  # make a copy
    for city in sorted(solution):
        test_set = [c for c in solution if c != city]
        covered = bitarray(n)
        covered.setall(0)
        for c in test_set:
            covered |= reach[c]
        if covered.count() == n:
            solution = test_set
    return solution


def encode_output(n: int, solution: List[int]) -> str:
    bits = ["0"] * n
    for city in solution:
        bits[city] = "1"
    return "".join(bits)


def solve(n: int, graph: Dict[int, List[int]]) -> str:
    reach = create_reach_sets(n, graph)
    greedy = greedy_cover(n, reach)
    pruned = prune_solution(n, reach, greedy)
    return encode_output(n, pruned)


def write_output(path: str, result: str) -> None:
    with open(path, "w") as f:
        f.write(result + "\n")


def main():
    if len(sys.argv) != 3:
        print("Usage: python3 solve.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    n, graph = read_input(input_file)
    result = solve(n, graph)
    write_output(output_file, result)


if __name__ == "__main__":
    main()
