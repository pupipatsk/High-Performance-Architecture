#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>
#include <algorithm>

using namespace std;

const int MAX_N = 1005;

int n;
vector<vector<int>> graph;               // Use vector instead of set
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
    vector<bool> chosen(n, false);

    while (covered.count() < n) {
        int best = -1;
        int max_gain = -1;

        for (int i = 0; i < n; ++i) {
            if (chosen[i]) continue;

            bitset<MAX_N> diff = reach[i] & ~covered;
            int gain = diff.count();

            if (gain > max_gain) {
                best = i;
                max_gain = gain;
            }
        }

        if (best == -1) break;

        chosen[best] = true;
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
    vector<int> current = plants;
    vector<bool> keep(n, false);
    for (int p : current) keep[p] = true;

    for (int p : current) {
        keep[p] = false;

        bitset<MAX_N> covered;
        for (int i = 0; i < n; ++i) {
            if (keep[i]) covered |= reach[i];
        }

        if (covered.count() != n) keep[p] = true;
    }

    vector<int> result;
    for (int i = 0; i < n; ++i) {
        if (keep[i]) result.push_back(i);
    }

    return result;
}

void write_output(const string& path, const vector<int>& plants) {
    string out(n, '0');
    for (int p : plants) {
        out[p] = '1';
    }
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
