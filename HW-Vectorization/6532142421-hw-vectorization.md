# Exercise I

Design an experiment to measure the speed up of this vectorize code.

```c
// No AVX
void add(int size, int *a,int *b) {
    for (int i=0;i<size;i++) {
        a[i] += b[i];
    }
}
```

```c
// with AVX2
void add_avx(int size, int *a, int*b) {
    int i=0;
    for (;i<size;i+=8) {
        // load 256-bit chunks of each array
        __m256i av = _mm256_loadu_si256((__m256i*) &a[i]);
        __m256i bv = _mm256_loadu_si256((__m256i*) &b[i]);
        // add each pair of 32-bit integers in chunks
        av = _mm256_add_epi32(av, bv);
        // store 256-bit chunk to a
        _mm256_storeu_si256((__m256i*) &a[i], av);
    }
    // clean up
    for(;i<size;i++) {
        a[i] += b[i];
    }
}
```

## Solution

- Chip: Apple M2
- SIMD: ARM NEON
- Array Size: 10M elements (40MB)
- Iterations: 100 iterations/run
- Runs: 3 runs
- Compiler: `clang` with `-O3 -arch arm64 -fno-vectorize`

```c
#include <arm_neon.h>
#include <stdio.h>
#include <stdlib.h>
#include <mach/mach_time.h>

// Scalar (non-vectorized) version
void add(int size, int *a, int *b) {
    for (int i = 0; i < size; i++) {
        a[i] += b[i];
    }
}

// ARM NEON vectorized version (4 elements per iteration)
void add_neon(int size, int *a, int *b) {
    int i = 0;
    for (; i <= size - 4; i += 4) {
        int32x4_t av = vld1q_s32(a + i);
        int32x4_t bv = vld1q_s32(b + i);
        av = vaddq_s32(av, bv);
        vst1q_s32(a + i, av);
    }
    // Handle remaining elements
    for (; i < size; i++) {
        a[i] += b[i];
    }
}

int main() {
    const int SIZE = 10000000;  // 10 million elements
    const int ITERS = 100;
    const int RUNS = 3;
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);

    // Allocate aligned memory for NEON
    int *a, *b, *a_backup;
    posix_memalign((void **)&a, 16, SIZE * sizeof(int));
    posix_memalign((void **)&b, 16, SIZE * sizeof(int));
    posix_memalign((void **)&a_backup, 16, SIZE * sizeof(int));

    if (!a || !b || !a_backup) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    // Initialize with random values
    for (int i = 0; i < SIZE; i++) {
        a[i] = arc4random();
        b[i] = arc4random();
        a_backup[i] = a[i];  // Backup for resetting
    }

    double scalar_times[RUNS];
    double neon_times[RUNS];

    for (int run = 0; run < RUNS; run++) {
        // Benchmark scalar version
        double total_scalar = 0;
        for (int i = 0; i < ITERS; i++) {
            // Reset array
            for (int j = 0; j < SIZE; j++) a[j] = a_backup[j];

            uint64_t start = mach_absolute_time();
            add(SIZE, a, b);
            uint64_t end = mach_absolute_time();
            total_scalar += (end - start) * (double)timebase.numer / timebase.denom;
        }

        // Benchmark NEON version
        double total_neon = 0;
        for (int i = 0; i < ITERS; i++) {
            // Reset array
            for (int j = 0; j < SIZE; j++) a[j] = a_backup[j];

            uint64_t start = mach_absolute_time();
            add_neon(SIZE, a, b);
            uint64_t end = mach_absolute_time();
            total_neon += (end - start) * (double)timebase.numer / timebase.denom;
        }

        // Store results for this run
        scalar_times[run] = total_scalar / ITERS / 1e6;  // ms
        neon_times[run] = total_neon / ITERS / 1e6;
    }

    // Calculate final averages
    double avg_scalar = 0, avg_neon = 0;
    for (int i = 0; i < RUNS; i++) {
        avg_scalar += scalar_times[i];
        avg_neon += neon_times[i];
    }
    avg_scalar /= RUNS;
    avg_neon /= RUNS;
    double avg_speedup = avg_scalar / avg_neon;

    // Print individual run results
    printf("Benchmark Results (10M elements, 100 iterations/run):\n");
    for (int i = 0; i < RUNS; i++) {
        printf("Run %d: Scalar=%.2f ms, NEON=%.2f ms, Speedup=%.2fx\n",
               i + 1, scalar_times[i], neon_times[i],
               scalar_times[i] / neon_times[i]);
    }

    // Print final averages
    printf("\nAverage across %d runs:\n", RUNS);
    printf("Scalar: %.2f ms\n", avg_scalar);
    printf("NEON:   %.2f ms\n", avg_neon);
    printf("Speedup: %.2fx\n", avg_speedup);

    free(a);
    free(b);
    free(a_backup);
    return 0;
}
```

```bash
clang -O3 -arch arm64 -fno-vectorize -o ex1 ex1.c
./ex1
```

```
Benchmark Results (10M elements, 100 iterations/run):
Run 1: Scalar=4.79 ms, NEON=1.82 ms, Speedup=2.64x
Run 2: Scalar=4.75 ms, NEON=1.57 ms, Speedup=3.02x
Run 3: Scalar=4.76 ms, NEON=1.58 ms, Speedup=3.02x

Average across 3 runs:
Scalar: 4.76 ms
NEON:   1.65 ms
Speedup: 2.88x
```

### Conclusion
The vectorized implementation using ARM NEON SIMD instructions demonstrated a significant performance improvement over the scalar implementation, achieving an average speedup of 2.88x.

# Exercise II

Design an experiment to measure the speed up of this in numpy (and cupy it you have a GPU)

# Exercise III

While vectorization is powerful, please explain a situation when it may not be beneficial.
Hint 1 - Compiler support
Hint 2 - Vectorizability of Software
