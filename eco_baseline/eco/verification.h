#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>

class BaseNode {
    // Implementation of the BaseNode class
};

void gen_miter_verilog(int input_width, int output_width) {
    // Implementation of the gen_miter_verilog function
}

class Miter {
public:
    Miter(
        const std::string& origin_file_path,
        const std::string& golden_file_path,
        int input_num,
        int output_num
    ) :
        origin_file_path_(origin_file_path),
        input_num_(input_num),
        output_num_(output_num)
    {
        std::ifstream origin_file(origin_file_path_);
        std::string line;
        while (std::getline(origin_file, line)) {
            origin_.push_back(line);
        }
        origin_[0].replace(origin_[0].find("top"), 3, "F");

        std::ifstream golden_file(golden_file_path);
        while (std::getline(golden_file, line)) {
            golden_.push_back(line);
        }
        golden_[0].replace(golden_[0].find("top"), 3, "G");

        if (std::filesystem::exists("tmp")) {
            std::filesystem::remove_all("tmp");
        }
        std::filesystem::create_directory("tmp");

        std::ofstream g_file("./tmp/G.v");
        for (const auto& line : golden_) {
            g_file << line << '\n';
        }

        gen_miter_verilog(input_num_, output_num_);

        std::ofstream sby_file("./tmp/verif.sby");
        sby_file << VERIF_FORMULA;
        sby_file.close();
    }

    bool verify(BaseNode& candidate, bool silent = true) {
        std::cout << "Miter is verifying: " << candidate.simple_str << std::endl;

        candidate.gen_patch();
        std::string patch_path = "./tmp/minterm.v";

        std::string patch_inst = "patch U_patch(";
        for (const auto& node : candidate.nodes_name) {
            patch_inst += node + ", ";
        }
        patch_inst += "t_0); \n";

        std::vector<std::string> cp_og = origin_;

        cp_og.insert(cp_og.end() - 2, patch_inst);

        std::ofstream f_file("./tmp/F.v");
        for (const auto& line : cp_og) {
            f_file << line << '\n';
        }

        std::string cmd = "sby -j 8 -f ./tmp/verif.sby";
        if (silent) {
            cmd += " > /dev/null";
        }

        int result = std::system(cmd.c_str());
        if (result == 0) {
            std::cout << "Verification Success !" << std::endl;
            return true;
        } else {
            std::cout << "Verification Fail !" << std::endl;
            return false;
        }
    }

private:
    std::string origin_file_path_;
    int input_num_;
    int output_num_;
    std::vector<std::string> origin_;
    std::vector<std::string> golden_;
};

// int main() {
//     std::string origin_file_path = "origin.txt";
//     std::string golden_file_path = "golden.txt";
//     int input_num = 4;
//     int output_num = 2;

//     Miter miter(origin_file_path, golden_file_path, input_num, output_num);

//     BaseNode candidate;
//     bool silent = true;
//     bool verification_result = miter.verify(candidate, silent);

//     return 0;
// }
