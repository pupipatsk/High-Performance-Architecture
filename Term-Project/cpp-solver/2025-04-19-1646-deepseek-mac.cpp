// solve_mac.cpp
// Optimized with mmap I/O, OpenMP, Bucket Sort, and cache-aware structures
// Removed x86-specific target pragma for broader compatibility (incl. Apple Silicon)
#pragma GCC optimize("Ofast", "unroll-loops", "inline")
// #pragma GCC target("avx2", "popcnt", "bmi2") // <-- REMOVED for Apple Silicon compatibility

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <omp.h>

using namespace std;

const int SMALL_N = 1000;
const int MEDIUM_N = 100000;

// MMap reader
struct MMap
{
    char *data;
    size_t size;
    size_t pos;

    MMap(const char *fname) : data(nullptr), size(0), pos(0)
    {
        int fd = open(fname, O_RDONLY);
        if (fd == -1)
        {
            perror("open");
            return;
        }
        struct stat sb;
        if (fstat(fd, &sb) == -1)
        {
            perror("fstat");
            close(fd);
            return;
        }
        size = sb.st_size;
        if (size == 0)
        {
            fprintf(stderr, "Warning: Input file is empty.\n");
            close(fd);
            return;
        }
        data = (char *)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data == MAP_FAILED)
        {
            perror("mmap");
            data = nullptr;
        }
        close(fd);
    }

    ~MMap()
    {
        if (data)
            munmap(data, size);
    }

    bool isValid() const { return data != nullptr; }

    int readInt()
    {
        if (!data || pos >= size)
            return 0;
        int x = 0;
        while (pos < size && (data[pos] < '0' || data[pos] > '9'))
            pos++;
        while (pos < size && data[pos] >= '0' && data[pos] <= '9')
            x = x * 10 + (data[pos++] - '0');
        return x;
    }
};

// Adjacency storage
static int *head = nullptr, *nxt = nullptr, *to = nullptr, edge_cnt = 0;

