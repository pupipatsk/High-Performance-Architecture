#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <bitset>

using namespace std;

const int MAX_N = 1005; // max number of cities

int n;
vector<set<int>> graph;
vector<bitset<MAX_N>> reach;

void read_input(const string& path) {
    ifstream in(path);
    int e;
    in >> n >> e;
    graph.assign(n, {});

    for (int i = 0; i < e; ++i) {
        int a, b;
        in >> a >> b;
        graph[a].insert(b);
        graph[b].insert(a);
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

    while (covered.count() < n) {
        int best = -1, max_gain = -1;
        for (int i = 0; i < n; ++i) {
            bitset<MAX_N> diff = reach[i] & ~covered;
            int gain = diff.count();
            if (gain > max_gain) {
                max_gain = gain;
                best = i;
            }
        }
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
    for (int p : plants) {
        vector<int> test;
        for (int q : current) if (q != p) test.push_back(q);
        if (is_valid(test)) current = test;
    }
    return current;
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
