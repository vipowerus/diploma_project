//
// Created by chechushkov on 23.02.2022.
//

#include <fstream>
#include "model.h"
#include "helpers.h"

using namespace std;

Model::Model() {
    std::ifstream model_file("../model.json");
    std::ifstream templates_file("../templates.json");
    model_file >> this->model;
    templates_file >> this->templates;

    this->M = model.at("M").get<int>();
    this->J = model.at("J").get<int>();
    this->base_path = templates.at("base").get<std::vector<std::string>>();
    this->bounds = model.at("bounds").get<std::vector<std::string>>();
    this->P = nullptr;

    if (model.contains("work_vertexes")) all_vertexes = split(model.at("work_vertexes").get<string>(), ' ');
    else if (model.contains("missed_vertexes")) build_all_vertexes_from_missed_vertexes(model.at("missed_vertexes").get<string>());
    else {
        cout << "Input vertexes are wrong. Exit...";
        exit(1);
    }

    create_GRBModel();
}

void Model::create_GRBModel(){
    GRBEnv env = GRBEnv(true);
    env.set("LogFile", "result.json");
    env.set("OutputFlag", "0");
    env.start();

    GRB_instance = new GRBModel(env);

    GRBVar ** p = GRBVars_matrix_malloc(M, J);
    for (size_t i = 0; i < M; i++)
        for (size_t j = 0; j < J; j++)
            p[i][j] = GRB_instance->addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, indexes_pair_to_string({i, j}));
    GRBVar rho = GRB_instance->addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "rho");

    GRB_instance->setObjective((GRBLinExpr) rho, GRB_MAXIMIZE);
    GRB_instance->update();
}

int Model::index_from_str(string str) {
    auto ind = find(all_vertexes.begin(), all_vertexes.end(), str);
    if (ind != all_vertexes.end()) {
        auto test = all_vertexes.begin();
        return ind - all_vertexes.begin();
    }
    all_vertexes.push_back(str);
    return all_vertexes.size() - 1;
}

int **Model::build_adjacency_matrix() {
    int **Matrix = matrix_malloc(all_vertexes.size());
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
    P = Matrix;
    return Matrix;
}

void Model::make_suitable_paths() {
    for(auto start : starting_vertexes()) select_suitable_paths(start, M*J, J);
}

std::vector<int> Model::starting_vertexes() const {
    vector<int> vertexes;
    for (int j = 0; j < all_vertexes.size(); ++j) {
        int not_zero = 0;
        for (int i = 0; i < all_vertexes.size(); ++i) {
            if (P[i][j] == 1) {
                not_zero++;
                break;
            }
        }
        if (not_zero == 0) vertexes.push_back(j);
    }
    return vertexes;
}

void Model::select_suitable_paths(int start, int v, int W) {
    way.push_back(start);
    int adj_counter = 0;
    for (int i = 0; i < v; ++i)
        if (P[start][i]) {
            adj_counter++;
            select_suitable_paths(i, v, W);
        }
    if (!adj_counter && check_way()) {
        string suitable_way;
        for (int i: way) {
            suitable_way += all_vertexes[i] + " ";
            cout << all_vertexes[i] << ' ';
        }
        suitable_paths.push_back(suitable_way);
        cout << '\n';
    }
    way.pop_back();
}

bool Model::check_way() {
    string path;
    for (int i = 0; i < way.size() - 1; ++i)
        path += all_vertexes[way[i]] + " ";
    path += all_vertexes[way[way.size() - 1]];

    vector<double> path_coeffs;
    vector<string> path_vars_names;
    prepare_coeffs_and_var_names(path, path_coeffs, path_vars_names);

    for (auto &bound: bounds) {
        if (path.length() > bound.length()) continue;
        bool need_to_stop = true;
        vector<double> bound_coeffs;
        vector<string> bound_vars_names;
        prepare_coeffs_and_var_names(bound, bound_coeffs, bound_vars_names);
        for (int bound_var_ind = 0; bound_var_ind < bound_vars_names.size(); bound_var_ind++) {
            int var_ind;
            for (var_ind = 0; var_ind < path_vars_names.size(); var_ind++)
                if (path_vars_names[var_ind] == bound_vars_names[bound_var_ind]) break;

            if ((var_ind == path_vars_names.size() && bound_coeffs[bound_var_ind] < 0)  ||
            path_coeffs[var_ind] > bound_coeffs[bound_var_ind]){
                need_to_stop = false;
                break;
            }
        }
        for (int path_var_ind = 0; path_var_ind < bound_vars_names.size(); path_var_ind++) {
            int var_ind;
            for (var_ind = 0; var_ind < bound_vars_names.size(); var_ind++)
                if (bound_vars_names[var_ind] == path_vars_names[path_var_ind]) break;

            if ((var_ind == bound_vars_names.size() && path_coeffs[path_var_ind] > 0)  ||
            bound_coeffs[var_ind] > path_coeffs[path_var_ind]){
                need_to_stop = false;
                break;
            }
        }
        if (need_to_stop) return false;
    }
    return true;
}

