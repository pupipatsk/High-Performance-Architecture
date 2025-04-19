#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <cassert>

using namespace std;

int n, m;
vector<vector<int>> graph;

// Input reader
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

// Small graph: greedy + prune
vector<int> greedy_prune() {
    vector<bool> covered(n, false);
    vector<bool> chosen(n, false);
    vector<int> plants;
    int uncovered = n;

    while (uncovered > 0) {
        int best = -1, max_gain = -1;
        for (int i = 0; i < n; ++i) {
            if (chosen[i]) continue;

            int gain = 0;
            if (!covered[i]) gain++;
            for (int j : graph[i]) {
                if (!covered[j]) gain++;
            }

            if (gain > max_gain) {
                max_gain = gain;
                best = i;
            }
        }

        if (best == -1) break;

        chosen[best] = true;
        plants.push_back(best);
        if (!covered[best]) {
            covered[best] = true;
            uncovered--;
        }
        for (int j : graph[best]) {
            if (!covered[j]) {
                covered[j] = true;
                uncovered--;
            }
        }
    }

    // Prune
    vector<bool> keep(n, false);
    for (int p : plants) keep[p] = true;

    for (int p : plants) {
        keep[p] = false;

        vector<bool> covered(n, false);
        for (int i = 0; i < n; ++i) {
            if (keep[i]) {
                covered[i] = true;
                for (int j : graph[i]) {
                    covered[j] = true;
                }
            }
        }

        bool valid = all_of(covered.begin(), covered.end(), [](bool x) { return x; });
        if (!valid) keep[p] = true;
    }

    vector<int> result;
    for (int i = 0; i < n; ++i) {
        if (keep[i]) result.push_back(i);
    }

    return result;
}

// Medium graph: degree-based greedy
vector<int> degree_greedy() {
    vector<bool> powered(n, false);
    vector<int> plants;
    int remaining = n;

    vector<pair<int, int>> degree(n);
    for (int i = 0; i < n; ++i)
        degree[i] = {int(graph[i].size()), i};
    sort(degree.rbegin(), degree.rend());

    for (auto [deg, i] : degree) {
        if (powered[i]) continue;

        plants.push_back(i);
        if (!powered[i]) {
            powered[i] = true;
            remaining--;
        }
        for (int j : graph[i]) {
            if (!powered[j]) {
                powered[j] = true;
                remaining--;
            }
        }

        if (remaining <= 0) break;
    }

    return plants;
}

// Large graph: place plant at any unpowered node
vector<int> fast_linear_cover() {
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

    read_input(argv[1]);

    vector<int> result;

    if (n <= 1000) {
        result = greedy_prune();
    } else if (n <= 100000) {
        result = degree_greedy();
    } else {
        result = fast_linear_cover();
    }

    write_output(argv[2], result);
    return 0;
}