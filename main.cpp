#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include "lib/json.hpp"
#include "gurobi_c++.h"
#include "rational.cpp"
#include "helpers.h"
#include "model.h"

using namespace std;
using json = nlohmann::json;

deque<int> way;

void rec_permutations(vector<int> arr, deque<vector<int>> permutations_stack, vector<vector<int>> &permutations) {
    auto perm = permutations_stack.back();
    permutations_stack.pop_back();
    auto ind_left = find(arr.begin(), arr.end(), perm.front());
    auto ind_right = find(arr.begin(), arr.end(), perm.back());
    do {
        if (!permutations_stack.empty()) rec_permutations(arr, permutations_stack, permutations);
        else {
            permutations.push_back(arr);
            for (int i: arr)
                cout << i;
            cout << '\n';
        }
    } while (std::next_permutation(ind_left, ind_right + 1));
}

void initialize_templates_arrays(vector<int> &machines, vector<int> &jobs, int M, int J) {
    for (int i = 0; i < M; ++i)
        machines.push_back(i + 1);
    for (int i = 0; i < J; ++i)
        jobs.push_back(i + 1);
}

void build_permutations(json templates, int M, int J, vector<vector<int>> &machines_permutations,
                          vector<vector<int>> &jobs_permutations) {
    vector<int> machines, jobs;
    initialize_templates_arrays(machines, jobs, M, J);
    deque<vector<int>> machines_perm_stack, jobs_perm_stack;

    auto machines_permutations_base = templates.at("Machines").get<vector<vector<int>>>();
    for (const auto &i: machines_permutations_base)
        machines_perm_stack.push_front(i);

    auto jobs_permutations_base = templates.at("Jobs").get<vector<vector<int>>>();
    for (const auto &i: jobs_permutations_base)
        jobs_perm_stack.push_front(i);

    rec_permutations(machines, machines_perm_stack, machines_permutations);
    rec_permutations(jobs, jobs_perm_stack, jobs_permutations);
}

string apply_permutation(string path, const vector<int> &machines_permutation, const vector<int> &jobs_permutation) {
    vector<string> operations = split(path, ' ');
    for (int i = 0; i < operations.size(); i++) {
        operations[i][0] = (char) (machines_permutation[operations[i][0] - 97] + 96);
        int current_job = stoi(operations[i].substr(1));
        operations[i].erase(1);
        int new_job = jobs_permutation[current_job - 1];
        operations[i].insert(1, to_string(new_job));
    }
    string result;
    for (const auto &s: operations) result += s + " ";
    return result;
}

GRBModel * create_GRBModel(const Model& model){
    GRBEnv env = GRBEnv(true);
    env.set("LogFile", "result.json");
    env.set("OutputFlag", "0");
    env.start();

    auto* m = new GRBModel(env);

    GRBVar ** p = GRBVars_matrix_malloc(model.M, model.J);
    for (size_t i = 0; i < model.M; i++)
        for (size_t j = 0; j < model.J; j++)
            p[i][j] = m->addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, indexes_pair_to_string({i, j}));
    GRBVar rho = m->addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "rho");

    m->setObjective((GRBLinExpr) rho, GRB_MAXIMIZE);
    m->update();
    return m;
}

int main() {
    Model model;
    model.build_adjacency_matrix();
    model.make_suitable_paths();

    try {
        auto GRB_instance = create_GRBModel(model);

        auto expressions = model.model.at("expressions").get<vector<vector<string>>>();
        for (auto expr : expressions){
            GRBLinExpr lhs_expr = make_constraint(expr.front(), GRB_instance);
            GRB_instance->addConstr(lhs_expr, expr[1].front(), stod(expr[2]), expr.back());
        }

        // m.addConstr(rho <= p[1][1] + p[0][1] + p[0][2], "P_1");
        GRBLinExpr lhs_expr = make_constraint(model.suitable_paths.front() + "- rho", GRB_instance);
        GRB_instance->addConstr(lhs_expr, '>', 0, "P_1");
        
        GRB_instance->addConstr(GRB_instance->getVarByName("rho") <= GRB_instance->getVarByName("a2")
            + GRB_instance->getVarByName("a3") + GRB_instance->getVarByName("b3"), "P_12");

        GRB_instance->update();
        GRB_instance->optimize();
        GRB_instance->write("result.lp");

        cout << "PARAMS_START ..." << "\n";
        auto vars = GRB_instance->getVars();
        for (int i = 0; i < GRB_instance->get(GRB_IntAttr_NumVars); i++)
            cout << vars[i].get(GRB_StringAttr_VarName) << " " << vars[i].get(GRB_DoubleAttr_X) << '\n';

        auto constrs = GRB_instance->getConstrs();
        for (int i = 0; i < GRB_instance->get(GRB_IntAttr_NumConstrs); i++)
            if (constrs[i].get(GRB_IntAttr_CBasis) == -1)
                cout << constrs[i].get(GRB_StringAttr_ConstrName) << " " << constrs[i].get(GRB_DoubleAttr_Pi) << '\n';

    } catch (GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch (...) {
        cout << "Exception during optimization" << endl;
    }
    return 0;
}