//
// Created by chechushkov on 14.03.2022.
//

#ifndef UNION_HELPERS_H
#define UNION_HELPERS_H

#include <iostream>
#include <vector>
#include "gurobi_c++.h"

std::vector<std::string> &split(const std::string &s, char divider, std::vector<std::string> &elems);

std::vector<std::string> split(const std::string &s, char divider);

int **matrix_malloc(int N);

void matrix_output(int **Matrix, int N);

std::string indexes_pair_to_string(std::pair<int, int> indexes);

GRBVar **GRBVars_matrix_malloc(int M, int J);

GRBVar* prepare_operations(const std::vector<std::string>& names, GRBModel &m);

void prepare_coeffs_and_var_names(const std::string& str, std::vector<double> &coeffs, std::vector<std::string> &vars);

GRBLinExpr make_constraint(const std::string &expr, GRBModel *m);

#endif //UNION_HELPERS_H
