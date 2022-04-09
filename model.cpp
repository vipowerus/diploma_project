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
    this->bounds = templates.at("base").get<std::vector<std::string>>();
    this->P = nullptr;

    if (model.contains("work_vertexes")) all_vertexes = split(model.at("work_vertexes").get<string>(), ' ');
    else if (model.contains("missed_vertexes")) build_all_vertexes_from_missed_vertexes(model.at("missed_vertexes").get<string>());
    else {
        cout << "Input vertexes are wrong. Exit...";
        exit(1);
    }
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
