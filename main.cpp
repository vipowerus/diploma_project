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

vector<string> all_vertexes = {"a1", "a2", "a3", "b1", "b2", "b3", "c1", "c2", "c3"};
deque<int> way;

int **build_adjacency_matrix(vector<string> base_path, int N) { // useless?
    int **Matrix = matrix_malloc(N);
    for (auto &path: base_path) {
        vector<string> st = split(path, ' ');
        int prev = -1;
        for (const string &str: st) {
            if (prev == -1) {
                prev = index_from_str(str);
                continue;
            }
            auto ind = index_from_str(str);
            Matrix[prev][ind] = 1; // номер параметра
            prev = ind;
        }
    }
//    matrix_output(Matrix, N);
    return Matrix;
}

bool check_way(const vector<string> &bounds) { // useless?
    string path;
    for (int i = 0; i < way.size(); ++i) {
        if (i == way.size() - 1) {
            path += all_vertexes[way[i]];
            break;
        }
        path += all_vertexes[way[i]] + " ";
    }
    for (auto &bound: bounds)
        if (path == bound) return false;
    return true;
}

void
make_suitable_paths(int **P, int start, int v, int W, const vector<string> &bounds, vector<string> &suitable_ways) { // useless?
    way.push_back(start);
    int adj_counter = 0;
    for (int i = 0; i < v; ++i)
        if (P[start][i]) {
            adj_counter++;
            make_suitable_paths(P, i, v, W, bounds, suitable_ways);
        }
    if (!adj_counter && check_way(bounds)) {
        string suitable_way;
        for (int i: way) {
            suitable_way += all_vertexes[i] + " ";
            cout << all_vertexes[i] << ' '; // все вершины + матрица = путь с параметрами
        }
        suitable_ways.push_back(suitable_way);
        cout << '\n';
    }
    way.pop_back();
}

vector<int> starting_vertexes(int **P, int M, int J) { // useless?
    vector<int> vertexes;
    for (int j = 0; j < J * M; ++j) { // !!!
        int not_zero = 0;
        for (int i = 0; i < J * M; ++i) { // !!!
            if (P[i][j] == 1) {
                not_zero++;
                break;
            }
        }
        if (not_zero == 0) vertexes.push_back(j);
    }
    return vertexes;
}

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

int main() {
    Model model;
    model.build_adjacency_matrix();
    model.make_suitable_paths();

    try {
        GRBEnv env = GRBEnv(true);
        env.set("LogFile", "result.json");
        env.set("OutputFlag", "0");
        env.start();

        // Create an empty model
        GRBModel m = GRBModel(env);

        GRBVar ** p = GRBVars_matrix_malloc(model.M, model.J);
        for (size_t i = 0; i < model.M; i++)
            for (size_t j = 0; j < model.J; j++)
                p[i][j] = m.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, indexes_pair_to_string({i, j}));
        GRBVar rho = m.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "rho");

        m.setObjective((GRBLinExpr) rho, GRB_MAXIMIZE);
        m.update();

        auto expressions = model.model.at("expressions").get<vector<vector<string>>>();
        for (auto expr : expressions){
            GRBLinExpr lhs_expr = make_constraint(expr.front(), m);
            m.addConstr(lhs_expr, expr[1].front(), stod(expr[2]), expr.back());
        }

        // m.addConstr(rho <= p[1][1] + p[0][1] + p[0][2], "P_1");
        GRBLinExpr lhs_expr = make_constraint(model.suitable_paths.front() + "- rho", m);
        m.addConstr(lhs_expr, '>', 0, "P_1");
        
        m.addConstr(rho <= p[0][1] + p[0][2] + p[1][2], "P_12");

        m.update();
        m.optimize();
        m.write("result.lp");

        cout << "PARAMS_START ..." << "\n";
        auto vars = m.getVars();
        for (int i = 0; i < m.get(GRB_IntAttr_NumVars); i++)
            cout << vars[i].get(GRB_StringAttr_VarName) << " " << vars[i].get(GRB_DoubleAttr_X) << '\n';

        auto constrs = m.getConstrs();
        for (int i = 0; i < m.get(GRB_IntAttr_NumConstrs); i++)
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