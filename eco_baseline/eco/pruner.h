#include <iostream>
#include <vector>
#include <algorithm>

struct Node {
    int driver_index;
    std::vector<int> load_index;
};

struct Gate {
    std::vector<int> input_index;
    int output_index;
};

class LogicTree {
public:
    int node_num;
    int gate_num;
    std::vector<Node> node_list;
    std::vector<Gate> gate_list;
    std::vector<Node> input_nodes;
    std::vector<Node> output_nodes;
    std::vector<Node> target_nodes;
};

std::vector<int> rm_duplicates(const std::vector<int>& vec) {
    std::vector<int> result;
    std::set<int> seen;
    for (int num : vec) {
        if (seen.find(num) == seen.end()) {
            result.push_back(num);
            seen.insert(num);
        }
    }
    return result;
}

class Pruner {
private:
    LogicTree origin;
    LogicTree golden;
    bool do_pruning;
    std::vector<Node> fanout_POs_og;
    std::vector<Node> fanin_PIs_gd;
    std::vector<Node> fanout_POs_gd;
    std::vector<Node> fanin_PIs_og;
    std::vector<int> fanout_nodes_og_i;
    std::vector<int> fanout_gates_og_i;
    std::vector<int> fanout_nodes_gd_i;
    std::vector<int> fanout_gates_gd_i;

public:
    Pruner(const LogicTree& origin_logic_tree, const LogicTree& golden_logic_tree) {
        origin = origin_logic_tree;
        golden = golden_logic_tree;
    }

    void run_pruning(bool do_pruning=true) {
        this->do_pruning = do_pruning;
        if (do_pruning) {
            pruning();
            mask_logic_tree();
            std::cout << "Finish pruning." << std::endl;
        } else {
            receive_mask(origin, std::vector<int>(origin.node_num, 1), std::vector<int>(origin.gate_num, 1));
            receive_mask(golden, std::vector<int>(golden.node_num, 1), std::vector<int>(golden.gate_num, 1));
        }
    }

    void pruning() {
        fanout_POs_og.clear();
        fanin_PIs_gd.clear();
        for (const Node& target_node : origin.target_nodes) {
            find_fanout_PO(target_node);
        }
        fanout_POs_og = rm_duplicates(fanout_POs_og);
        std::sort(fanout_POs_og.begin(), fanout_POs_og.end(), [](const Node& a, const Node& b) {
            return a.driver_index < b.driver_index;
        });

        for (const Node& po : fanout_POs_og) {
            Node fanout_po = golden.output_nodes[po.driver_index];
            fanout_POs_gd.push_back(fanout_po);
        }

        for (const Node& fanout_PO : fanout_POs_gd) {
            find_fanin_PI(fanout_PO);
        }
        fanin_PIs_gd = rm_duplicates(fanin_PIs_gd);
        std::sort(fanin_PIs_gd.begin(), fanin_PIs_gd.end(), [](const Node& a, const Node& b) {
            return a.driver_index < b.driver_index;
        });

        for (const Node& pi : fanin_PIs_gd) {
            Node fanin_pi = origin.input_nodes[pi.driver_index];
            fanin_PIs_og.push_back(fanin_pi);
        }

        find_all_fanout(origin, fanin_PIs_og, fanout_nodes_og_i, fanout_gates_og_i);
        find_all_fanout(golden, fanin_PIs_gd, fanout_nodes_gd_i, fanout_gates_gd_i);
    }

    void find_fanout_PO(const Node& node) {
        if (node.load_index.empty()) {
            fanout_POs_og.push_back(node);
        } else {
            for (int load_gate_index : node.load_index) {
                int fanout_node_index = origin.gate_list[load_gate_index].output_index;
                Node fanout_node = origin.node_list[fanout_node_index];
                find_fanout_PO(fanout_node);
            }
        }
    }

    void find_fanin_PI(const Node& node) {
        if (std::find(golden.input_nodes.begin(), golden.input_nodes.end(), node) != golden.input_nodes.end()) {
            fanin_PIs_gd.push_back(node);
        } else if (node.driver_index == -1) {
            return;
        } else {
            Gate driver_gate = golden.gate_list[node.driver_index];
            for (int input_i : driver_gate.input_index) {
                find_fanin_PI(golden.node_list[input_i]);
            }
        }
    }

