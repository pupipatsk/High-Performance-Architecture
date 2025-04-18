#!/bin/bash

# === Configuration ===
DOCKER_IMAGE_NAME="power-plant-solver"
CPP_SOLVER_DIR="cpp-solver"
OUTPUT_DIR="data/output"
TMP_DIR=".tmp"
RUNS=1
# == Choose dataset ==
# v.Real
INPUT_DIR="data/input"
EXPECTED_OUTPUT_DIR="data/expected-output"
INPUT_FILES=($(find "$INPUT_DIR" -type f -name "*.txt" | sort | head -n 3))
# INPUT_FILES=($(find "$INPUT_DIR" -type f -name "*.txt" | sort))
# v.Synthetic
# INPUT_DIR="data/syn/input"
# EXPECTED_OUTPUT_DIR="data/syn/expected-output"
# INPUT_FILES=($(find "$INPUT_DIR" -type f -name "syn-*.txt" | sort)) #| head -n 15))
# ======

mkdir -p "$TMP_DIR" "$OUTPUT_DIR"

# === Docker Build ===
echo "🔨 Building Docker image (not used for run)..."
docker build -t "$DOCKER_IMAGE_NAME" -f Dockerfile-cpp --build-arg SOLVER=dummy.cpp . || true

# === Compile Solvers ===
echo "🔧 Compiling all C++ solvers locally..."
chmod +x compile-cpp.sh
./compile-cpp.sh "$CPP_SOLVER_DIR" "$TMP_DIR"

# === OpenMP Test ===
echo "🧪 Testing OpenMP inside Docker..."
cat >"$TMP_DIR/test_openmp.cpp" <<EOF
#include <omp.h>
#include <cstdio>
int main() {
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        printf("Hello from thread %d\\n", tid);
    }
    return 0;
}
EOF
docker run --rm \
    -v "$(pwd)/$TMP_DIR":/src \
    gcc:12 \
    bash -c "g++ -fopenmp -o /src/test_openmp /src/test_openmp.cpp && /src/test_openmp"
echo

# === Run Solvers ===
echo "🚀 Running C++ solvers locally..."
global_correct=0
CPP_EXES=($(find "$TMP_DIR" -type f -perm -111 -name "solve-*" | sort))
NUM_FILES=${#INPUT_FILES[@]}

# Validator
check_validity() {
    input_file="$1"
    output_file="$2"

    input_name=$(basename "$input_file" .txt)
    # expected_file="data/expected-output/${input_name}.out"
    expected_file="$EXPECTED_OUTPUT_DIR/${input_name}.out"

    if [[ ! -f "$output_file" || ! -f "$expected_file" ]]; then
        return 1
    fi

    # Compare only the last line of each file
    actual=$(tail -n 1 "$output_file" | tr -d '\r\n[:space:]')
    expected=$(tail -n 1 "$expected_file" | tr -d '\r\n[:space:]')

    [[ "$actual" == "$expected" ]]
    if [[ "$actual" != "$expected" ]]; then
        echo "❌ Mismatch for $input_name"
        echo "Expected: $expected"
        echo "Actual:   $actual"
    fi
}

echo
echo "📊 Average Time per File (in ms) & Accuracy:"

best_solver=""
best_time=""
declare -a results=()

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
                bash -c "/project/$TMP_DIR/$solver_tag /project/$input /project/$output_path" ||
                {
                    echo "💥 Runtime error in $solver_id on $input_name"
                    continue
                }
            ((total++))
            if [[ ! -s "$output_path" ]]; then
                echo "⚠️  $solver_id produced empty output for $(basename "$input")"
            elif check_validity "$input" "$output_path" >/dev/null; then
                ((correct++))
            fi
            progress=$(((total * 100) / (NUM_FILES * RUNS)))
            if [[ $total_time_ns -gt 0 ]]; then
                tmp_avg_per_file_ns=$((total_time_ns / total))
                tmp_avg_per_file_ms=$(echo "scale=2; $tmp_avg_per_file_ns / 1000000" | bc)
            else
                tmp_avg_per_file_ms="0.00"
            fi
            tmp_accuracy_percent=$(echo "scale=2; 100 * $correct / $total" | bc)
            echo -ne "\r> $solver_id: Run $run/$RUNS | Progress: $progress% | Avg: ${tmp_avg_per_file_ms} ms | Acc: ${tmp_accuracy_percent}%"
        done
        end=$(date +%s%N)
        run_time_ns=$((end - start))
        total_time_ns=$((total_time_ns + run_time_ns))
    done

    avg_time_ns=$((total_time_ns / RUNS))
    avg_per_file_ns=$((avg_time_ns / NUM_FILES))
    avg_per_file_ms=$(echo "scale=3; $avg_per_file_ns / 1000000" | bc)
    accuracy_percent=$(echo "scale=2; 100 * $correct / $total" | bc)
    global_correct=$((global_correct + correct))
    printf "\n%s: %.3f ms | Accuracy: %.2f%% (%d/%d)\n" "$solver_id" "$avg_per_file_ms" "$accuracy_percent" "$correct" "$total"
    results+=("$solver_id: ${avg_per_file_ms} ms | Accuracy: ${accuracy_percent}% (${correct}/${total})")
    if [ -z "$best_time" ] || (($(echo "$avg_per_file_ms < $best_time" | bc -l))); then
        best_time="$avg_per_file_ms"
        best_solver="$solver_id"
    fi
done

# === Summary ===
echo
echo "🏆 Summary:"
for line in "${results[@]}"; do
    echo " - $line"
done
echo
if [[ "$global_correct" -gt 0 ]]; then
    echo "🥇 Best Solver (fastest per file): $best_solver (${best_time} ms)"
else
    echo "❌ No solver produced any valid result (all accuracy = 0%)."
fi

# === Cleanup ===
echo
echo "🧹 Cleaning up Docker image..."
docker image rm -f "$DOCKER_IMAGE_NAME" >/dev/null 2>&1 || echo "⚠️  Failed to remove image: $CPP_IMAGE_NAME"
echo "🧹 Cleaning up temporary files..."
rm -rf "$TMP_DIR"
echo
echo "✅ Done."
