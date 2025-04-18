#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include <omp.h>

using namespace std;

const int MAX_N = 1024;
const int WORDS = (MAX_N + 63) / 64;

// Wrapper struct so we can use vector<Bitmask>
struct Bitmask {
    uint64_t data[WORDS] = {};
};

int n;
vector<vector<int>> graph;
vector<Bitmask> reach;

inline void bitmask_set(uint64_t* b, int i) {
    b[i / 64] |= (1ULL << (i % 64));
}

inline void bitmask_or(uint64_t* dst, const uint64_t* src) {
    for (int i = 0; i < WORDS; ++i) dst[i] |= src[i];
}

inline void bitmask_andnot(uint64_t* out, const uint64_t* a, const uint64_t* b) {
    for (int i = 0; i < WORDS; ++i) out[i] = a[i] & ~b[i];
}

inline int bitmask_count(const uint64_t* b) {
    int count = 0;
    for (int i = 0; i < WORDS; ++i)
        count += __builtin_popcountll(b[i]);
    return count;
}

inline void bitmask_clear(uint64_t* b) {
    for (int i = 0; i < WORDS; ++i) b[i] = 0;
}

void read_input(const string& path) {
    ifstream in(path);
    int e;
    in >> n >> e;
    graph.assign(n, {});

    for (int i = 0; i < e; ++i) {
        int a, b;
        in >> a >> b;
        graph[a].push_back(b);
        graph[b].push_back(a);
    }

    reach.resize(n);
    #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        bitmask_clear(reach[i].data);
        bitmask_set(reach[i].data, i);
        for (int j : graph[i]) {
            bitmask_set(reach[i].data, j);
        }
    }
}

vector<int> greedy_dominating_set() {
    uint64_t covered[WORDS] = {};
    vector<int> plants;
    vector<bool> used(n, false);

    while (bitmask_count(covered) < n) {
        int best = -1, max_gain = -1;

        #pragma omp parallel
        {
            int local_best = -1, local_gain = -1;
            uint64_t temp[WORDS];

            #pragma omp for nowait
            for (int i = 0; i < n; ++i) {
                if (used[i]) continue;
                bitmask_andnot(temp, reach[i].data, covered);
                int gain = bitmask_count(temp);
                if (gain > local_gain) {
                    local_gain = gain;
                    local_best = i;
                }
            }

            #pragma omp critical
            {
                if (local_gain > max_gain) {
                    max_gain = local_gain;
                    best = local_best;
                }
            }
        }

        if (best == -1) break;
        used[best] = true;
        plants.push_back(best);
        bitmask_or(covered, reach[best].data);
    }

    return plants;
}

bool is_valid(const vector<int>& plants) {
    uint64_t covered[WORDS] = {};
    for (int p : plants) bitmask_or(covered, reach[p].data);
    return bitmask_count(covered) == n;
}

vector<int> prune(const vector<int>& plants) {
    vector<bool> keep(n, false);
    for (int p : plants) keep[p] = true;

    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < (int)plants.size(); ++i) {
        int p = plants[i];
        keep[p] = false;

        uint64_t covered[WORDS] = {};
        for (int j = 0; j < n; ++j)
            if (keep[j]) bitmask_or(covered, reach[j].data);

        if (bitmask_count(covered) != n)
            keep[p] = true;
    }

    vector<int> result;
    for (int i = 0; i < n; ++i)
        if (keep[i]) result.push_back(i);
    return result;
}

void write_output(const string& path, const vector<int>& plants) {
    string out(n, '0');
    for (int p : plants) out[p] = '1';
    ofstream out_file(path);
    out_file << out << '\n';
}

int main(int argc, char** argv) {
    if (argc != 3) {
        cerr << "Usage: ./solve <input_file> <output_file>\n";
        return 1;
    }

    string input_file = argv[1];
    string output_file = argv[2];

    read_input(input_file);
    vector<int> greedy = greedy_dominating_set();
    vector<int> optimized = prune(greedy);
    write_output(output_file, optimized);

    return 0;
}
