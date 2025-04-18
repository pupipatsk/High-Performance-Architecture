#!/bin/bash

cd input || { echo "❌ Cannot find input/ folder"; exit 1; }

for file in *; do
    # Skip if it's already .txt
    if [[ "$file" == *.txt ]]; then
        continue
    fi

    # If it ends in .dat, rename to .txt
    if [[ "$file" == *.dat ]]; then
        newname="${file%.dat}.txt"
        mv "$file" "$newname"
        echo "Renamed $file → $newname"

    else
        # Otherwise, add .txt extension
        newname="${file}.txt"
        mv "$file" "$newname"
        echo "Renamed $file → $newname"
    fi
done

echo "All filenames in input/ are normalized to *.txt"
