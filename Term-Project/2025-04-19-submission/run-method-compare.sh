#!/bin/bash

# === CONFIGURATION ===
ARCHIVE_DIR="archive"
INPUT_DIR="data/input"
OUTPUT_DIR="data/output"
TMP_DIR=".compare_build"
TEST_LIMIT=5
SEED=42
THREADS=12

mkdir -p "$TMP_DIR" "$OUTPUT_DIR"

# === SAMPLE FILES ===
sample_files() {
    find "$INPUT_DIR" -type f -name "*.txt" | sort | awk 'BEGIN {srand('"$SEED"')} {print rand() "\t" $0}' \
    | sort -n | cut -f2 | head -n "$TEST_LIMIT"
}
INPUT_FILES=($(sample_files))

# === Benchmark container ===
benchmark_in_docker() {
    local image="$1"
    local start=$(date +%s.%N)

    for infile in "${INPUT_FILES[@]}"; do
        local base=$(basename "$infile")
        local outfile="${base%.txt}.out"

        docker run --rm \
            -e OMP_NUM_THREADS=$THREADS \
            -v "$(pwd)/$INPUT_DIR:/input" \
            -v "$(pwd)/$OUTPUT_DIR:/output" \
            "$image" "/input/$base" "/output/$outfile" > /dev/null

        if [ $? -ne 0 ]; then
            echo "  ❌ Failed on $base"
            return 1
        fi
    done

    local end=$(date +%s.%N)
    echo "$(echo "$end - $start" | bc)"
}

# === MAIN ===
echo "📊 Comparing C++ methods using Docker..."
echo "Test set: ${#INPUT_FILES[@]} input files"
echo

for cppfile in "$ARCHIVE_DIR"/*.cpp; do
    method_name=$(basename "$cppfile" .cpp)
    image_name="solver-$method_name"

    echo "🔧 Building Docker image for: $method_name"

    # Generate a temp Dockerfile
    cat > "$TMP_DIR/Dockerfile" <<EOF
FROM gcc:12
WORKDIR /app
COPY $(basename "$cppfile") solve.cpp
RUN g++ -O3 -march=native -fopenmp solve.cpp -o solve
ENTRYPOINT ["./solve"]
EOF

    cp "$cppfile" "$TMP_DIR/$(basename "$cppfile")"

    docker build -t "$image_name" "$TMP_DIR" > /dev/null
    if [ $? -ne 0 ]; then
        echo "  ❌ Docker build failed: $method_name"
        continue
    fi

    echo "🚀 Benchmarking $method_name ..."
    runtime=$(benchmark_in_docker "$image_name")
    docker image rm -f "$image_name" > /dev/null
    if [ $? -eq 0 ]; then
        echo "✅ $method_name — Total time: $(printf "%.3f" "$runtime") seconds"
    fi

    echo
done