    void find_all_fanout(const LogicTree& logic_tree, const std::vector<Node>& fanin_PIs, std::vector<int>& fanout_nodes_i, std::vector<int>& fanout_gates_i) {
        for (const Node& pi : fanin_PIs) {
            fanout_nodes_i.push_back(std::distance(logic_tree.node_list.begin(), std::find(logic_tree.node_list.begin(), logic_tree.node_list.end(), pi)));
        }
        for (const Node& target_node : logic_tree.target_nodes) {
            int target_node_i = std::distance(logic_tree.node_list.begin(), std::find(logic_tree.node_list.begin(), logic_tree.node_list.end(), target_node));
            fanout_nodes_i.push_back(target_node_i);
        }
        fanout_nodes_i.push_back(logic_tree.node_num - 2);
        fanout_nodes_i.push_back(logic_tree.node_num - 1);

        std::vector<Node> node_list_copy = logic_tree.node_list;
        std::vector<Gate> gate_list_copy = logic_tree.gate_list;

        for (const Node& pi : fanin_PIs) {
            sub_propagate(pi, node_list_copy, gate_list_copy, fanout_nodes_i, fanout_gates_i);
        }

        fanout_nodes_i = rm_duplicates(fanout_nodes_i);
        fanout_gates_i = rm_duplicates(fanout_gates_i);
        std::sort(fanout_nodes_i.begin(), fanout_nodes_i.end());
        std::sort(fanout_gates_i.begin(), fanout_gates_i.end());
    }

    void sub_propagate(const Node& node, std::vector<Node>& node_list, std::vector<Gate>& gate_list, std::vector<int>& fanout_nodes_i, std::vector<int>& fanout_gates_i) {
        int node_i = std::distance(node_list.begin(), std::find(node_list.begin(), node_list.end(), node));

        if (node.load_index.empty()) {
            fanout_nodes_i.push_back(node_i);
        }

        for (int load_gate_i : node.load_index) {
            Gate& load_gate = gate_list[load_gate_i];

            auto it = std::find(load_gate.input_index.begin(), load_gate.input_index.end(), node_i);
            if (it != load_gate.input_index.end()) {
                load_gate.input_index.erase(it);
            }

            if (load_gate.input_index.empty()) {
                int fanout_node_i = load_gate.output_index;
                Node& fanout_node = node_list[fanout_node_i];
                fanout_nodes_i.push_back(fanout_node_i);
                fanout_gates_i.push_back(load_gate_i);
                sub_propagate(fanout_node, node_list, gate_list, fanout_nodes_i, fanout_gates_i);
            }
        }
    }

    void mask_logic_tree() {
        auto index_2_mask = [](const std::vector<int>& indexes, int node_num) {
            std::vector<int> mask(node_num, 0);
            for (int i : indexes) {
                mask[i] = 1;
            }
            return mask;
        };

        origin.receive_mask(
            index_2_mask(fanout_nodes_og_i, origin.node_num),
            index_2_mask(fanout_gates_og_i, origin.gate_num)
        );

        golden.receive_mask(
            index_2_mask(fanout_nodes_gd_i, golden.node_num),
            index_2_mask(fanout_gates_gd_i, golden.gate_num)
        );
    }

    void print_info() {
        if (do_pruning) {
            std::cout << "Original node num: " << origin.node_num << std::endl;
            std::cout << "After Pruning: " << fanout_nodes_og_i.size() << std::endl;
            std::cout << "Pruning rate: " << (1 - (double)fanout_nodes_og_i.size() / origin.node_num) << std::endl;
        }
    }

    std::string repr() {
        std::string repr = "Fanout POs: ";
        for (const Node& node : fanout_POs_og) {
            repr += std::to_string(node.index) + " ";
        }
        repr += "\nFanin PIs: ";
        for (const Node& node : fanin_PIs_og) {
            repr += std::to_string(node.index) + " ";
        }
        repr += "\nAll fanouts: ";
        for (int i : fanout_nodes_og_i) {
            repr += origin.node_list[i].to_string() + " ";
        }
        return repr;
    }
};

int main() {
    LogicTree origin;
    LogicTree golden;
    Pruner pruner(origin, golden);
    pruner.run_pruning();
    std::cout << pruner.repr() << std::endl;
    pruner.print_info();
    return 0;
}
