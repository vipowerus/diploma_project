//
// Created by chechushkov on 23.02.2022.
//

#ifndef TEMPLATES_MODEL_H
#define TEMPLATES_MODEL_H

#include "lib/json.hpp"
#include <deque>


class Model {
public:
    int M, J;
    int **P;
    nlohmann::json model, templates;
    std::vector<std::string> base_path, bounds, all_vertexes = {"a1", "a2", "a3", "b1", "b2", "b3", "c1", "c2", "c3"}, suitable_paths;
    std::deque<int> way;

    Model();

    int **build_adjacency_matrix();

    void make_suitable_paths();

private:
    std::vector<int> starting_vertexes() const;

    void select_suitable_paths(int start, int v, int W);

    bool check_way();
};


#endif //TEMPLATES_MODEL_H