inline void addEdge(int u, int v)
{
    nxt[edge_cnt] = head[u];
    to[edge_cnt] = v;
    head[u] = edge_cnt++;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input> <output>\n", argv[0]);
        return 1;
    }

    MMap mmap(argv[1]);
    if (!mmap.isValid())
    {
        fprintf(stderr, "Failed to mmap input or file is invalid/empty.\n");
        return 1;
    }

    int n = mmap.readInt();
    int m = mmap.readInt();

    if (n <= 0 || m < 0)
    {
        fprintf(stderr, "Invalid n or m (n=%d, m=%d). n must be > 0, m must be >= 0.\n", n, m);
        return 1;
    }

    size_t alignment = 64;
    head = (int *)aligned_alloc(alignment, n * sizeof(int));
    if (!head)
    {
        perror("aligned_alloc head");
        return 1;
    }
    fill(head, head + n, -1);

    size_t edge_array_size = (size_t)2 * m;
    if (m > 0 && edge_array_size / 2 != (size_t)m)
    {
        fprintf(stderr, "Error: m is too large, potential overflow for edge arrays.\n");
        free(head);
        return 1;
    }

    if (m > 0)
    {
        nxt = (int *)aligned_alloc(alignment, edge_array_size * sizeof(int));
        to = (int *)aligned_alloc(alignment, edge_array_size * sizeof(int));
        if (!nxt || !to)
        {
            perror("aligned_alloc edges");
            free(head);
            free(nxt);
            free(to);
            return 1;
        }
    }
    else
    {
        nxt = nullptr;
        to = nullptr;
    }

    edge_cnt = 0;
    vector<int> deg(n, 0);
    for (int i = 0; i < m; ++i)
    {
        int u = mmap.readInt();
        int v = mmap.readInt();
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            fprintf(stderr, "Error: Edge %d (%d, %d) out of bounds for n=%d.\n", i, u, v, n);
            free(head);
            free(nxt);
            free(to);
            return 1;
        }
        addEdge(u, v);
        deg[u]++;
        addEdge(v, u);
        deg[v]++;
    }

    vector<int> solution;
    solution.reserve(n / 5 + 1);

    if (n <= SMALL_N)
    {
        const int B = (n + 63) / 64;
        vector<vector<uint64_t>> reach(n, vector<uint64_t>(B, 0));

        for (int i = 0; i < n; ++i)
        {
            reach[i][i / 64] |= 1ULL << (i % 64);
            if (head)
            {
                for (int e = head[i]; e != -1; e = nxt[e])
                {
                    int j = to[e];
                    if (j >= 0 && j < n)
                        reach[i][j / 64] |= 1ULL << (j % 64);
                }
            }
        }

        vector<uint64_t> covered(B, 0);
        vector<bool> chosen(n, false);
        int covered_cnt = 0;

        while (covered_cnt < n)
        {
            int best = -1, max_gain = -1;

#pragma omp parallel
            {
                int local_best = -1, local_max = -1;
#pragma omp for nowait
                for (int i = 0; i < n; ++i)
                {
                    if (chosen[i])
                        continue;
                    int gain = 0;
                    for (int k = 0; k < B; ++k)
                    {
                        uint64_t diff = reach[i][k] & ~covered[k];
                        gain += __builtin_popcountll(diff);
                    }
                    if (gain > local_max || (gain == local_max && (local_best == -1 || i < local_best)))
                    {
                        local_max = gain;
                        local_best = i;
                    }
                }
#pragma omp critical
                {
                    if (local_max > max_gain || (local_max == max_gain && (best == -1 || local_best < best)))
                    {
                        max_gain = local_max;
                        best = local_best;
                    }
                }
            }

            if (best == -1)
                break;
            chosen[best] = true;
            solution.push_back(best);

#pragma omp parallel for
            for (int k = 0; k < B; ++k)
            {
                uint64_t diff = reach[best][k] & ~covered[k];
                if (diff)
                {
#pragma omp atomic
                    covered[k] |= reach[best][k];
#pragma omp atomic
                    covered_cnt += __builtin_popcountll(diff);
                }
            }
        }

        vector<bool> keep(n, false);
        for (int p : solution)
            if (p >= 0 && p < n)
                keep[p] = true;

        vector<int> current_solution = solution;
        for (int p : current_solution)
        {
            if (p < 0 || p >= n)
                continue;
            keep[p] = false;
            vector<uint64_t> temp_covered(B, 0);
            int temp_covered_nodes = 0;
            for (int i = 0; i < n; ++i)
            {
                if (keep[i])
                {
                    for (int k = 0; k < B; ++k)
                        temp_covered[k] |= reach[i][k];
                }
            }
            for (int k = 0; k < B; ++k)
                temp_covered_nodes += __builtin_popcountll(temp_covered[k]);
            if (temp_covered_nodes < n)
                keep[p] = true;
        }

        solution.clear();
        for (int i = 0; i < n; ++i)
            if (keep[i])
                solution.push_back(i);
    }
    else if (n <= MEDIUM_N)
    {
        int max_deg = !deg.empty() ? *max_element(deg.begin(), deg.end()) : 0;
        vector<vector<int>> buckets(max_deg + 1);
        for (int i = 0; i < n; ++i)
            if (deg[i] >= 0 && deg[i] <= max_deg)
                buckets[deg[i]].push_back(i);

        vector<char> powered(n, 0);
        int rem = n;
        for (int d = max_deg; d >= 0 && rem > 0; --d)
        {
            for (int u : buckets[d])
            {
                if (!powered[u])
                {
                    solution.push_back(u);
                    powered[u] = 1;
                    rem--;
                    if (head)
                    {
                        for (int e = head[u]; e != -1; e = nxt[e])
                        {
                            int v = to[e];
                            if (v >= 0 && v < n && !powered[v])
                            {
                                powered[v] = 1;
                                rem--;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        vector<char> powered(n, 0);
        for (int i = 0; i < n; ++i)
        {
            if (!powered[i])
            {
                solution.push_back(i);
                powered[i] = 1;
                if (head)
                {
                    for (int e = head[i]; e != -1; e = nxt[e])
                    {
                        int neighbor = to[e];
                        if (neighbor >= 0 && neighbor < n)
                            powered[neighbor] = 1;
                    }
                }
            }
        }
    }

    string out(n, '0');
    for (int p : solution)
        if (p >= 0 && p < n)
            out[p] = '1';
    out += '\n';

    FILE *fo = fopen(argv[2], "wb");
    if (!fo)
    {
        perror("fopen output");
        free(head);
        free(nxt);
        free(to);
        return 1;
    }
    fwrite(out.data(), 1, out.size(), fo);
    fclose(fo);

    free(head);
    free(nxt);
    free(to);

    return 0;
}
