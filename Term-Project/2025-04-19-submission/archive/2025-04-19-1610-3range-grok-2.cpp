#include <bits/stdc++.h>
#include <omp.h>
using namespace std;

// Compiler optimizations
#pragma GCC optimize("Ofast", "unroll-loops", "inline")
// #pragma GCC target("popcnt", "avx2")

// Thresholds
const int SMALL_N = 1000;
const int MEDIUM_N = 100000;

// Fast buffered reader
static const size_t BUF_SZ = 1 << 20;
static char buf[BUF_SZ];
static size_t buf_pos = 0, buf_len = 0;

static inline void refill() {
    buf_len = fread(buf, 1, BUF_SZ, stdin);
    buf_pos = 0;
    if (!buf_len) buf[0] = EOF;
}

static inline int readInt() {
    int x = 0;
    char c;
    if (buf_pos >= buf_len) refill();
    c = buf[buf_pos++];
    while (c < '0' || c > '9') {
        if (buf_pos >= buf_len) refill();
        c = buf[buf_pos++];
    }
    for (; c >= '0' && c <= '9'; c = (buf_pos < buf_len ? buf[buf_pos++] : EOF))
        x = x * 10 + (c - '0');
    return x;
}

// Global variables
int n, m;
vector<vector<int>> graph;

// Small graph solver: Bitset-based greedy with pruning
vector<int> solve_small() {
    const int B = (n + 63) >> 6; // Number of 64-bit words
    vector<uint64_t> reach(n * B, 0);
    vector<uint64_t> covered(B, 0);
    vector<bool> chosen(n, false);
    vector<int> plants;

    // Precompute reach sets in parallel
    #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        int base = i * B;
        reach[base + (i >> 6)] |= (1ULL << (i & 63));
        for (int j : graph[i]) {
            reach[base + (j >> 6)] |= (1ULL << (j & 63));
        }
    }

    int covered_cnt = 0;
    while (covered_cnt < n) {
        int best = -1;
        int max_gain = -1;
        #pragma omp parallel for reduction(max : max_gain, best)
        for (int i = 0; i < n; ++i) {
            if (chosen[i]) continue;
            int gain = 0;
            int base = i * B;
            #pragma GCC unroll 4
            for (int k = 0; k < B; ++k) {
                uint64_t diff = reach[base + k] & ~covered[k];
                gain += __builtin_popcountll(diff);
            }
            if (gain > max_gain) {
                max_gain = gain;
                best = i;
            }
        }
        if (best == -1) break;
        chosen[best] = true;
        plants.push_back(best);
        int base = best * B;
        #pragma GCC unroll 4
        for (int k = 0; k < B; ++k) {
            uint64_t diff = reach[base + k] & ~covered[k];
            covered[k] |= reach[base + k];
            covered_cnt += __builtin_popcountll(diff);
        }
    }

    // Pruning
    vector<bool> keep(n, true);
    for (int p : plants) {
        keep[p] = false;
        vector<uint64_t> temp_covered(B, 0);
        int cnt = 0;
        for (int i = 0; i < n; ++i) {
            if (keep[i]) {
                int base = i * B;
                #pragma GCC unroll 4
                for (int k = 0; k < B; ++k) {
                    temp_covered[k] |= reach[base + k];
                }
            }
        }
        #pragma GCC unroll 4
        for (int k = 0; k < B; ++k) {
            cnt += __builtin_popcountll(temp_covered[k]);
        }
        if (cnt < n) keep[p] = true;
    }

    vector<int> result;
    result.reserve(plants.size());
    for (int i = 0; i < n; ++i) {
        if (keep[i]) result.push_back(i);
    }
    return result;
}

// Medium graph solver: Degree-based greedy with priority queue
vector<int> solve_medium() {
    vector<int> degree(n, 0);
    #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        degree[i] = graph[i].size();
    }
    auto comp = [&degree](int a, int b) { return degree[a] < degree[b]; };
    priority_queue<int, vector<int>, decltype(comp)> pq(comp);
    for (int i = 0; i < n; ++i) {
        pq.push(i);
    }

    vector<bool> powered(n, false);
    vector<int> plants;
    plants.reserve(n / 10);

    while (!pq.empty()) {
        int u = pq.top();
        pq.pop();
        if (powered[u]) continue;
        plants.push_back(u);
        powered[u] = true;
        for (int v : graph[u]) {
            if (!powered[v]) {
                powered[v] = true;
                for (int w : graph[v]) {
                    if (!powered[w]) {
                        degree[w]--;
                        pq.push(w);
                    }
                }
            }
        }
    }
    return plants;
}

// Large graph solver: Fast linear cover
vector<int> solve_large() {
    vector<bool> powered(n, false);
    vector<int> plants;
    plants.reserve(n / 10);
    for (int i = 0; i < n; ++i) {
        if (!powered[i]) {
            plants.push_back(i);
            powered[i] = true;
            for (int j : graph[i]) {
                powered[j] = true;
            }
        }
    }
    return plants;
}

// Write output
void write_output(const string& path, const vector<int>& plants) {
    string out(n, '0');
    for (int p : plants) {
        out[p] = '1';
    }
    ofstream out_file(path);
    out_file << out << '\n';
}

int main(int argc, char** argv) {
    if (argc != 3) {
        cerr << "Usage: ./solve <input_file> <output_file>" << endl;
        return 1;
    }

    string input_file = argv[1];
    string output_file = argv[2];

    // Read input
    n = readInt();
    m = readInt();
    graph.assign(n, {});
    for (int i = 0; i < m; ++i) {
        int a = readInt(), b = readInt();
        graph[a].push_back(b);
        graph[b].push_back(a);
    }

    vector<int> result;
    if (n <= SMALL_N) {
        result = solve_small();
    } else if (n <= MEDIUM_N) {
        result = solve_medium();
    } else {
        result = solve_large();
    }

    write_output(output_file, result);
    return 0;
}