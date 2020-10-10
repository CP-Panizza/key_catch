//
// Created by Administrator on 2020/6/3.
//


#include <ctime>
#include "utils.h"
#include <cmath>
#include <cstdio>
#include <sstream>
#include <random>
#include <vector>
#include <map>

std::string rgb2hex(int r, int g, int b)
{

    if(r < 0 || g < 0 || b < 0 || r > 255 || g > 255 || b > 255)
    {
        return "rgb value not right";
    }
    std::map<int, std::string> val_map{
                                        {0,"0"},
                                        {1,"1"},
                                        {2,"2"},
                                        {3,"3"},
                                        {4,"4"},
                                        {5,"5"},
                                        {6,"6"},
                                        {7,"7"},
                                        {8,"8"},
                                        {9,"9"},
                                        {10,"A"},
                                        {11,"B"},
                                        {12,"C"},
                                        {13,"D"},
                                        {14,"E"},
                                        {15,"F"}
                                    };
    std::string hex = "#"
    + std::string(std::string(val_map[int(r / 16)])
    + std::string(val_map[r % 16]))
    + std::string(std::string(val_map[int(g / 16)])
    + std::string(val_map[g % 16]))
    + std::string(std::string(val_map[int(b / 16)])
    + std::string(val_map[b % 16]));
    return hex;
}



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



