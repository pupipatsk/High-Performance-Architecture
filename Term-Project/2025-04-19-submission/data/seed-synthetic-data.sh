# === Tiny graphs ===
python generate-synthetic-data.py -n 1 -e 0
python generate-synthetic-data.py -n 2 -e 1
python generate-synthetic-data.py -n 10 -e 9
python generate-synthetic-data.py -n 10 -e 15
python generate-synthetic-data.py -n 10 -e 45  # complete

# === Small graphs ===
python generate-synthetic-data.py -n 50 -e 49      # chain/tree
python generate-synthetic-data.py -n 50 -e 100     # sparse
python generate-synthetic-data.py -n 50 -e 300     # dense

python generate-synthetic-data.py -n 100 -e 99
python generate-synthetic-data.py -n 100 -e 300
python generate-synthetic-data.py -n 100 -e 1000

# === Medium graphs ===
python generate-synthetic-data.py -n 1000 -e 999
python generate-synthetic-data.py -n 1000 -e 2000
python generate-synthetic-data.py -n 1000 -e 10000
python generate-synthetic-data.py -n 1000 -e 499500  # complete

python generate-synthetic-data.py -n 5000 -e 5000
python generate-synthetic-data.py -n 5000 -e 15000
python generate-synthetic-data.py -n 5000 -e 50000

# === Large graphs ===
python generate-synthetic-data.py -n 10000 -e 9999
python generate-synthetic-data.py -n 10000 -e 50000
python generate-synthetic-data.py -n 10000 -e 100000

python generate-synthetic-data.py -n 50000 -e 50000
python generate-synthetic-data.py -n 50000 -e 100000
python generate-synthetic-data.py -n 50000 -e 250000

# === Extra-large graphs ===
python generate-synthetic-data.py -n 100000 -e 99999
python generate-synthetic-data.py -n 100000 -e 250000
python generate-synthetic-data.py -n 100000 -e 1000000

python generate-synthetic-data.py -n 500000 -e 500000
python generate-synthetic-data.py -n 500000 -e 1500000

# === Massive graph ===
python generate-synthetic-data.py -n 1000000 -e 1000000
python generate-synthetic-data.py -n 1000000 -e 2000000