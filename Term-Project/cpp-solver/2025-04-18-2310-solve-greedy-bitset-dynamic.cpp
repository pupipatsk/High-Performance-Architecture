#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>

using namespace std;

int n;
vector<vector<int>> graph;
vector<vector<int>> reach;

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
        unordered_set<int> s;
        s.insert(i);
        for (int j : graph[i]) {
            s.insert(j);
        }
        reach[i].assign(s.begin(), s.end());
    }
}

vector<int> greedy_dominating_set() {
    vector<bool> covered(n, false);
    vector<bool> chosen(n, false);
    vector<int> plants;
    int uncovered = n;

    while (uncovered > 0) {
        int best = -1, max_gain = -1;

        for (int i = 0; i < n; ++i) {
            if (chosen[i]) continue;

            int gain = 0;
            for (int j : reach[i]) {
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
        for (int j : reach[best]) {
            if (!covered[j]) {
                covered[j] = true;
                --uncovered;
            }
        }
    }

    return plants;
}

bool is_valid(const vector<int>& plants) {
    vector<bool> covered(n, false);
    for (int p : plants) {
        for (int j : reach[p]) {
            covered[j] = true;
        }
    }

    for (bool ok : covered) {
        if (!ok) return false;
    }
    return true;
}

vector<int> prune(const vector<int>& plants) {
    vector<bool> keep(n, false);
    for (int p : plants) keep[p] = true;

    for (int p : plants) {
        keep[p] = false;

        vector<bool> covered(n, false);
        for (int i = 0; i < n; ++i) {
            if (keep[i]) {
                for (int j : reach[i]) {
                    covered[j] = true;
                }
            }
        }

        bool valid = true;
        for (bool c : covered) {
            if (!c) {
                valid = false;
                break;
            }
        }

        if (!valid) keep[p] = true;
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
    vector<int> greedy = greedy_dominating_set();
    vector<int> optimized = prune(greedy);
    write_output(output_file, optimized);

    return 0;
}