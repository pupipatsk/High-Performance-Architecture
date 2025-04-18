#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include "ortools/linear_solver/linear_solver.h"

using namespace operations_research;

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: ./solve input.txt output.txt\n";
        return 1;
    }

    std::ifstream infile(argv[1]);
    std::ofstream outfile(argv[2]);

    int n, m;
    infile >> n >> m;
    std::vector<std::set<int>> adj(n);
    for (int i = 0; i < m; ++i) {
        int u, v;
        infile >> u >> v;
        adj[u].insert(v);
        adj[v].insert(u);
    }

    // Create solver
    std::unique_ptr<MPSolver> solver(MPSolver::CreateSolver("CBC"));
    if (!solver) {
        std::cerr << "Solver not available.\n";
        return 1;
    }

    std::vector<const MPVariable*> vars(n);
    for (int i = 0; i < n; ++i) {
        vars[i] = solver->MakeBoolVar("x_" + std::to_string(i));
    }

    // Constraints: for each city, either it or one of its neighbors must be powered
    for (int i = 0; i < n; ++i) {
        LinearExpr coverage;
        coverage += vars[i];
        for (int neighbor : adj[i]) {
            coverage += vars[neighbor];
        }
        solver->MakeRowConstraint(coverage >= 1);
    }

    // Objective: minimize the number of power plants
    LinearExpr obj;
    for (int i = 0; i < n; ++i) {
        obj += vars[i];
    }
    solver->Minimize(obj);

    const MPSolver::ResultStatus result_status = solver->Solve();
    if (result_status != MPSolver::OPTIMAL) {
        std::cerr << "No optimal solution found.\n";
        return 1;
    }

    // Output: binary string
    for (int i = 0; i < n; ++i) {
        outfile << (vars[i]->solution_value() > 0.5 ? '1' : '0');
    }
    outfile << "\n";
    return 0;
}
