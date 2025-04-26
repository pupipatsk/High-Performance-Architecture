#!/bin/bash

# === Configuration ===
CPP_DIR="cpp-solver"
OUT_DIR="$CPP_DIR/bin"
GCC_BIN="/opt/homebrew/bin/g++-14"

# Make sure the output directory exists
mkdir -p "$OUT_DIR"

echo "🔨 Compiling solvers in '$CPP_DIR' with output to '$OUT_DIR'..."
echo

for file in "$CPP_DIR"/*.cpp; do
    filename=$(basename "$file")
    base="${filename%.cpp}"
    output="$OUT_DIR/solve-$base"

    echo "🔧 Compiling $filename -> $(basename "$output")"

    # Special flags for specific file
    if [[ "$filename" == "2025-04-19-1555-3range-o4minihigh-2.cpp" || "$filename" == "2025-04-19-1619-gemini.cpp" ]]; then
        echo "   ⚙️ Using special flags: -O3 -march=native -funroll-loops"
        "$GCC_BIN" -O3 -march=native -funroll-loops \
            "$file" -o "$output" || echo "❌ Failed to compile $filename"
    elif [[ "$filename" == "2025-04-19-1610-3range-grok-2.cpp" ]]; then
        echo "   ⚙️ Using special flags: -O3 -march=native -funroll-loops -fopenmp"
        "$GCC_BIN" -O3 -march=native -funroll-loops -fopenmp\
            "$file" -o "$output" || echo "❌ Failed to compile $filename"
    # OpenMP detection
    elif grep -q "omp_" "$file" || [[ "$filename" == "2025-04-19-1626-grok-3.cpp" || "$filename" == "2025-04-19-1646-deepseek-mac.cpp" ]]; then
        echo "   ↪ Detected OpenMP. Using -fopenmp"
        "$GCC_BIN" -O3 -march=native -fopenmp \
            -isystem /opt/homebrew/include \
            -L/opt/homebrew/lib \
            "$file" -o "$output" || echo "❌ Failed to compile $filename"
    else
        "$GCC_BIN" -O3 -march=native \
            "$file" -o "$output" || echo "❌ Failed to compile $filename"
    fi
done

echo "✅ Done compiling all solvers."
