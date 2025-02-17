#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ITERATIONS 3  // Number of times to run each test
#define SIZE1 100000 / 100
#define SIZE2 100000 // Default size
#define SIZE3 100000 * 100

void serial_version(int size, double *serial_time) {
    int *a = (int*)malloc(size * sizeof(int)); // Allocate memory for large arrays
    if (!a) {
        printf("Memory allocation failed for size %d\n", size);
        exit(1);
    }

    double start_time = omp_get_wtime();
    for (int i = 0; i < size; i++) {
        a[i] = 2 * i + i; // Computes 3*i
    }
    double end_time = omp_get_wtime();

    *serial_time = (end_time - start_time) * 1000.0; // Convert to milliseconds
    free(a); // Free memory
}

void parallel_version(int size, double *parallel_time) {
    int *a = (int*)malloc(size * sizeof(int)); // Allocate memory for large arrays
    if (!a) {
        printf("Memory allocation failed for size %d\n", size);
        exit(1);
    }

    double start_time = omp_get_wtime();
    #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        a[i] = 2 * i + i;
    }
    double end_time = omp_get_wtime();

    *parallel_time = (end_time - start_time) * 1000.0; // Convert to milliseconds
    free(a); // Free memory
}

void run_test(int size) {
    printf("\n=== Testing with SIZE = %d ===\n", size);

    double total_serial = 0.0, total_parallel = 0.0;

    for (int i = 0; i < ITERATIONS; i++) {
        double serial_time, parallel_time;

        serial_version(size, &serial_time);
        parallel_version(size, &parallel_time);

        total_serial += serial_time;
        total_parallel += parallel_time;

        printf("  Iteration %d: Serial = %.3f ms, Parallel = %.3f ms, Speedup = %.2fX\n",
               i + 1, serial_time, parallel_time, serial_time / parallel_time);
    }

    double avg_serial = total_serial / ITERATIONS;
    double avg_parallel = total_parallel / ITERATIONS;
    double avg_speedup = avg_serial / avg_parallel;

    printf("\n  >>> AVERAGE RESULTS for SIZE = %d <<<\n", size);
    printf("  Avg Serial Time: %.3f ms\n", avg_serial);
    printf("  Avg Parallel Time: %.3f ms\n", avg_parallel);
    printf("  Avg Speedup: %.2fX\n", avg_speedup);
}

int main(void) {
    int num_cores = omp_get_num_procs();
    printf("Number of CPU cores: %d\n", num_cores);

    run_test(SIZE1);
    run_test(SIZE2);
    run_test(SIZE3);

    return 0;
}
