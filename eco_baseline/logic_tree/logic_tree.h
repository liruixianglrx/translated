#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>

class Node {
private:
    int level;
    int value;
    std::vector<int> load_index;
    int driver_index;
public:
    Node(std::string name) : node_name(name), level(-1), value(-1), driver_index(-1) {}
    void set_level(int lvl) { level = lvl; }
    void set_value(int val) { value = val; }
    void add_load(int load) { load_index.push_back(load); }
    void set_driver(int driver) { driver_index = driver; }
    void get_level(const std::vector<Node>& node_list, const std::vector<Gate>& gate_list);
    std::string node_name;
};

class Gate {
private:
    std::vector<int> input_index;
    int output_index;
    std::vector<Node>& node_list;
public:
    Gate(const std::vector<int>& input, int output, std::vector<Node>& nodes) : input_index(input), output_index(output), node_list(nodes) {}
    void get_output_value();
};

void Node::get_level(const std::vector<Node>& node_list, const std::vector<Gate>& gate_list) {
    if (level != -1) {
        return;
    }
    
    level = 0;
    for (int load : load_index) {
        int load_node_index = gate_list[load].get_output_index();
        node_list[load_node_index].get_level(node_list, gate_list);
        int load_node_level = node_list[load_node_index].get_level();
        level = std::max(level, load_node_level + 1);
    }
}

void Gate::get_output_value() {
    std::vector<int> input_values;
    for (int input : input_index) {
        input_values.push_back(node_list[input].get_value());
    }
    
    int output_value = compute_gate_output(input_values);
    node_list[output_index].set_value(output_value);
}

class LogicTree {
private:
    std::unordered_map<std::string, std::vector<std::string>> circuit_info;
    std::vector<Node> node_list;
    std::vector<Gate> gate_list;
    std::vector<Node> input_nodes;
    std::vector<Node> output_nodes,target_nodes;
    std::vector<int> target_nodes_i;
    int input_num,output_num,target_num;
    // Add additional member variables if needed
    
public:
    LogicTree(std::unordered_map<std::string, int> circuit_info, bool verbose = false) : circuit_info(circuit_info) {
        _read_circuit();
        
        if (verbose) {
            std::cout << "Nodes: ";
            for (const auto& node : node_list) {
                std::cout << node.get_name() << " ";
            }
            std::cout << std::endl;
            
            std::cout << "Gates: ";
            for (const auto& gate : gate_list) {
                std::cout << gate.get_name() << " ";
            }
            std::cout << std::endl;
        }
    }
    
    void _read_circuit() {
        std::vector<std::string> port_list;
        port_list.reserve(circuit_info["input"].size() + circuit_info["output"].size() + circuit_info["wire"].size());
        port_list.insert(port_list.end(), circuit_info["input"].begin(), circuit_info["input"].end());
        port_list.insert(port_list.end(), circuit_info["output"].begin(), circuit_info["output"].end());
        port_list.insert(port_list.end(), circuit_info["wire"].begin(), circuit_info["wire"].end());

        input_num = circuit_info["input"].size();
        output_num = circuit_info["output"].size();

        // Change the port name to Node
        for (const std::string& port : port_list) {
            node_list.push_back(Node(port));
        }

        // Add constant nodes
        Node constant_0("0");
        Node constant_1("1");
        constant_0.set_level(0);
        constant_1.set_level(0);
        constant_0.set_value(0);
        constant_1.set_value(1);

        node_list.push_back(constant_0);
        node_list.push_back(constant_1);

        input_nodes = std::vector<Node>(node_list.begin(), node_list.begin() + input_num);
        output_nodes = std::vector<Node>(node_list.begin() + input_num, node_list.begin() + input_num + output_num);

        // This only works for ICCAD2017
        // Set the target node's level to 0
        target_num = 0;
        for (int i = 0; i < node_list.size(); ++i) {
            Node& node = node_list[i];
            // if (node.node_name.find("t_") != std::string::npos) {
            //     node.set_level(0);
            //     target_nodes.push_back(&node);
            //     target_nodes_i.push_back(i);
            //     target_num++;
            // }
            if (node.node_name.find("t_") != -1) {
                node.set_level(0);
                target_nodes.push_back(node);
                target_nodes_i.push_back(i);
                target_num++;
            }
        }

        // Instantiate all gates & update relations between Node and Gate.
        std::vector<std::vector<int>> inst_list;
        int gate_index_count = 0;  // help to locate

        // E.g. 'inst': {'AND': [['g1', 'a', 'b'], ['y1', 'g1', 'g2']]}
        std::vector<std::string> inst_dict = circuit_info["inst"];
        //question circuit_info's structure ?
        // std::unordered_map<string,std::pair<key,val>> ?
        for (const auto& kv : inst_dict) {
            std::string key = kv.first;
            std::vector<std::vector<std::string>> value = kv.second;
            Gate* gate_inst;

            if (key == "AND") {
                std::vector<int> output_index = eco_utils.port_name_2_index(
                    std::vector<std::string>(value[0].begin(), value[0].begin() + 1),
                    node_list
                );
                int input_index = eco_utils.port_name_2_index(
                    std::vector<std::string>(value[0].begin() + 1, value[0].end()),
                    node_list
                );
                gate_inst = new Gate(input_index, output_index[0], node_list);
            }
            // Add more gate types as needed

            gate_list.push_back(gate_inst);

            // Update node's load
            for (const int& input_i : input_index) {
                Node& node = node_list[input_i];
                node.add_load(gate_index_count);
            }

            // Update node's driver
            Node& node = node_list[output_index[0]];
            node.set_driver(gate_index_count);

            // gate index++
            gate_index_count++;
        }
    }

    void receive_mask(std::vector<int> node_mask, std::vector<int> gate_mask) {
        // Implement the receive_mask logic
    }
    
    void _node_backward() {
        // Implement the _node_backward logic
    }
    
    void forward(const std::vector<int>& input_data, const std::vector<int>& target_data) {
        assert(input_data.size() == masked_input_num && "Wrong input size");
        assert(target_data.size() == target_num && "Wrong input size");

        for (int i = 0; i < masked_input_num; i++) {
            masked_input_nodes[i] = input_data[i];
        }

        for (int i = 0; i < target_num; i++) {
            target_nodes[i] = target_data[i];
        }

        for (int gate_index : gate_forward_order) {
            gate_list[gate_index].get_output_value();
        }
    }
    
    void print_masked_circuit() {
        // Implement the print_masked_circuit logic
    }
    
    int get_node_num() const {
        return node_list.size();
    }
    
    int get_gate_num() const {
        return gate_list.size();
    }
};

int main() {
    std::unordered_map<std::string, int> circuit_info;
    // Initialize circuit_info with the required values
    
    LogicTree logic_tree(circuit_info, true);
    
    // Use the logic_tree object to perform operations
    
    return 0;
}