void Model::build_all_vertexes_from_missed_vertexes(const string& missed_vertexes) {
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < J; ++j) {
            all_vertexes.push_back(indexes_pair_to_string({i, j}));
        }
    }
    for(const auto& vertex : split(missed_vertexes, ' ')){
        auto ind = std::find(all_vertexes.begin(), all_vertexes.end(), vertex);
        all_vertexes.erase(ind);
    }
}

bool Model::is_sign(char c) {
    if(c == '+' || c == '-' || c == '*')
        return true;
    return false;
}

bool Model::is_preset(string str) {
    if (presets.find(str) != presets.end()) return true;
    if (str[0] == 'L' || str[0] == 'D') return true;
    return false;
}

void Model::process_preset(string str, vector<double> &coeffs, vector<string> &vars, double current_coeff) {
    if (str[0] == 'L') {
        for (auto v : all_vertexes) {
            if (48 + str[1] == v[0]) {
                coeffs.push_back(current_coeff);
                vars.push_back(v);
            }
        }
        return;
    }
    if (str == "Delta") {
        for (auto v : all_vertexes) {
            coeffs.push_back(current_coeff);
            vars.push_back(v);
        }
    }
    if (str[0] == 'D') {
        for (auto v : all_vertexes) {
            if (str[1] == v[1]) {
                coeffs.push_back(current_coeff);
                vars.push_back(v);
            }
        }
    }
}

void Model::prepare_coeffs_and_var_names(const string& str, vector<double> &coeffs, vector<string> &vars) {
    vector<string> divided_expression = split(str, ' ');
    int sign = 1;
    double current_coeff = 1;
    for (auto part : divided_expression) {
        if (isalpha(part.front())) {
            if (is_preset(part)) {
                process_preset(part, coeffs, vars, sign * current_coeff);
                continue;
            }
            coeffs.push_back(sign);
            vars.push_back(part);
            continue;
        }
        if (part.length() == 1) {
            sign = part == "+" ? 1 : -1;
            continue;
        }
        int i = 0;
        do {
            i++;
        } while(!is_sign(part[i]));

        string coeff = part.substr(0, i);
        auto slash_position = coeff.find('/');
        if (slash_position != -1) {
            double num = stod(coeff.substr(0, slash_position));
            double denum = stod(coeff.substr(slash_position + 1, i));
            current_coeff = num / denum;
            coeffs.push_back(sign * current_coeff);
        } else {
            current_coeff = stod(part.substr(0, i));
            coeffs.push_back(sign * current_coeff);
        }
        string operation = part.substr(i + 1, part.length());
        vars.push_back(operation);
    }
}

GRBVar* Model::prepare_operations(const vector<string>& names) {
    GRBVar * operations = (GRBVar *) malloc(sizeof(GRBVar) * names.size());
    if (!operations) exit(1);
    for (int i = 0; i < names.size(); ++i)
        operations[i] = GRB_instance->getVarByName(names[i]);
    return operations;
}

GRBLinExpr Model::make_constraint(const string& expr) {
    vector<double> coeffs;
    vector<string> vars_names;
    prepare_coeffs_and_var_names(expr, coeffs, vars_names);
    GRBVar* operations = prepare_operations(vars_names);
    GRBLinExpr result;
    result.addTerms(&coeffs[0], operations, coeffs.size());
    return result;
}

void Model::process_complex_expressions(vector<string> expr) {
    if (expr.front() == "Lmax") {
        for (int i = 1; i <= M; ++i) {
            GRBLinExpr lhs_expr = make_constraint("L" + to_string(i));
            GRB_instance->addConstr(lhs_expr, expr[1].front(), stod(expr[2]), "L" + to_string(i));
        }
    }
    if (expr.front() == "Dmax") {
        for (int i = 1; i <= J; ++i) {
            GRBLinExpr lhs_expr = make_constraint("D" + to_string(i));
            GRB_instance->addConstr(lhs_expr, expr[1].front(), stod(expr[2]), "D" + to_string(i));
        }
    }
    if (expr.front() == "Pmax") {
        for (const auto& vertex : all_vertexes) {
            GRBLinExpr lhs_expr = make_constraint(vertex);
            GRB_instance->addConstr(lhs_expr, expr[1].front(), stod(expr[2]), vertex);
        }
    }
}

void Model::process_expressions() {
    auto expressions = model.at("expressions").get<vector<vector<string>>>();
    for (auto expr : expressions) {
        if (expr.front() == "Lmax" || expr.front() == "Dmax" || expr.front() == "Pmax") {
            process_complex_expressions(expr);
            continue;
        }
        GRBLinExpr lhs_expr = make_constraint(expr.front());
        GRB_instance->addConstr(lhs_expr, expr[1].front(), stod(expr[2]), expr.back());
    }

    // addConstr(rho <= p[1][1] + p[0][1] + p[0][2], "P_1");
    GRBLinExpr lhs_expr = make_constraint(suitable_paths.front() + "- rho");
    GRB_instance->addConstr(lhs_expr, '>', 0, "P_1");


    GRB_instance->update();
    GRB_instance->optimize();
    GRB_instance->addConstr(GRB_instance->getVarByName("rho") <= GRB_instance->getVarByName("a2")
        + GRB_instance->getVarByName("a3") + GRB_instance->getVarByName("b3"), "P_12");

    GRB_instance->update();
    GRB_instance->optimize();
}