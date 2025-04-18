#!/usr/bin/env python3
import random
import argparse
from pathlib import Path
import sys


def generate_graph(n, m, seed=42):
    random.seed(seed)
    edge_set = set((i, i + 1) for i in range(n - 1))  # Chain
    while len(edge_set) < m:
        u = random.randint(0, n - 1)
        v = random.randint(0, n - 1)
        if u != v:
            edge = (min(u, v), max(u, v))
            edge_set.add(edge)  # No need to check manually
    return list(edge_set)


def build_adjacency(n, edges):
    adj = [[] for _ in range(n)]
    for u, v in edges:
        adj[u].append(v)
        adj[v].append(u)
    return adj


def greedy_power_plan(n, adj):
    solution = [0] * n
    powered = [False] * n
    for i in range(n):
        if not powered[i]:
            solution[i] = 1
            powered[i] = True
            for j in adj[i]:
                powered[j] = True
    return solution


def validate_solution(input_path, output_path):
    with open(input_path) as f:
        n = int(f.readline())
        m = int(f.readline())
        adj = [[] for _ in range(n)]
        for _ in range(m):
            u, v = map(int, f.readline().split())
            adj[u].append(v)
            adj[v].append(u)

    with open(output_path) as f:
        lines = f.read().splitlines()
        if not lines:
            return False
        last = lines[-1].strip()
        if len(last) != n or any(c not in "01" for c in last):
            return False

        sol = list(map(int, last))
        powered = [False] * n
        for i, has_plant in enumerate(sol):
            if has_plant:
                powered[i] = True
                for j in adj[i]:
                    powered[j] = True

        return all(powered)


def save_input(path, n, m, edges):
    with open(path, "w") as f:
        f.write(f"{n}\n{m}\n")
        for u, v in edges:
            f.write(f"{u} {v}\n")


def save_output(path, solution):
    with open(path, "w") as f:
        f.write("".join(str(bit) for bit in solution) + "\n")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--nodes", "-n", type=int, required=True, help="Number of nodes"
    )
    parser.add_argument(
        "--edges", "-e", type=int, required=True, help="Number of edges"
    )
    parser.add_argument(
        "--outdir", type=str, default="./syn", help="Base output directory"
    )
    args = parser.parse_args()

    name = f"syn-n{args.nodes}-e{args.edges}"
    input_dir = Path(args.outdir) / "input"
    output_dir = Path(args.outdir) / "expected-output"
    input_dir.mkdir(parents=True, exist_ok=True)
    output_dir.mkdir(parents=True, exist_ok=True)

    edges = generate_graph(args.nodes, args.edges)
    adj = build_adjacency(args.nodes, edges)
    solution = greedy_power_plan(args.nodes, adj)

    input_path = input_dir / f"{name}.txt"
    output_path = output_dir / f"{name}.out"

    save_input(input_path, args.nodes, args.edges, edges)
    save_output(output_path, solution)

    if not validate_solution(input_path, output_path):
        print("❌ Validation failed: output does not power all nodes.", file=sys.stderr)
        input_path.unlink(missing_ok=True)
        output_path.unlink(missing_ok=True)
        sys.exit(1)

    print(f"✅ Input saved to: {input_path}")
    print(f"✅ Output saved to: {output_path}")


if __name__ == "__main__":
    main()
