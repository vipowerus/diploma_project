//
// Created by chechushkov on 23.02.2022.
//

#ifndef TEMPLATES_MODEL_H
#define TEMPLATES_MODEL_H

#include "lib/json.hpp"
#include <deque>
#include <map>
#include "gurobi_c++.h"


class Model {
public:
    int M, J;
    int **P;
    GRBModel *GRB_instance;
    nlohmann::json model, templates;
    std::vector<std::string> base_path, bounds, all_vertexes, suitable_paths;
    std::deque<int> way;
    std::map<std::string, std::string> presets = {{"Lmax", "a1"}, {"Dmax", "a2"}, {"Delta", "a3"}, {"Pmax", "asd"}};

    Model();

    int **build_adjacency_matrix();

    void make_suitable_paths();

    void process_expressions();

private:
    int index_from_str(std::string str);

    void build_all_vertexes_from_missed_vertexes(const std::string&);

    void create_GRBModel();

    std::vector<int> starting_vertexes() const;

    void select_suitable_paths(int start, int v, int W);

    bool check_way();

    bool is_sign(char c);

    bool is_preset(std::string str);

    void process_preset(std::string str, std::vector<double> &coeffs, std::vector<std::string> &vars, double current_coeff);

    void process_complex_expressions(std::vector<std::string> expr);

    void prepare_coeffs_and_var_names(const std::string& str, std::vector<double> &coeffs, std::vector<std::string> &vars);

    GRBVar* prepare_operations(const std::vector<std::string>& names);

    GRBLinExpr make_constraint(const std::string& expr);
};


#endif //TEMPLATES_MODEL_H
