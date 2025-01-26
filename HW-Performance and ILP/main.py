import gzip
import time
import numpy as np
# from tqdm import tqdm

def solution1(filename):
    """.gz files."""
    count = 0
    with gzip.open(filename, 'rt') as f:
        for line in f:
        # for line in tqdm(f):
            # v.1
            value = float(line.strip().split(' ')[1])
            if value > 50:
                count += 1
    return count

def solution2(filename):
    """.gz files."""
    count = 0
    with gzip.open(filename, 'rt') as f:
        for line in f:
        # for line in tqdm(f):
            # v.2
            value = line.strip().split(' ')[1]
            value = int(value.split('.')[0])
            if value > 50:
                count += 1
    return count

def solution3(filename):
    """.txt files."""
    count = 0
    with open(filename, 'r') as f:
        for line in f:
            value = float(line.strip().split(' ')[1])
            if value > 50:
                count += 1
    return count

def solution4(filename):
    """.txt files."""
    count = 0
    with open(filename, 'r') as f:
        for line in f:
            value = line.strip().split(' ')[1]
            value = int(value.split('.')[0])
            if value > 50:
                count += 1
    return count

def solution5(filename):
    """NumPy solution for .txt files."""
    with open(filename, 'r') as f:
        # Read all lines and split into a NumPy array
        values = np.array([float(line.strip().split(' ')[1]) for line in f])

    # Use NumPy vectorized operations to count values > 50
    count = np.sum(values > 50)
    return count

if __name__ == '__main__':
    # .gz files
    # filename = "HPA/sort-rand-19999999.txt.gz"
    # filename = "HPA/sort-rand-199999999.txt.gz"
    filename = "HPA/sort-rev-19999999.txt.gz"
    # filename = "HPA/sort-rev-199999999.txt.gz"

    # .txt files
    # filename = "HPA/sort-rand-19999999.txt"
    # filename = "HPA/sort-rand-199999999.txt"
    # filename = "HPA/sort-rev-19999999.txt"
    # filename = "HPA/sort-rev-199999999.txt"

    start_time = time.time()
    count = solution1(filename)
    print(f"Count(x>50): {count:,}")
    print(f"Total time: {time.time() - start_time:.3f} s.")
