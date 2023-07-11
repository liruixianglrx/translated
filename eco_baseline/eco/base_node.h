#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>

#include "node.h"
#include "min_term.h"
#include "gen_minterm.h"
#include "yosys_flow.h"

class BaseNode {
private:
    std::vector<Node> nodes;
    std::vector<std::vector<int>> on_datas;
    MinTerms minterms;

public:
    BaseNode(const std::vector<Node>& node_list, const std::tuple<int, int>& index_list, const std::vector<std::vector<int>>& on_datas)
        : nodes(get_nodes(node_list, index_list)), on_datas(get_unique_tuples(on_datas)), minterms(get_unique_tuples(on_datas)) {}

    void gen_patch() {
        std::vector<std::string> variable_list = get_nodes_name();

        // Only support 1 target node now.
        variable_list.push_back("t_0");

        std::vector<std::vector<int>> minterms = gen_minterm(variable_list, minterms.get_truth_table());
        yosys_flow(minterms, variable_list, true);
    }

    std::vector<Node> get_nodes(const std::vector<Node>& node_list, const std::tuple<int, int>& index_list) const {
        int start_index = std::get<0>(index_list);
        int end_index = std::get<1>(index_list);
        std::vector<Node> selected_nodes(node_list.begin() + start_index, node_list.begin() + end_index + 1);
        return selected_nodes;
    }

    std::vector<std::vector<int>> get_unique_tuples(const std::vector<std::vector<int>>& data) const {
        std::vector<std::vector<int>> unique_tuples;
        for (const std::vector<int>& tuple : data) {
            if (std::find(unique_tuples.begin(), unique_tuples.end(), tuple) == unique_tuples.end()) {
                unique_tuples.push_back(tuple);
            }
        }
        return unique_tuples;
    }

    std::vector<std::string> get_nodes_name() const {
        std::vector<std::string> names;
        for (const Node& node : nodes) {
            names.push_back(node.node_name);
        }
        return names;
    }

    int get_rough_size() const {
        return minterms.get_rough_size();
    }

    std::string get_simple_str() const {
        return minterms.to_string();
    }

    friend bool operator<(const BaseNode& node1, const BaseNode& node2) {
        return node1.get_rough_size() < node2.get_rough_size();
    }
};
