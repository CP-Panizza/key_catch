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

std::string rgb2hex(int r, int g, int b);

std::vector<std::string> split(std::string str, std::string pattern);

std::string &replace_all(std::string &str, const std::string &old_value, const std::string &new_value);

bool is_special_dir(const char *path);

bool is_dir(int attrib);

void show_error(const char *file_name = NULL);

void get_file_path(const char *path, const char *file_name, char *file_path);

void delete_file(char *path, char *removeshot);

bool file_exists(const std::string &name);

bool dir_exists(std::string path);

long file_size(const char *filepath);

void trim_space(std::string &s);


std::string read_file(std::string file);

#endif //CONVELUTION_UTILS_H
