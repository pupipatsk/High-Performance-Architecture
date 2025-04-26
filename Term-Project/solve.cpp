#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include "ortools/linear_solver/linear_solver.h"

using namespace operations_research;

void solve_power_plant_problem(const std::string& input_file, const std::string& output_file) {
    // Step 1: Read input
    std::ifstream fin(input_file);
    int num_nodes, num_edges;
    fin >> num_nodes >> num_edges;

    std::vector<std::vector<int>> neighbors(num_nodes);
    for (int i = 0; i < num_edges; ++i) {
        int u, v;
        fin >> u >> v;
        neighbors[u].push_back(v);
        neighbors[v].push_back(u);
    }
    fin.close();

    // Step 2: Create solver
    std::unique_ptr<MPSolver> solver(MPSolver::CreateSolver("SCIP"));
    if (!solver) {
        std::cerr << "Could not create solver SCIP." << std::endl;
        return;
    }

    // Step 3: Variables: x[i] = 1 if city i has a power plant
    std::vector<const MPVariable*> x(num_nodes);
    for (int i = 0; i < num_nodes; ++i) {
        x[i] = solver->MakeBoolVar("x_" + std::to_string(i));
    }

    // Step 4: Constraints: Each city must be powered
    for (int i = 0; i < num_nodes; ++i) {
        MPConstraint* ct = solver->MakeRowConstraint(1, solver->infinity());
        ct->SetCoefficient(x[i], 1.0);
        for (int neighbor : neighbors[i]) {
            ct->SetCoefficient(x[neighbor], 1.0);
        }
    }

    // Step 5: Objective: minimize total number of power plants
    MPObjective* objective = solver->MutableObjective();
    for (int i = 0; i < num_nodes; ++i) {
        objective->SetCoefficient(x[i], 1.0);
    }
    objective->SetMinimization();

    // Step 6: Solve
    const MPSolver::ResultStatus result_status = solver->Solve();

    // Step 7: Output
    if (result_status == MPSolver::OPTIMAL) {
        std::ofstream fout(output_file);
        for (int i = 0; i < num_nodes; ++i) {
            fout << (x[i]->solution_value() > 0.5 ? '1' : '0');
        }
        fout << std::endl;
        fout.close();
    } else {
        std::cerr << "No optimal solution found!" << std::endl;
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " input_file output_file" << std::endl;
        return 1;
    }
    solve_power_plant_problem(argv[1], argv[2]);
    return 0;
}
