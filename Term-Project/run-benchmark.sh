#!/bin/bash

# Final Docker run test:
# time docker run -v $(pwd)/data/input:/input -v $(pwd)/data/output:/output power-plant-solver /input/grid-6-7.txt /output/grid-6-7.out

# === CONFIGURATION ===
IMAGE_NAME="power-plant-solver"          # Docker image name
INPUT_DIR="data/input"
OUTPUT_DIR="data/output"
RUNS=5
TEST_LIMIT=15
SEED=42

# === INIT ===
mkdir -p "$OUTPUT_DIR"
echo "Benchmarking up to $TEST_LIMIT input files for $RUNS runs..."

# === FUNCTION: Sample files with fixed seed ===
sample_files() {
    find "$INPUT_DIR" -type f -name "*.txt" | sort | awk 'BEGIN {srand('"$SEED"')} {print rand() "\t" $0}' \
    | sort -n | cut -f2 | head -n "$TEST_LIMIT"
}

# === FUNCTION: Run single full-set benchmark ===
run_single_set() {
    local files=("$@")
    local start=$(date +%s.%N)
    for file in "${files[@]}"; do
        local base=$(basename "$file")
        local outfile="${base%.txt}.out"

        docker run --rm \
            -e OMP_NUM_THREADS=12 \
            -v "$(pwd)/$INPUT_DIR:/input" \
            -v "$(pwd)/$OUTPUT_DIR:/output" \
            "$IMAGE_NAME" \
            "/input/$base" "/output/$outfile" > /dev/null

        if [ $? -ne 0 ]; then
            echo "Error: Failed on $base"
            return 1
        fi
    done
    local end=$(date +%s.%N)
    local duration=$(echo "$end - $start" | bc)
    echo "$duration"
    return 0
}

# === FUNCTION: Compute report ===
benchmark_report() {
    local times=("$@")
    local total=0
    local min=${times[0]}
    local max=${times[0]}

    for t in "${times[@]}"; do
        total=$(echo "$total + $t" | bc)
        (( $(echo "$t < $min" | bc -l) )) && min=$t
        (( $(echo "$t > $max" | bc -l) )) && max=$t
    done

    local count=${#times[@]}
    local avg=$(echo "$total / $count" | bc -l)

    # Optional: standard deviation
    local sumsq=0
    for t in "${times[@]}"; do
        diff=$(echo "$t - $avg" | bc -l)
        sq=$(echo "$diff * $diff" | bc -l)
        sumsq=$(echo "$sumsq + $sq" | bc -l)
    done
    local stddev=$(echo "sqrt($sumsq / $count)" | bc -l)

    echo "Average time       : $(printf "%.3f" "$avg") seconds"
    echo "Total runs         : $count"
    echo "Total time         : $(printf "%.3f" "$total") seconds"
    echo "Fastest run        : $(printf "%.3f" "$min") seconds"
    echo "Slowest run        : $(printf "%.3f" "$max") seconds"
    echo "Standard deviation : $(printf "%.3f" "$stddev") seconds"
}

# === MAIN ===
echo "Rebuilding Docker image: $IMAGE_NAME..."
docker build -t "$IMAGE_NAME" . || {
    echo "Docker build failed. Exiting."
    exit 1
}
echo "Docker build completed."

all_files=($(sample_files))
durations=()

for ((i=1; i<=RUNS; i++)); do
    echo "Run $i"
    result=$(run_single_set "${all_files[@]}")
    if [ $? -eq 0 ]; then
        durations+=("$result")
        echo "  Time: $result seconds"
    else
        echo "  Run failed. Skipping timing."
    fi
done

echo
benchmark_report "${durations[@]}"
docker image rm -f "$IMAGE_NAME" > /dev/null
