#include <iostream>
#include <vector>
#include <unordered_set>
#include <random>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>

#include "numpy/numpy.hpp"

double comb_num(int base, int sample)
{
    double numerator = 1.0;
    double denominator = 1.0;
    for (int i = base; i > base - sample; --i) {
        numerator *= i;
    }
    for (int i = 1; i <= sample; ++i) {
        denominator *= i;
    }
    return numerator / denominator;
}

std::vector<std::vector<int>> gen_test_vec(int node_num)
{
    std::vector<std::vector<int>> test_vec;
    for (int i = 0; i < pow(2, node_num); ++i) {
        std::vector<int> vec(node_num, 0);
        int value = i;
        int j = node_num - 1;
        while (value > 0) {
            vec[j] = value % 2;
            value /= 2;
            --j;
        }
        test_vec.push_back(vec);
    }
    return test_vec;
}

std::vector<int> rm_duplicates(const std::vector<int>& l)
{
    std::unordered_set<int> unique_set(l.begin(), l.end());
    std::vector<int> unique_list(unique_set.begin(), unique_set.end());
    return unique_list;
}

std::string random_id(int num)
{
    const std::string number = "0123456789";
    const std::string alpha = "abcdefghijklmnopqrstuvwxyz";

    std::string id;
    for (int i = 0; i < num; ++i) {
        char ch;
        if (rand() % 2 == 0) {
            ch = number[rand() % number.size()];
        } else {
            ch = alpha[rand() % alpha.size()];
        }
        id += ch;
    }
    return id;
}

std::tuple<std::vector<std::vector<int>>, bool> merge_combination(
    const std::vector<std::vector<int>>& sample_list_1,
    int chosen_num_1,
    const std::vector<std::vector<int>>& sample_list_2,
    int chosen_num_2,
    bool prune = true,
    int pruned_max = 2000)
{
    double comb1_n = comb_num(sample_list_1.size(), chosen_num_1);
    double comb2_n = comb_num(sample_list_2.size(), chosen_num_2);

    double combs_cnt = comb1_n * comb2_n;
    if (prune && combs_cnt > pruned_max) {
        bool pruned = true;
        std::vector<std::vector<int>> combs;
        for (int i = 0; i < pruned_max; ++i) {
            std::vector<int> merge_comb;
            std::vector<int> indices1 = random_indices(sample_list_1.size(), chosen_num_1);
            std::vector<int> indices2 = random_indices(sample_list_2.size(), chosen_num_2);
            for (int idx1 : indices1) {
                merge_comb.insert(merge_comb.end(), sample_list_1[idx1].begin(), sample_list_1[idx1].end());
            }
            for (int idx2 : indices2) {
                merge_comb.insert(merge_comb.end(), sample_list_2[idx2].begin(), sample_list_2[idx2].end());
            }
            combs.push_back(merge_comb);
        }
        return std::make_tuple(combs, pruned);
    } else {
        bool pruned = false;
        std::vector<std::vector<int>> combs;
        for (const auto& comb1 : sample_list_1) {
            for (const auto& comb2 : sample_list_2) {
                std::vector<int> merge_comb = comb1;
                merge_comb.insert(merge_comb.end(), comb2.begin(), comb2.end());
                combs.push_back(merge_comb);
            }
        }
        return std::make_tuple(combs, pruned);
    }
}

std::tuple<std::vector<std::vector<int>>, bool> pruned_combs(const std::vector<std::vector<int>>& sample_list, int chosen_num, int max = 2000)
{
    double comb_n = comb_num(sample_list.size(), chosen_num);
    if (comb_n > max) {
        bool pruned = true;
        std::vector<std::vector<int>> combs;
        for (int i = 0; i < max; ++i) {
            std::vector<int> indices = random_indices(sample_list.size(), chosen_num);
            std::vector<int> comb;
            for (int idx : indices) {
                comb.insert(comb.end(), sample_list[idx].begin(), sample_list[idx].end());
            }
            combs.push_back(comb);
        }
        return std::make_tuple(combs, pruned);
    } else {
        bool pruned = false;
        std::vector<std::vector<int>> combs;
        for (const auto& comb : sample_list) {
            combs.push_back(comb);
        }
        return std::make_tuple(combs, pruned);
    }
}

numpy::ndarray random_input_vec(int input_num, int n)
{
    std::vector<int> shape = {n, input_num};
    numpy::ndarray arr(shape, numpy::dtype::get_builtin<int8_t>());
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 1);
    for (auto it = arr.begin<int8_t>(); it != arr.end<int8_t>(); ++it) {
        *it = dist(rng);
    }
    return arr;
}

template <typename T>
T read_pickle(const std::string& pickle_path)
{
    std::ifstream input_file(pickle_path, std::ios::binary);
    if (input_file) {
        std::stringstream buffer;
        buffer << input_file.rdbuf();
        return pickle::loads(buffer.str());
    }
    return T();
}

double relative_error(const numpy::ndarray& y_true, const numpy::ndarray& y_pred)
{
    numpy::ndarray abs_diff = numpy::abs(y_true - y_pred);
    return numpy::sum(abs_diff) / numpy::sum(y_true);
}
