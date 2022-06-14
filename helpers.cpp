//
// Created by chechushkov on 23.02.2022.
//
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include "gurobi_c++.h"
#include <algorithm>

using namespace std;

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

