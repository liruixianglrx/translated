#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "numpy.hpp"
#include "pandas.hpp"

#include "tb_to_gate.hpp"
#include "yosys_flow.hpp"
#include "verilog_parser.hpp"

class MinTerms {
private:
    std::vector<std::vector<int>> value;
    int node_num;
    int min_term_num;
    std::vector<std::vector<int>> truth_table;

public:
    MinTerms(std::vector<std::vector<int>>& value) : value(value) {
        node_num = value[0].size();
        min_term_num = value.size();
        for (auto term : value) {
            term.push_back(1);
            truth_table.push_back(term);
        }
    }

    int get_gate_num(std::string tmp_path = "tmp") {
        _synthesis(tmp_path);

        VerilogParser vp;
        auto circuit_info = vp.read_file("./" + tmp_path + "/synth.v");
        int gate_num = 0;
        for (auto& [gate, inst] : circuit_info["inst"]) {
            gate_num += inst.size();
        }

        return gate_num;
    }

    std::tuple<int> get_distance_tuple() {
        if (min_term_num == 1) {
            return std::make_tuple(0);
        }

        std::vector<std::vector<int>> distance_matrix(min_term_num, std::vector<int>(min_term_num, node_num));

        std::vector<int> distance_sum;

        for (int index_1 = 0; index_1 < min_term_num; index_1++) {
            for (int index_2 = index_1 + 1; index_2 < min_term_num; index_2++) {
                int dist = _distance(index_1, index_2);
                distance_matrix[index_1][index_2] = dist;
                distance_matrix[index_2][index_1] = dist;
            }
            distance_sum.push_back(*std::min_element(distance_matrix[index_1].begin(), distance_matrix[index_1].end()));
        }

        return std::make_tuple(distance_sum);
    }

private:
    int _distance(int index_1, int index_2) {
        assert(index_1 < min_term_num);
        assert(index_2 < min_term_num);

        int d = 0;
        for (int i = 0; i < node_num; i++) {
            d += std::abs(value[index_1][i] - value[index_2][i]);
        }
        return d;
    }

    void _synthesis(std::string tmp_path) {
        std::vector<std::string> fake_variable_list;
        for (int i = 0; i < node_num; i++) {
            fake_variable_list.push_back("g" + std::to_string(i));
        }
        fake_variable_list.push_back("t");

        yosys_flow(gen_minterm(fake_variable_list, truth_table), fake_variable_list, tmp_path, true);
    }
};

float model_predict_size(Estimator& estimator, int node_num, int min_term_num, int distance) {
    std::vector<std::string> cols = {"node_num", "minterm_num", "distance"};
    if (min_term_num == 1 || min_term_num == std::pow(2, node_num) - 1) {
        return node_num - 1;
    }

    pandas::DataFrame data({{node_num, min_term_num, distance}}, cols);
    float predict_size = estimator.predict(data).item();
    return predict_size;
}

int rough_size(int node_num, int min_term_num) {
    return node_num * min_term_num;
}

int main() {
    // Example usage
    std::vector<std::vector<int>> minterms = {{0, 0, 1}, {1, 0, 1}, {0, 1, 0}};
    MinTerms mt(minterms);
    int gate_num = mt.get_gate_num();
    std::tuple<int> distance_tuple = mt.get_distance_tuple();
    float predict_size = model_predict_size(estimator, mt.node_num, mt.min_term_num, std::get<0>(distance_tuple));
    int size = rough_size(mt.node_num, mt.min_term_num);

    std::cout << "Gate Number: " << gate_num << std::endl;
    std::cout << "Predicted Size: " << predict_size << std::endl;
    std::cout << "Rough Size: " << size << std::endl;

    return 0;
}
