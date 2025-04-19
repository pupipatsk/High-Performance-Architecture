#!/bin/bash

# === Configuration ===
CPP_SOLVER_DIR="cpp-solver"
BIN_DIR="$CPP_SOLVER_DIR/bin"
OUTPUT_DIR=".tmp"
RUNS=5

# == Choose dataset ==
# v.Original
# INPUT_DIR="data/input"
# EXPECTED_OUTPUT_DIR="data/expected-output"
# # INPUT_FILES=($(find "$INPUT_DIR" -type f -name "*.txt" | sort | head -n 3))
# INPUT_FILES=($(find "$INPUT_DIR" -type f -name "*.txt" | sort))
# v.Synthetic
INPUT_DIR="data/syn/input"
EXPECTED_OUTPUT_DIR="data/syn/expected-output"
INPUT_FILES=($(find "$INPUT_DIR" -type f -name "syn-*.txt" | sort))

mkdir -p "$OUTPUT_DIR"

# === Compile Solvers ===
echo "🔧 Compiling all C++ solvers locally..."
./compile-cpp.sh

# === OpenMP Test ===
echo
echo "🧪 Testing OpenMP on host..."
g++-14 -O2 -fopenmp -isystem /opt/homebrew/include -L/opt/homebrew/lib openmp-test.cpp -o omp-test && ./omp-test && rm omp-test
echo

# === Run Solvers ===
echo "🚀 Running C++ solvers locally..."
global_correct=0
CPP_EXES=($(find "$BIN_DIR" -type f | sort))
echo "Found ${#CPP_EXES[@]} solvers in $BIN_DIR:"
NUM_FILES=${#INPUT_FILES[@]}

# Validator function
check_validity() {
    input_file="$1"
    output_file="$2"

    input_name=$(basename "$input_file" .txt)
    expected_file="$EXPECTED_OUTPUT_DIR/${input_name}.out"

    if [[ ! -f "$output_file" || ! -f "$expected_file" ]]; then
        return 1
    fi

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
    echo "Running solver: $solver_id..."
    total_time_ns=0
    correct=0
    total=0

    for ((run = 1; run <= RUNS; run++)); do
        for input in "${INPUT_FILES[@]}"; do
            input_name=$(basename "$input" .txt)
            output_path="$OUTPUT_DIR/${solver_id}/run${run}/${input_name}.out"
            mkdir -p "$(dirname "$output_path")"

            start=$(date +%s%N)
            "$exe" "$input" "$output_path" || {
                echo "💥 Runtime error in $solver_id on $input_name"
                continue
            }
            end=$(date +%s%N)

            elapsed_ns=$((end - start))
            total_time_ns=$((total_time_ns + elapsed_ns))

            ((total++))
            if [[ ! -s "$output_path" ]]; then
                echo "⚠️  $solver_id produced empty output for $input_name"
            elif check_validity "$input" "$output_path" >/dev/null; then
                ((correct++))
            fi

            progress=$(((total * 100) / (NUM_FILES * RUNS)))
            tmp_avg_per_file_ns=$((total_time_ns / total))
            tmp_avg_per_file_ms=$(echo "scale=2; $tmp_avg_per_file_ns / 1000000" | bc)
            tmp_accuracy_percent=$(echo "scale=2; 100 * $correct / $total" | bc)
            echo -ne "\r> $solver_id: Run $run/$RUNS | Progress: $progress% | Avg: ${tmp_avg_per_file_ms} ms | Acc: ${tmp_accuracy_percent}%"
        done
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
for line in "${results[@]}"; do echo " - $line"; done
echo
if [[ "$global_correct" -gt 0 ]]; then
    echo "🥇 Best Solver (fastest per file): $best_solver (${best_time} ms)"
else
    echo "❌ No solver produced any valid result (all accuracy = 0%)."
fi

# === Cleanup ===
echo "💾 Cleaning up..."
rm -rf "$OUTPUT_DIR"
rm -rf "$BIN_DIR"
echo
echo "✅ Done."
