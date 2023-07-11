#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

#include "gate.h"
#include "node.h"
#include "utils.h"

class Gate {
protected:
    std::vector<int> input_index;
    int output_index;
    std::vector<Node> node_list;
    std::string id;

public:
    Gate(const std::vector<int>& input_index, int output_index, const std::vector<Node>& node_list)
        : input_index(input_index), output_index(output_index), node_list(node_list), id(random_id(2)) {}

    virtual bool backward_done() const {
        for (int i : input_index) {
            if (!node_list[i].level_set_done) {
                return false;
            }
        }
        return true;
    }

    virtual int get_output_value() = 0;

    void set_output_value(int value) {
        assert(value == 0 || value == 1);
        node_list[output_index].set_value(value);
    }

    std::string to_string() const {
        return "[" + typeid(*this).name() + "_" + id + "] inputs: " + vector_to_string(input_index) +
               ", output: " + std::to_string(output_index);
    }

    bool operator<(const Gate& other) const {
        const Node& self_out = node_list[output_index];
        const Node& other_out = node_list[other.output_index];
        return self_out.level < other_out.level;
    }
};

class Not : public Gate {
public:
    Not(const std::vector<int>& input_index, int output_index, const std::vector<Node>& node_list)
        : Gate(input_index, output_index, node_list) {}

    int get_output_value() override {
        int input_value = node_list[input_index[0]].value;
        if (input_value == 1) {
            set_output_value(0);
            return 0;
        } else {
            set_output_value(1);
            return 1;
        }
    }
};

class Buf : public Gate {
public:
    Buf(const std::vector<int>& input_index, int output_index, const std::vector<Node>& node_list)
        : Gate(input_index, output_index, node_list) {}

    int get_output_value() override {
        int input_value = node_list[input_index[0]].value;
        set_output_value(input_value);
        return input_value;
    }
};

class And : public Gate {
public:
    And(const std::vector<int>& input_index, int output_index, const std::vector<Node>& node_list)
        : Gate(input_index, output_index, node_list) {}

    int get_output_value() override {
        for (int i : input_index) {
            if (node_list[i].value == 0) {
                set_output_value(0);
                return 0;
            }
        }
        set_output_value(1);
        return 1;
    }
};

class Nand : public Gate {
public:
    Nand(const std::vector<int>& input_index, int output_index, const std::vector<Node>& node_list)
        : Gate(input_index, output_index, node_list) {}

    int get_output_value() override {
        for (int i : input_index) {
            if (node_list[i].value == 0) {
                set_output_value(1);
                return 1;
            }
        }
        set_output_value(0);
        return 0;
    }
};

class Or : public Gate {
public:
    Or(const std::vector<int>& input_index, int output_index, const std::vector<Node>& node_list)
        : Gate(input_index, output_index, node_list) {}

    int get_output_value() override {
        for (int i : input_index) {
            if (node_list[i].value == 1) {
                set_output_value(1);
                return 1;
            }
        }
        set_output_value(0);
        return 0;
    }
};

class Nor : public Gate {
public:
    Nor(const std::vector<int>& input_index, int output_index, const std::vector<Node>& node_list)
        : Gate(input_index, output_index, node_list) {}

    int get_output_value() override {
        for (int i : input_index) {
            if (node_list[i].value == 1) {
                set_output_value(0);
                return 0;
            }
        }
        set_output_value(1);
        return 1;
    }
};

class Xor : public Gate {
public:
    Xor(const std::vector<int>& input_index, int output_index, const std::vector<Node>& node_list)
        : Gate(input_index, output_index, node_list) {}

    int get_output_value() override {
        int value_0 = node_list[input_index[0]].value;
        int value_1 = node_list[input_index[1]].value;
        if (value_0 != value_1) {
            set_output_value(1);
            return 1;
        } else {
            set_output_value(0);
            return 0;
        }
    }
};

class Xnor : public Gate {
public:
    Xnor(const std::vector<int>& input_index, int output_index, const std::vector<Node>& node_list)
        : Gate(input_index, output_index, node_list) {}

    int get_output_value() override {
        int value_0 = node_list[input_index[0]].value;
        int value_1 = node_list[input_index[1]].value;
        if (value_0 == value_1) {
            set_output_value(1);
            return 1;
        } else {
            set_output_value(0);
            return 0;
        }
    }
};
