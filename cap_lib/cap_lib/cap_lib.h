#ifndef _CAP_LIB_H_
#define _CAP_LIB_H_

#include <windows.h>

extern "C" __declspec(dllexport) HBITMAP cap_screen();
extern "C" __declspec(dllexport) void free_img(HBITMAP data);
#endif  //_CAP_LIB_H_

