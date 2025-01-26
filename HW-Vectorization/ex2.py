import timeit
import numpy as np
import sys

def main():
    # Benchmark configuration
    config = {
        'size': 6_400_000,      # Number of elements
        'dtype': np.int64,      # Data type
        'repeats': 5,           # Number of benchmark repeats
        'numpy_iterations': 1000,  # Iterations per numpy run
        'random_seed': 42       # For reproducibility
    }

    # System information
    print(f"Python {sys.version}\nNumPy {np.__version__}")
    print(f"Benchmarking {config['size']:,} elements ({config['dtype'].__name__})")

    # Initialize data with fixed seed
    np.random.seed(config['random_seed'])
    a_np = np.random.randint(1, 1000, config['size'], dtype=config['dtype'])
    b_np = np.random.randint(1, 1000, config['size'], dtype=config['dtype'])
    a_list = a_np.tolist()
    b_list = b_np.tolist()

    # Benchmark 1: Python list iteration
    list_times = timeit.repeat(
        stmt='for i in range(len(a)): a[i] += b[i]',
        setup='a = list_a.copy(); b = list_b.copy()',
        globals={'list_a': a_list, 'list_b': b_list},
        number=1,
        repeat=config['repeats']
    )

    # Benchmark 2: NumPy vectorized operation
    numpy_times = timeit.repeat(
        stmt='a += b',
        setup='a = np_a.copy(); b = np_b.copy()',
        globals={'np_a': a_np, 'np_b': b_np},
        number=config['numpy_iterations'],
        repeat=config['repeats']
    )

    # Calculate statistics
    avg_list_ms = (sum(list_times) / len(list_times)) * 1000  # Convert s to ms
    avg_numpy_ms = (sum(numpy_times) / (len(numpy_times) * config['numpy_iterations'])) * 1000
    speedup = avg_list_ms / avg_numpy_ms

    # Print results
    print("\nBenchmark Results (milliseconds):")
    print(f"{'Run':<5} {'List time (ms)':<15} {'NumPy time (ms)':<15}")
    for i, (lt, nt) in enumerate(zip(list_times, numpy_times)):
        list_ms = lt * 1000
        numpy_ms = (nt / config['numpy_iterations']) * 1000
        print(f"{i+1:<5} {list_ms:<15.3f} {numpy_ms:<15.3f}")

    print("\nSummary:")
    print(f"Average List Time:    {avg_list_ms:.3f} ms")
    print(f"Average NumPy Time:   {avg_numpy_ms:.3f} ms")
    print(f"Speedup Factor:       {speedup:,.0f}x")

if __name__ == '__main__':
    main()