#include <fstream>
#include <sstream>
#include "utils.h"


std::vector<std::string> split_string(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}


std::tuple<std::string, std::string> read_cookie() {
    std::string rel_path = ".bitcoin/regtest/.cookie";
    std::string home_path = getenv("HOME");
    std::string cookie_path = home_path + "/" + rel_path;

    std::ifstream file {cookie_path};
    if (!file.is_open()) {
        throw std::runtime_error("Could not open cookie file");
    }

    std::string line;
    std::getline(file, line);

    auto tokens = split_string(line, ':');
    return {tokens.at(0), tokens.at(1)};
}
