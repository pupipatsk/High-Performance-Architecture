// #include <iostream>
// #include <fstream>
// #include <vector>
// #include <set>
// #include <algorithm>
// #include <cassert>
// #include <omp.h>
// #include <cstring>
// #include <stdexcept>

// using namespace std;

// class Graph {
//     int size_;
//     vector<set<int>> adj;
//     set<int> nodes;

// public:
//     Graph() : size_(0) {}
//     void add_edge(int u, int v) {
//         if ((int)adj.size() <= max(u, v)) adj.resize(max(u, v) + 1);
//         adj[u].insert(v);
//         adj[v].insert(u);
//         nodes.insert(u);
//         nodes.insert(v);
//         size_ = adj.size();
//     }
//     const set<int>& neighbors(int u) const {
//         return adj[u];
//     }
//     set<int> get_nodes() const {
//         return nodes;
//     }
//     int size() const {
//         return size_;
//     }
//     bool contains(int u) const {
//         return nodes.count(u) > 0;
//     }
//     void normalize() {
//         size_ = adj.size();
//     }
// };

// bool all(const vector<bool>& v) {
//     return all_of(v.begin(), v.end(), [](bool b) { return b; });
// }

// bool all(const vector<bool>& v1, const vector<bool>& v2) {
//     for (int i = 0; i < (int)v1.size(); ++i) {
//         if (!v1[i] && !v2[i]) return false;
//     }
//     return true;
// }

// int active = 1;
// int MAX_ACTIVE = 4;

// void backtrack(const Graph& graph, const vector<vector<bool>>& coverable,
//                const vector<int>& vertices, int idx, int no_erased,
//                int& min_erased, vector<bool>& erased, int n) {
//     if (all(erased)) {
//         #pragma omp critical
//         min_erased = min(min_erased, no_erased);
//         return;
//     }
//     if (idx == n || no_erased >= min_erased) return;
//     if (!all(erased, coverable[idx])) return;

//     int v = vertices[idx];
//     vector<bool> new_erased = erased;
//     for (int u : graph.neighbors(v)) {
//         new_erased[u] = true;
//     }

//     if ((active < MAX_ACTIVE) && (n - idx >= 20)) {
//         #pragma omp atomic
//         active++;
//         #pragma omp parallel sections
//         {
//             #pragma omp section
//             backtrack(graph, coverable, vertices, idx + 1, no_erased + 1, min_erased, new_erased, n);
//             #pragma omp section
//             backtrack(graph, coverable, vertices, idx + 1, no_erased, min_erased, erased, n);
//         }
//         #pragma omp atomic
//         active--;
//     } else {
//         backtrack(graph, coverable, vertices, idx + 1, no_erased + 1, min_erased, new_erased, n);
//         backtrack(graph, coverable, vertices, idx + 1, no_erased, min_erased, erased, n);
//     }
// }

// int dominating_set(const Graph& graph, vector<bool>& final_erased) {
//     int n = graph.size();
//     vector<pair<int, int>> aux(n);
//     for (int i = 0; i < n; ++i) {
//         aux[i] = {graph.neighbors(i).size(), i};
//     }
//     sort(aux.rbegin(), aux.rend());

//     vector<int> vertices(n);
//     for (int i = 0; i < n; ++i) vertices[i] = aux[i].second;

//     vector<vector<bool>> coverable(n, vector<bool>(n, false));
//     for (int i = n - 1; i >= 0; --i) {
//         coverable[i] = (i + 1 < n ? coverable[i + 1] : vector<bool>(n, false));
//         for (int u : graph.neighbors(vertices[i])) {
//             coverable[i][u] = true;
//         }
//     }

//     final_erased.assign(n, false);
//     int min_erased = n;
//     vector<bool> erased(n, false);
//     backtrack(graph, coverable, vertices, 0, 0, min_erased, erased, n);
//     final_erased = erased;
//     return min_erased;
// }

// vector<Graph> connected_components(Graph graph) {
//     vector<Graph> cc;
//     set<int> s = graph.get_nodes();
//     while (!s.empty()) {
//         Graph g;
//         set<int> t;
//         t.insert(*s.begin());
//         while (!t.empty()) {
//             int v = *t.begin();
//             t.erase(v);
//             s.erase(v);
//             g.add_edge(v, v);
//             for (int u : graph.neighbors(v)) {
//                 if (s.count(u)) {
//                     t.insert(u);
//                     g.add_edge(v, u);
//                 }
//             }
//         }
//         g.normalize();
//         cc.push_back(g);
//     }
//     return cc;
// }

// int main(int argc, char** argv) {
//     if (argc != 3) {
//         cerr << "Usage: ./solve <input_file> <output_file>\n";
//         return 1;
//     }

//     ifstream in(argv[1]);
//     if (!in) {
//         cerr << "Failed to open input file.\n";
//         return 1;
//     }

//     int n, m;
//     in >> n >> m;
//     Graph graph;
//     for (int i = 0; i < m; ++i) {
//         int u, v;
//         in >> u >> v;
//         graph.add_edge(u, v);
//     }
//     while (graph.size() < n) {
//         int k = graph.size();
//         if (!graph.contains(k))
//             graph.add_edge(k, k);  // add isolated node
//     }

//     graph.normalize();

//     // Set OMP
//     MAX_ACTIVE = 4;
//     omp_set_nested(1);

//     vector<bool> global_mask(n, false);
//     int total = 0;
//     for (const Graph& comp : connected_components(graph)) {
//         vector<bool> local_mask;
//         total += dominating_set(comp, local_mask);
//         const set<int>& nodes = comp.get_nodes();
//         for (int u : nodes) {
//             if (!local_mask[u]) global_mask[u] = true;
//         }
//     }

//     // Output solution
//     ofstream out(argv[2]);
//     for (int i = 0; i < n; ++i) out << (global_mask[i] ? '1' : '0');
//     out << '\n';

//     return 0;
// }
