#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <tuple>
#include <string>


std::vector<std::string> split_string(const std::string& str, char delimiter);


std::tuple<std::string, std::string> read_cookie();

#endif
