// solve.cpp
#include <bits/stdc++.h>
using namespace std;

static const int SMALL_N  = 1000;    // threshold for bit‑prune greedy
static const int MEDIUM_N = 100000;  // threshold for degree‑greedy

// super‑fast integer reader
inline int readInt() {
    int x = 0, c = getchar_unlocked();
    while (c < '0' || c > '9') c = getchar_unlocked();
    for (; c >= '0' && c <= '9'; c = getchar_unlocked())
        x = x * 10 + (c - '0');
    return x;
}

int n, m;
vector<vector<int>> graph;

// 1) Small n ≤ SMALL_N: greedy + prune (dominating set)
vector<int> solve_small() {
    vector<char> covered(n, 0), chosen(n, 0);
    vector<int> plants;
    int uncovered = n;

    // Greedy selection
    while (uncovered > 0) {
        int best = -1, max_gain = -1;
        for (int i = 0; i < n; ++i) {
            if (chosen[i]) continue;
            int gain = !covered[i];
            for (int j : graph[i])
                if (!covered[j]) ++gain;
            if (gain > max_gain) {
                max_gain = gain;
                best = i;
            }
        }
        if (best < 0) break;
        chosen[best] = 1;
        plants.push_back(best);
        if (!covered[best]) { covered[best] = 1; --uncovered; }
        for (int j : graph[best])
            if (!covered[j]) { covered[j] = 1; --uncovered; }
    }

    // Prune redundant plants
    vector<char> keep(n, 0);
    for (int p : plants) keep[p] = 1;
    for (int p : plants) {
        keep[p] = 0;
        vector<char> cov(n, 0);
        int cnt = 0;
        for (int i = 0; i < n; ++i) if (keep[i]) {
            if (!cov[i]) { cov[i] = 1; ++cnt; }
            for (int j : graph[i])
                if (!cov[j]) { cov[j] = 1; ++cnt; }
        }
        if (cnt < n) keep[p] = 1;
    }

    vector<int> res;
    for (int i = 0; i < n; ++i)
        if (keep[i]) res.push_back(i);
    return res;
}

// 2) Medium n ≤ MEDIUM_N: sort by degree + one‑pass greedy
vector<int> solve_medium() {
    vector<pair<int,int>> deg(n);
    for (int i = 0; i < n; ++i)
        deg[i] = { (int)graph[i].size(), i };
    sort(deg.begin(), deg.end(), greater<>());

    vector<char> powered(n, 0);
    vector<int> plants;
    int remaining = n;
    for (auto &pr : deg) {
        if (remaining == 0) break;
        int u = pr.second;
        if (!powered[u]) {
            plants.push_back(u);
            powered[u] = 1; --remaining;
            for (int v : graph[u])
                if (!powered[v]) { powered[v] = 1; --remaining; }
        }
    }
    return plants;
}

// 3) Large n > MEDIUM_N: simple linear cover in one pass
vector<int> solve_large() {
    vector<char> powered(n, 0);
    vector<int> plants;
    plants.reserve(n);
    for (int i = 0; i < n; ++i) {
        if (!powered[i]) {
            plants.push_back(i);
            powered[i] = 1;
            for (int j : graph[i])
                powered[j] = 1;
        }
    }
    return plants;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input> <output>\n", argv[0]);
        return 1;
    }
    // redirect I/O
    freopen(argv[1], "r", stdin);
    freopen(argv[2], "w", stdout);

    n = readInt();
    m = readInt();
    graph.assign(n, {});

    // build adjacency
    for (int i = 0; i < m; ++i) {
        int u = readInt(), v = readInt();
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    // pick strategy by n
    vector<int> ans;
    if (n <= SMALL_N) {
        ans = solve_small();
    } else if (n <= MEDIUM_N) {
        ans = solve_medium();
    } else {
        ans = solve_large();
    }

    // output binary string
    string out(n, '0');
    for (int p : ans) out[p] = '1';
    out.push_back('\n');
    fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}
