#include <arm_neon.h>
#include <chrono>
#include <iostream>
#include <vector>
#include <cstdlib>

void add(int size, int *a, int *b) {
    for (int i = 0; i < size; i++) {
        a[i] += b[i];
    }
}

void add_neon(int size, int *a, int *b) {
    int i = 0;
    for (; i <= size - 4; i += 4) {
        int32x4_t av = vld1q_s32(&a[i]); // Load 4 integers
        int32x4_t bv = vld1q_s32(&b[i]); // Load 4 integers
        av = vaddq_s32(av, bv);          // Add 4 integers
        vst1q_s32(&a[i], av);            // Store 4 integers
    }
    for (; i < size; i++) {
        a[i] += b[i];
    }
}

int main() {
    const int sizes[] = {1000000, 10000000, 100000000};
    for (int size : sizes) {
        std::vector<int> a(size), b(size);
        std::generate(a.begin(), a.end(), std::rand);
        std::generate(b.begin(), b.end(), std::rand);

        auto start = std::chrono::high_resolution_clock::now();
        add(size, a.data(), b.data());
        auto end = std::chrono::high_resolution_clock::now();
        double time_non_neon = std::chrono::duration<double>(end - start).count();

        std::generate(a.begin(), a.end(), std::rand); // Reset arrays
        start = std::chrono::high_resolution_clock::now();
        add_neon(size, a.data(), b.data());
        end = std::chrono::high_resolution_clock::now();
        double time_neon = std::chrono::duration<double>(end - start).count();

        std::cout << "Size: " << size << "\n";
        std::cout << "Non-NEON Time: " << time_non_neon << "s\n";
        std::cout << "NEON Time: " << time_neon << "s\n";
        std::cout << "Speedup: " << time_non_neon / time_neon << "x\n";
    }
    return 0;
}
