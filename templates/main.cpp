#include <iostream>
#include <string>
#include <stack>
#include <vector>
#include <deque>
#include <set>
#include <fstream>
#include <sstream>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

ifstream file("input.json");
stack<int, vector<int>> ways;
deque<int> abc;

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

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

void matrix_output(int N, int **Matrix) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            cout << Matrix[i][j] << ' ';
        }
        cout << '\n';
    }
}

int index_from_str(string str, int W) {
    return (str[0] - 97) * W + str[1] - 49;
}

string str_from_index(int ind, int W) {
    string str(to_string(ind));
    string result;
    for (char c : str){
        char first = (c - 48) / W + 97;
        char second = (c - 48) % W + 49;
        result.push_back(first);
        result.push_back(second);
    }
    return result;
}

int **build_adjacency_matrix(vector<string> trivials, int N, int M, int W) {
    int **Matrix = matrix_malloc(N);
    for (auto & trivial : trivials) {
        std::vector<std::string> st = split(trivial, ' ');
        int prev = -1;
        for (const string& str : st) { // !!!
            if (prev == -1) {
                prev = index_from_str(str, W);
                continue;
            }
            auto ind = index_from_str(str, W);
            Matrix[prev][ind] = 1;
            prev = ind;
        }
    }
    matrix_output(N, Matrix);
    return Matrix;
}

void print_ways(int **P, int start, int v, int W) {
    abc.push_back(start);
    int adj_counter = 0;
    for (int i = 0; i < v; ++i)
        if (P[start][i]) {
            adj_counter++;
            print_ways(P, i, v, W);
        }
    if (!adj_counter && true) {
        for (int i: abc)
            cout << str_from_index(i, W) << ' ';
        cout << '\n';
    }
    abc.pop_back();
}

vector<int> starts(int **P, int M, int W) {
    vector<int> vertexes;
    for (int j = 0; j < W * W; ++j) {
        int not_zero = 0;
        for (int i = 0; i < M * M; ++i) {
            if (P[i][j] == 1) {
                not_zero++;
                break;
            }
        }
        if(not_zero == 0) vertexes.push_back(j);
    }
    return vertexes;
}

int main() {
    json j;
    file >> j;

    auto trivials = j.at("base").get<vector<string>>();
    int M = j.at("M").get<int>(), W = j.at("W").get<int>();
    int m = max(M, W);

    auto P = build_adjacency_matrix(trivials, m * m, M, W);
    vector<int> vertexes = starts(P, M, W * M);
    for (int i : vertexes) print_ways(P, i, m * m, W);
    matrix_output(m * m, P);
    return 0;
}

/*
 "trivials" : [
    "a1 a2 a3 a4", "b1 b2 b3 b4", "c1 c4 c2 c3", "d1 d4 d2 d3",
    "a1 b1 c1 d1", "a2 b2 c2 d2", "a3 b3 c3 d3", "a4 b4 c4 d4"
  ]
 */