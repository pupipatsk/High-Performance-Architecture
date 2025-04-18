#!/bin/bash

# === CONFIG ===
PYTHON_IMAGE_NAME="power-plant-python"
CPP_IMAGE_NAME="power-plant-cpp"
INPUT_DIR="data/input"
OUTPUT_DIR="data/output"
TMP_DIR=".tmp_outputs"
CPP_SOLVER_DIR="cpp-solver"
PYTHON_SOLVER_DIR="python-solver"
THREADS=12

mkdir -p "$TMP_DIR" "$OUTPUT_DIR"

# # === STEP 1: Build Docker for Python solvers ===
# echo "🔨 Building Docker image for Python solvers..."
# docker build -t "$PYTHON_IMAGE_NAME" -f Dockerfile-python .

# # === STEP 2: Run all Python solvers ===
# echo "🚀 Running Python solvers..."
# PY_SOLVERS=($(find "$PYTHON_SOLVER_DIR" -type f -name "*.py" | sort))
# INPUT_FILES=($(find "$INPUT_DIR" -name "*.txt"))

# for solver in "${PY_SOLVERS[@]}"; do
#     tag=$(basename "$solver")
#     echo "  ▶ $tag"
#     for input in "${INPUT_FILES[@]}"; do
#         base=$(basename "$input")
#         output="$TMP_DIR/python-${tag%.py}-${base%.txt}.out"

#         start=$(date +%s.%N)
#         docker run --rm \
#             -v "$(pwd)/$INPUT_DIR:/input" \
#             -v "$(pwd)/$TMP_DIR:/output" \
#             "$PYTHON_IMAGE_NAME" "$tag" "/input/$base" "/output/$(basename "$output")"
#         end=$(date +%s.%N)
#         dur=$(echo "$end - $start" | bc)
#         echo "$dur" > "$output.time"
#     done
# done

# === STEP 3: Build Docker for C++ solvers ===
echo "🔨 Building Docker image for C++ compilation..."
docker build -t "$CPP_IMAGE_NAME" -f Dockerfile-cpp --build-arg SOLVER=dummy.cpp . || true

# === STEP 4: Compile all C++ solvers using script ===
echo "🔧 Compiling all C++ solvers..."
chmod +x compile-cpp.sh
./compile-cpp.sh "$CPP_SOLVER_DIR" "$TMP_DIR"

# === STEP 5: Run all C++ solvers ===
echo "🚀 Running C++ solvers..."
CPP_EXES=($(find "$TMP_DIR" -type f -perm -111 -name "solve-*"))

for exe in "${CPP_EXES[@]}"; do
    tag=$(basename "$exe")
    echo "  ▶ $tag"
    for input in "${INPUT_FILES[@]}"; do
        base=$(basename "$input")
        output="$TMP_DIR/cpp-${tag#solve-}-${base%.txt}.out"

        start=$(date +%s.%N)
        "$exe" "$input" "$output"
        end=$(date +%s.%N)
        dur=$(echo "$end - $start" | bc)
        echo "$dur" > "$output.time"
    done
done

# === STEP 6: Report timing results ===
echo
echo "📊 Timing Results (seconds):"
for file in $(find "$TMP_DIR" -name "*.time" | sort); do
    name=$(basename "$file" .out.time)
    echo -n "⏱️  $name: "
    cat "$file"
done

# === STEP 7: Cleanup docker images ===
echo "🧹 Removing Docker images..."
docker image rm -f "$PYTHON_IMAGE_NAME" "$CPP_IMAGE_NAME" > /dev/null

echo "✅ Done."