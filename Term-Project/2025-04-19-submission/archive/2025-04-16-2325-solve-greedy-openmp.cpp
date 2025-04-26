#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>
#include <algorithm>
#include <omp.h>

using namespace std;

const int MAX_N = 1000;

int n;
vector<vector<int>> graph;
vector<bitset<MAX_N>> reach;

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

    reach.assign(n, {});
    #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        reach[i].set(i);
        for (int j : graph[i]) {
            reach[i].set(j);
        }
    }
}

vector<int> greedy_dominating_set() {
    bitset<MAX_N> covered;
    vector<int> plants;
    vector<bool> used(n, false);

    while (covered.count() < n) {
        int best = -1, max_gain = -1;

        #pragma omp parallel
        {
            int local_best = -1;
            int local_gain = -1;

            #pragma omp for nowait
            for (int i = 0; i < n; ++i) {
                if (used[i]) continue;
                bitset<MAX_N> diff = reach[i] & ~covered;
                int gain = diff.count();
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
        covered |= reach[best];
    }

    return plants;
}

bool is_valid(const vector<int>& plants) {
    bitset<MAX_N> covered;
    for (int p : plants) {
        covered |= reach[p];
    }
    return covered.count() == n;
}

vector<int> prune(const vector<int>& plants) {
    vector<bool> keep(n, false);
    for (int p : plants) keep[p] = true;

    #pragma omp parallel for
    for (int i = 0; i < (int)plants.size(); ++i) {
        int p = plants[i];
        keep[p] = false;

        bitset<MAX_N> covered;
        for (int j = 0; j < n; ++j) {
            if (keep[j]) covered |= reach[j];
        }

        if (covered.count() != n) {
            keep[p] = true;
        }
    }

    vector<int> result;
    for (int i = 0; i < n; ++i) {
        if (keep[i]) result.push_back(i);
    }
    return result;
}

void write_output(const string& path, const vector<int>& plants) {
    string out(n, '0');
    for (int p : plants) out[p] = '1';
    ofstream out_file(path);
    out_file << out << endl;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        cerr << "Usage: ./solve <input_file> <output_file>" << endl;
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
