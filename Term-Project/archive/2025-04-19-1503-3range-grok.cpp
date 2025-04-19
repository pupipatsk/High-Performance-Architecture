#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <bitset>
#include <omp.h>

using namespace std;

// Thresholds for different strategies
const int SMALL_N = 1000;    // Use greedy with bitsets and pruning
const int MEDIUM_N = 100000; // Use degree-based greedy

// Global variables
int n, m;
vector<vector<int>> graph;

// Read input file
void read_input(const string& path) {
    ifstream in(path);
    in >> n >> m;
    graph.assign(n, {});
    for (int i = 0; i < m; ++i) {
        int a, b;
        in >> a >> b;
        graph[a].push_back(b);
        graph[b].push_back(a);
    }
}

// Solver for small graphs (n <= 1000): Greedy with bitsets and pruning
vector<int> solve_small() {
    const int MAX_N = 1005; // Slightly above SMALL_N to be safe
    vector<bitset<MAX_N>> reach(n);

    // Precompute reach sets
    #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        reach[i].set(i);
        for (int j : graph[i]) {
            reach[i].set(j);
        }
    }

    bitset<MAX_N> covered;
    vector<int> plants;

    // Greedy selection
    while (covered.count() < n) {
        vector<int> gains(n, 0);
        #pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            bitset<MAX_N> diff = reach[i] & ~covered;
            gains[i] = diff.count();
        }

        int best = -1;
        int max_gain = -1;
        for (int i = 0; i < n; ++i) {
            if (gains[i] > max_gain) {
                max_gain = gains[i];
                best = i;
            }
        }
        if (best == -1) break;
        plants.push_back(best);
        covered |= reach[best];
    }

    // Pruning to minimize plants
    vector<bool> keep(n, true);
    for (int p : plants) {
        keep[p] = false;
        bitset<MAX_N> temp_covered;
        for (int i = 0; i < n; ++i) {
            if (keep[i]) temp_covered |= reach[i];
        }
        if (temp_covered.count() != n) keep[p] = true;
    }

    vector<int> result;
    for (int i = 0; i < n; ++i) {
        if (keep[i]) result.push_back(i);
    }
    return result;
}

// Solver for medium graphs (1000 < n <= 100000): Degree-based greedy
vector<int> solve_medium() {
    vector<pair<int, int>> degree(n);
    for (int i = 0; i < n; ++i) {
        degree[i] = {graph[i].size(), i};
    }
    sort(degree.rbegin(), degree.rend());

    vector<bool> powered(n, false);
    vector<int> plants;

    for (auto [deg, i] : degree) {
        if (powered[i]) continue;
        plants.push_back(i);
        powered[i] = true;
        for (int j : graph[i]) {
            powered[j] = true;
        }
    }

    return plants;
}

// Solver for large graphs (n > 100000): Fast linear cover
vector<int> solve_large() {
    vector<bool> powered(n, false);
    vector<int> plants;

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

// Write output file
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

    read_input(input_file);

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