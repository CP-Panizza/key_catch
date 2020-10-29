#ifndef _CAP_LIB_H_
#define _CAP_LIB_H_

#include <windows.h>

extern "C" __declspec(dllexport) void init_cap_screen();
extern "C" __declspec(dllexport) unsigned char* cap_screen(int *width, int *height, long *len);
extern "C" __declspec(dllexport) void free_buffer(unsigned char * data);
void free_img(HBITMAP data);
#endif  //_CAP_LIB_H_

