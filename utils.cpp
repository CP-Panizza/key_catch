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
std::string rgb2hex(int r, int g, int b)
{
    if(r < 0 || g < 0 || b < 0 || r > 255 || g > 255 || b > 255)
    {
        return std::string("rgb value not right");
    }

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <direct.h>
#include <errno.h>

//判断是否是".."目录和"."目录
bool is_special_dir(const char *path)
{
    return strcmp(path, "..") == 0 || strcmp(path, ".") == 0;
}

//判断文件属性是目录还是文件
 bool is_dir(int attrib)
{
    return attrib == 16 || attrib == 18 || attrib == 20;
}

//显示删除失败原因
 void show_error(const char *file_name)
{
    errno_t err;
    _get_errno(&err);
    switch(err)
    {
    case ENOTEMPTY:
        printf("Given path is not a directory, the directory is not empty, or the directory is either the current working directory or the root directory.\n");
        break;
    case ENOENT:
        printf("Path is invalid.\n");
        break;
    case EACCES:
        printf("%s had been opend by some application, can't delete.\n", file_name);
        break;
    }
}

 void get_file_path(const char *path, const char *file_name, char *file_path)
{
    strcpy_s(file_path, sizeof(char) * _MAX_PATH, path);
    file_path[strlen(file_path) - 1] = '\0';
    strcat_s(file_path, sizeof(char) * _MAX_PATH, file_name);
    strcat_s(file_path, sizeof(char) * _MAX_PATH, "\\*");
}

//递归搜索目录中文件并删除
 void delete_file(char *path, char *removeshot)
{

    _finddata_t dir_info;
    _finddata_t file_info;
    intptr_t f_handle;
    char tmp_path[_MAX_PATH];
    remove(removeshot);
    if((f_handle = _findfirst(path, &dir_info)) != -1)
    {
        while(_findnext(f_handle, &file_info) == 0)
        {
            if(is_special_dir(file_info.name))
                continue;
            if(is_dir(file_info.attrib))//如果是目录，生成完整的路径
            {
                get_file_path(path, file_info.name, tmp_path);
                delete_file(tmp_path, removeshot); //开始递归删除目录中的内容
                tmp_path[strlen(tmp_path) - 2] = '\0';
                if(file_info.attrib == 20)
                    printf("This is system file, can't delete!\n");
                else
                {
                    //删除空目录，必须在递归返回前调用_findclose,否则无法删除目录
                    if(_rmdir(tmp_path) == -1)
                    {
                        show_error();//目录非空则会显示出错原因
                    }
                }
            }
            else
            {
                strcpy_s(tmp_path, path);
                tmp_path[strlen(tmp_path) - 1] = '\0';
                strcat_s(tmp_path, file_info.name);//生成完整的文件路径

                if(remove(tmp_path) == -1)
                {
                    show_error(file_info.name);
                }

            }
        }
        _findclose(f_handle);//关闭打开的文件句柄，并释放关联资源，否则无法删除空目录
    }
    else
    {
        show_error();//若路径不存在，显示错误信息
    }
}



