#include <stdio.h>
#include <omp.h>

#define SIZE 100000

void serial_version(double *serial_time) {
    int a[SIZE];
    double start_time = omp_get_wtime();

    for (int i = 0; i < SIZE; i++) {
        a[i] = 2 * i + i; // Computes 3*i
    }

    double end_time = omp_get_wtime();
    *serial_time = end_time - start_time;
    printf("Serial Execution Time: %f seconds\n", *serial_time);
}

void parallel_version(double *parallel_time) {
    int a[SIZE];
    double start_time = omp_get_wtime();

    #pragma omp parallel for
    for (int i = 0; i < SIZE; i++) {
        a[i] = 2 * i + i;
    }

    double end_time = omp_get_wtime();
    *parallel_time = end_time - start_time;
    printf("Parallel Execution Time: %f seconds\n", *parallel_time);
}

int main(void) {
    int num_cores = omp_get_num_procs();
    printf("Number of CPU cores: %d\n", num_cores);

    double serial_time, parallel_time;

    printf("\nRunning Serial Version...\n");
    serial_version(&serial_time);

    printf("\nRunning Parallel Version...\n");
    parallel_version(&parallel_time);

    // Compute and print speedup
    if (parallel_time > 0) {
        double speedup = serial_time / parallel_time;
        printf("\nSpeedup: %.2fX\n", speedup);
    } else {
        printf("\nParallel execution time too small to compute speedup accurately.\n");
    }

    return 0;
}
