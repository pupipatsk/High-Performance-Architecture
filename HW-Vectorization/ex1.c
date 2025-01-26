#include <stdio.h>
#include <stdlib.h>
#include <arm_neon.h>
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

    // Calculate results
    double mean_scalar = total_scalar / ITERS / 1e6;  // ms
    double mean_neon = total_neon / ITERS / 1e6;
    double speedup = mean_scalar / mean_neon;

    printf("Results (%d elements, %d iterations):\n", SIZE, ITERS);
    printf("Scalar: %.2f ms\n", mean_scalar);
    printf("NEON:   %.2f ms\n", mean_neon);
    printf("Speedup: %.2fx\n", speedup);

    free(a);
    free(b);
    free(a_backup);
    return 0;
}