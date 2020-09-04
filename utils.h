//
// Created by Administrator on 2020/6/4.
//

#ifndef CONVELUTION_UTILS_H
#define CONVELUTION_UTILS_H

#include <vector>
#include <string>
#include <cmath>

template< class T>
bool have(std::vector<T> &src, T target){
    for(auto &e:src){
        if(e == target){
            return true;
        }
    }
    return false;
}

std::vector<std::string> split(std::string str, std::string pattern);



std::string &replace_all(std::string &str, const std::string &old_value, const std::string &new_value);


#endif //CONVELUTION_UTILS_H
