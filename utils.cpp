//
// Created by Administrator on 2020/6/3.
//


#include <ctime>
#include "utils.h"
#include <cmath>
#include <cstdio>
#include <sstream>
#include <random>

std::vector<std::string> split(std::string str, std::string pattern) {
    std::string::size_type pos;
    std::vector<std::string> result;
    str += pattern;//扩展字符串以方便操作
    auto size = static_cast<int>(str.size());
    for (int i = 0; i < size; i++) {
        pos = str.find(pattern, i);
        if (pos < size) {
            std::string s = str.substr(i, pos - i);
            result.push_back(s);
            i = static_cast<int>(pos + pattern.size() - 1);
        }
    }
    return result;
}



std::string &replace_all(std::string &str, const std::string &old_value, const std::string &new_value) {
    while (true) {
        std::string::size_type pos(0);
        if ((pos = str.find(old_value)) != std::string::npos) {
            str.replace(pos, old_value.length(), new_value);
        } else { break; }
    }
    return str;
}



