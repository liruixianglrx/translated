#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

const int INIT_LEVEL = 9999;

class Gate;

class Node {
private:
    std::string node_name;
    int level;
    int value;
    int driver_index;
    std::vector<int> load_index;
public:
    Node(std::string name) : node_name(name), level(INIT_LEVEL), value(-1), driver_index(-1) {}
    void set_driver(int gate_index) {
        assert(driver_index == -1);
        driver_index = gate_index;
    }
    void add_load(int load_index) {
        this->load_index.push_back(load_index);
    }
    void set_level(int level) {
        assert(!level_set_done());
        this->level = level;
    }
    int get_level(const std::vector<Node>& node_list, const std::vector<Gate>& gate_list) {
        if (level_set_done()) {
            return level;
        }
        else {
            const Gate& driver_gate = gate_list[driver_index];
            std::vector<int> input_node_levels;
            for (int driver_gate_input : driver_gate.get_input_index()) {
                const Node& input_node = node_list[driver_gate_input];
                input_node_levels.push_back(input_node.get_level(node_list, gate_list));
            }
            set_level(*std::max_element(input_node_levels.begin(), input_node_levels.end()) + 1);
            return level;
        }
    }
    void set_value(int value) {
        this->value = value;
    }
    void clear_value() {
        value = -1;
    }
    bool level_set_done() const {
        return level != INIT_LEVEL;
    }
    std::string get_name() const {
        return node_name;
    }
    int get_level() const {
        return level;
    }
    int get_value() const {
        return value;
    }
    int get_driver_index() const {
        return driver_index;
    }
    const std::vector<int>& get_load_index() const {
        return load_index;
    }
    std::string to_string() const {
        return "[Node: " + node_name + "][Lv: " + std::to_string(level) + "], Value: " + std::to_string(value) + ", DRV: " + std::to_string(driver_index) + ", LD: " + vector_to_string(load_index) + "]";
    }
    bool operator==(const Node& other) const {
        return node_name == other.node_name;
    }
    bool operator<(const Node& other) const {
        return node_name < other.node_name;
    }
    std::string vector_to_string(const std::vector<int>& vec) const {
        std::string result = "[";
        for (int i : vec) {
            result += std::to_string(i) + " ";
        }
        result += "]";
        return result;
    }
};

std::ostream& operator<<(std::ostream& os, const Node& node) {
    os << node.to_string();
    return os;
}
