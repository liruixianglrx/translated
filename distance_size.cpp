#include <iostream>
#include <vector>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cmath>
#include <tuple>
#include <chrono>
#include <ctime>

#include "utils/min_term.hpp"
#include "utils/utils.hpp"

void get_distance_size(int node_num, int minterm_num, std::unordered_map<int, std::vector<int>>& result_dict)
{
    std::cout << "Running: node_num: " << node_num << ", minterm_num: " << minterm_num << std::endl;

    std::vector<std::vector<int>> test_vec = gen_test_vec(node_num);

    std::vector<std::vector<int>> combs;
    for (int i = 0; i < 1000; ++i) {
        combs.push_back(sample(test_vec, minterm_num));
    }

    for (const auto& comb : combs) {
        MinTerms minterm(comb);
        int distance_sum = sum(minterm.get_distance_tuple());
        int size = minterm.get_gate_num();

        if (result_dict.find(distance_sum) != result_dict.end()) {
            result_dict[distance_sum].push_back(size);
        }
        else {
            result_dict[distance_sum] = std::vector<int>{size};
        }
    }
}

void comb_distance_and_size(const std::vector<std::vector<int>>& comb, std::unordered_map<std::vector<int>, std::vector<int>, VectorHash>& result_proxy, const std::string& tmp_path = "tmp")
{
    MinTerms minterm(comb);
    std::vector<int> distance_sum_tuple = minterm.get_distance_tuple();
    int size = minterm.get_gate_num(tmp_path);

    if (result_proxy.find(distance_sum_tuple) != result_proxy.end()) {
        result_proxy[distance_sum_tuple].push_back(size);
    }
    else {
        result_proxy[distance_sum_tuple] = std::vector<int>{size};
    }
}

void get_distance_size_list(int node_num, int minterm_num, std::unordered_map<std::pair<int, int>, std::unordered_map<std::vector<int>, std::vector<int>, VectorHash>, PairHash>& result_dict)
{
    if (minterm_num >= pow(2, node_num)) {
        return;
    }

    std::unordered_map<std::vector<int>, std::vector<int>, VectorHash> result_proxy;

    std::vector<std::vector<int>> test_vec = gen_test_vec(node_num);

    std::vector<std::vector<int>> combs;
    for (int i = 0; i < 200; ++i) {
        combs.push_back(sample(test_vec, minterm_num));
    }

    #pragma omp parallel for num_threads(10)
    for (int i = 0; i < combs.size(); ++i) {
        comb_distance_and_size(combs[i], result_proxy, "tmp_" + std::to_string(i));
    }

    result_dict[std::make_pair(node_num, minterm_num)] = result_proxy;

    std::system("rm -rf tmp_*");
}

int main()
{
    std::vector<int> node_nums = {2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15};
    std::vector<int> minterm_nums = {2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 20, 30, 40, 50};

    std::unordered_map<std::pair<int, int>, std::unordered_map<std::vector<int>, std::vector<int>, VectorHash>, PairHash> result_dict;

    for (const auto& node_num : node_nums) {
        for (const auto& minterm_num : minterm_nums) {
            get_distance_size_list(node_num, minterm_num, result_dict);
        }
    }

    std::ofstream output_file("./size_predictor/distance_size_full.pkl", std::ios::binary);
    if (output_file) {
        pickle.dump(result_dict, output_file);
    }

    return 0;
}
