#include <bits/stdc++.h>
#include "ortools/sat/cp_model.h"
using namespace std;
using namespace operations_research::sat;

static const int BSZ = 1 << 20;
static char buf[BSZ];
int bp = BSZ, be = BSZ;

inline int readInt()
{
    if (bp == be)
        be = fread(buf, 1, BSZ, stdin), bp = 0;
    if (be == 0)
        return -1;
    int c, x = 0;
    while ((c = buf[bp++]) < '0')
        ;
    for (; c >= '0'; c = buf[bp++])
        x = x * 10 + (c - '0');
    return x;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
        return 1;
    }
    freopen(argv[1], "r", stdin);
    freopen(argv[2], "w", stdout);

    int num_nodes = readInt();
    int num_edges = readInt();
    vector<vector<int>> neighbors(num_nodes);

    for (int i = 0; i < num_edges; ++i)
    {
        int u = readInt(), v = readInt();
        neighbors[u].push_back(v);
        neighbors[v].push_back(u);
    }

    CpModelBuilder model;
    vector<BoolVar> x;
    x.reserve(num_nodes);
    for (int i = 0; i < num_nodes; ++i)
        x.push_back(model.NewBoolVar());

    for (int i = 0; i < num_nodes; ++i)
    {
        LinearExpr cover;
        cover += x[i];
        for (int j : neighbors[i])
            cover += x[j];
        model.AddGreaterOrEqual(cover, 1);
    }

    model.Minimize(LinearExpr::Sum(x));

    Model sat_model;
    sat_model.Add(NewSatParameters(
        "max_time_in_seconds:0.2 num_search_workers:12 linearization_level:1"));
    const CpSolverResponse response = SolveCpModel(model.Build(), &sat_model);

    if (response.status() == CpSolverStatus::OPTIMAL ||
        response.status() == CpSolverStatus::FEASIBLE)
    {
        for (int i = 0; i < num_nodes; ++i)
            fputc(SolutionIntegerValue(response, x[i]) ? '1' : '0', stdout);
    }
    else
    {
        for (int i = 0; i < num_nodes; ++i)
            fputc('0', stdout);
    }
    fputc('\n', stdout);
}
