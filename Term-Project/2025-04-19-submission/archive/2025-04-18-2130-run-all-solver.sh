#!/bin/bash

CPP_IMAGE_NAME="power-plant-cpp"
INPUT_DIR="data/input"
OUTPUT_DIR="data/output"
TMP_DIR=".tmp_outputs"
CPP_SOLVER_DIR="cpp-solver"
RUNS=5

mkdir -p "$TMP_DIR" "$OUTPUT_DIR"

echo "🔨 Building Docker image (not used for run)..."
docker build -t "$CPP_IMAGE_NAME" -f Dockerfile-cpp --build-arg SOLVER=dummy.cpp . || true

echo "🔧 Compiling all C++ solvers locally..."
chmod +x compile-cpp.sh
./compile-cpp.sh "$CPP_SOLVER_DIR" "$TMP_DIR"

echo "🚀 Running C++ solvers locally..."
CPP_EXES=($(find "$TMP_DIR" -type f -perm -111 -name "solve-*"))
INPUT_FILES=($(find "$INPUT_DIR" -type f -name "*.txt" | sort))
NUM_FILES=${#INPUT_FILES[@]}

# Validator
check_validity() {
    input_file="$1"
    output_file="$2"

    if [[ ! -f "$output_file" ]]; then return 1; fi

    python3 - "$input_file" "$output_file" <<EOF
import sys

def read_input(path):
    with open(path) as f:
        n = int(f.readline())
        m = int(f.readline())
        edges = [tuple(map(int, line.split())) for line in f]
    graph = [[] for _ in range(n)]
    for u, v in edges:
        graph[u].append(v)
        graph[v].append(u)
    return graph

def read_solution(path, n):
    with open(path) as f:
        lines = f.read().splitlines()
        if not lines:
            return None
        last = lines[-1].strip()
        if len(last) != n or any(c not in "01" for c in last):
            return None
        return list(map(int, last))

def is_valid(graph, solution):
    powered = [False] * len(graph)
    for i, has_plant in enumerate(solution):
        if has_plant:
            powered[i] = True
            for neighbor in graph[i]:
                powered[neighbor] = True
    return all(powered)

graph = read_input(sys.argv[1])
solution = read_solution(sys.argv[2], len(graph))
if solution is None: sys.exit(1)
sys.exit(0 if is_valid(graph, solution) else 1)
EOF
}

echo
echo "📊 Average Time per File (in ms) & Accuracy:"

best_solver=""
best_time=""

for exe in "${CPP_EXES[@]}"; do
    solver_tag=$(basename "$exe")
    solver_id="cpp-${solver_tag#solve-}"

    total_time_ns=0
    correct=0
    total=0

    for ((run = 1; run <= RUNS; run++)); do
        start=$(date +%s%N)
        for input in "${INPUT_FILES[@]}"; do
            input_name=$(basename "$input" .txt)
            solver_dir="$TMP_DIR/${solver_id}"
            mkdir -p "$solver_dir"
            output_path="$solver_dir/${input_name}.out"
            docker run --rm \
                -v "$(pwd)":/project \
                gcc:12 \
                bash -c "/project/$TMP_DIR/$solver_tag /project/$input /project/$output_path"
            ((total++))
            if [[ ! -s "$output_path" ]]; then
                echo "⚠️  $solver_id produced empty output for $(basename "$input")"
            elif [[ -f "$output_path" ]]; then
                echo "🧪 Validating: $solver_id/$(basename "$output_path")..."
                if check_validity "$input" "$output_path" >/dev/null; then
                    ((correct++))
                else
                    echo "❌ Invalid output for $solver_id on $(basename "$input")"
                fi
            fi
        done
        end=$(date +%s%N)
        run_time_ns=$((end - start))
        total_time_ns=$((total_time_ns + run_time_ns))
    done

    avg_time_ns=$((total_time_ns / RUNS))
    avg_per_file_ns=$((avg_time_ns / NUM_FILES))
    avg_per_file_ms=$(echo "scale=3; $avg_per_file_ns / 1000000" | bc)
    accuracy_percent=$(echo "scale=2; 100 * $correct / $total" | bc)

    printf "%s: %.3f ms | Accuracy: %.2f%% (%d/%d)\n" "$solver_id" "$avg_per_file_ms" "$accuracy_percent" "$correct" "$total"

    if [ -z "$best_time" ] || (($(echo "$avg_per_file_ms < $best_time" | bc -l))); then
        best_time="$avg_per_file_ms"
        best_solver="$solver_id"
    fi
done

echo
echo "🥇 Best Solver (fastest per file): $best_solver (${best_time} ms)"
echo
echo "✅ Done."
