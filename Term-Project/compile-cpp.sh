#!/bin/bash
set -e
CPP_DIR="$1"
OUT_DIR="$2"

OPENMP_LIST=("2025-04-19-0003-parallel-recursive-backtracking.cpp")

mkdir -p "$OUT_DIR"

for file in "$CPP_DIR"/*.cpp; do
    filename=$(basename "$file")             # e.g., mysolver.cpp
    base="${filename%.cpp}"                  # e.g., mysolver
    exe="solve-$base"
    echo "  🔧 Compiling $filename -> $exe inside Docker..."


    docker run --rm \
        -v "$(pwd)/$CPP_DIR":/src \
        -v "$(pwd)/$OUT_DIR":/out \
        gcc:12 \
        bash -c "g++ -O3 -march=native /src/$filename -o /out/$exe || { echo '❌ Failed to compile $filename'; exit 1; }"
done