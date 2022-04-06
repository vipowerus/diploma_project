//
// Created by chechushkov on 23.02.2022.
//
#include <iostream>
#include <vector>
#include <sstream>
#include "gurobi_c++.h"
#include <algorithm>

using namespace std;

extern vector<string> all_vertexes;

int **matrix_malloc(int N) {
    int **Matrix = (int **) malloc(sizeof(int *) * N);
    if (!Matrix) exit(1);

    for (int i = 0; i < N; ++i) {
        Matrix[i] = (int *) malloc(sizeof(int) * N);
        if (!Matrix[i]) exit(1);
    }
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            Matrix[i][j] = 0;
    return Matrix;
}

GRBVar **GRBVars_matrix_malloc(int M, int J) {
    auto **Matrix = (GRBVar **) malloc(sizeof(GRBVar *) * M);
    if (!Matrix) exit(1);

    for (int i = 0; i < M; ++i) {
        Matrix[i] = (GRBVar *) malloc(sizeof(GRBVar) * J);
        if (!Matrix[i]) exit(1);
    }
    return Matrix;
}

void matrix_output(int **Matrix, int N) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            std::cout << Matrix[i][j] << ' ';
        }
        std::cout << '\n';
    }
}

std::vector<std::string> &split(const std::string &s, char divider, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, divider)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char divider) {
    std::vector<std::string> elems;
    split(s, divider, elems);
    return elems;
}

int index_from_str(string str) {
    auto ind = find(all_vertexes.begin(), all_vertexes.end(), str);
    if (ind != all_vertexes.end())
        return ind - all_vertexes.begin();
    all_vertexes.push_back(str);
    return all_vertexes.size() - 1;
}

pair<int, int> index_pair_from_str(string str) { // useless for now
    return {str[0] - 97, str[1] - 49};
}

string indexes_pair_to_string(pair<int, int> indexes){
    string a = {(char)('a' + indexes.first), (char)(indexes.second + 49)};
    return a;
}

bool is_sign(char c){
    if(c == '+' || c == '-' || c == '*')
        return true;
    return false;
}

void prepare_coeffs_and_var_names(const string& str, vector<double> &coeffs, vector<string> &vars){
    vector<string> divided_expression = split(str, ' ');
    int sign = 1;
    for (auto part : divided_expression){
        if(isalpha(part.front())){
            coeffs.push_back(sign);
            vars.push_back(part);
            continue;
        }
        if(part.length() == 1){
            sign = part == "+" ? 1 : -1;
            continue;
        }
        int i = 0;
        do{
            i++;
        }while(!is_sign(part[i]));
        double coeff = stod(part.substr(0, i));
        coeffs.push_back(coeff * sign);
        string operation = part.substr(i + 1, part.length());
        vars.push_back(operation);
    }
}

GRBVar* prepare_operations(const vector<string>& names, GRBModel &m){
    GRBVar * operations = (GRBVar *) malloc(sizeof(GRBVar) * names.size());
    if(!operations) exit(1);
    for (int i = 0; i < names.size(); ++i) {
        operations[i] = m.getVarByName(names[i]);
    }
    return operations;
}

GRBLinExpr make_constraint(const string& expr, GRBModel &m){
    vector<double> coeffs;
    vector<string> vars_names;
    prepare_coeffs_and_var_names(expr, coeffs, vars_names);
    GRBVar* operations = prepare_operations(vars_names, m);
    GRBLinExpr result;
    result.addTerms(&coeffs[0], operations, coeffs.size());
    return result;
}