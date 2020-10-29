// cap_lib.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include "cap_lib.h"
#include "toojpeg.h"
#include <vector>
#include <iostream>


int srcxsize = 0;
int srcysize = 0;
BITMAPINFOHEADER bi;
void init_cap_screen(){
	srcxsize = GetSystemMetrics(SM_CXSCREEN);//获取屏幕设备尺寸信息
	srcysize = GetSystemMetrics(SM_CYSCREEN);
}


unsigned char * cap_screen(int *width, int *height, long *len) {
	
	CURSORINFO pci;//定义光标结构体信息
				   //获取结构体大小，这一步必须要。否则后面就无法获取句柄
	pci.cbSize = sizeof(CURSORINFO);
	GetCursorInfo(&pci);//获取光标信息
	POINT point = pci.ptScreenPos;//光标当前所在坐标位置

	ICONINFO iconinfo;

	GetIconInfo(pci.hCursor, &iconinfo);//主要用于获取光标对应的位图句柄，后面会在屏幕画出。
	HDC hdc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	HDC memdc = CreateCompatibleDC(hdc);//创建兼容屏幕的内存DC
	HDC memdc1 = CreateCompatibleDC(hdc);
	HDC memdc2 = CreateCompatibleDC(hdc);
	SelectObject(memdc, iconinfo.hbmMask);//白底黑鼠标
	SelectObject(memdc1, iconinfo.hbmColor);//黑底彩色鼠标


	
	//创建与屏幕DC兼容的DIB位图，初始化尺寸为屏幕尺寸
	HBITMAP bitmap = CreateCompatibleBitmap(hdc, srcxsize, srcysize);
	SelectObject(memdc2, bitmap);

	//将屏幕位图拷贝到兼容内存DC中。
	BitBlt(memdc2, 0, 0, srcxsize, srcysize, hdc, 0, 0, SRCCOPY);

	//将光标画到屏幕所在位置
	BitBlt(memdc2, point.x, point.y, 20, 20, memdc, 0, 0, SRCAND);
	BitBlt(memdc2, point.x, point.y, 20, 20, memdc1, 0, 0, SRCPAINT);//将带有鼠标的屏幕位图抓取

	DeleteDC(memdc2);
	DeleteDC(memdc1);
	DeleteDC(memdc);
	DeleteDC(hdc);

	BITMAP Bitmap;
	GetObject(bitmap, sizeof(BITMAP), (LPSTR)&Bitmap);

	*width = Bitmap.bmWidth;
	*height = Bitmap.bmHeight;
	long Count = Bitmap.bmHeight * Bitmap.bmWidthBytes; //得到buf大小 
	unsigned char *buffer = new unsigned char[Count];

	GetBitmapBits(bitmap, Count, buffer); //拷贝BGR数据到buffer
	int count = Count / 4;
	long length = count * 3;
	unsigned char *pixels = new unsigned char[length];

	//Bitmap像素点的色彩通道排列顺序是RGBA
	for (int i = 0; i < count; i++) {
		pixels[i * 3] = buffer[i * 4 + 2];  //R
		pixels[i * 3 + 1] = buffer[i * 4 + 1]; //G
		pixels[i * 3 + 2] = buffer[i * 4];    //B
	}
	*len = length;
	delete[](buffer);
	free_img(bitmap);
	return pixels;
}

void free_img(HBITMAP data) {
	DeleteObject(data);
}





void free_buffer(unsigned char * data) {
	delete[](data);
}

//No2

//struct cap_screen_t
//{
//	HDC memdc;
//	HBITMAP hbmp;
//	unsigned char* buffer;
//	int            length;
//
//	int width;
//	int height;
//	int bitcount;
//	int left, top;
//};
//
//
//int init_cap_screen(struct cap_screen_t* sc)
//{
//	DEVMODE devmode;
//	BOOL bRet;
//	BITMAPINFOHEADER bi;
//	int srcxsize = GetSystemMetrics(SM_CXSCREEN);//获取屏幕设备尺寸信息
//	int srcysize = GetSystemMetrics(SM_CYSCREEN);
//	sc->width = srcxsize;
//	sc->height = srcysize;
//	sc->bitcount = 16;
//	sc->left = 0;
//	sc->top = 0;
//	memset(&bi, 0, sizeof(bi));
//	bi.biSize = sizeof(bi);
//	bi.biWidth = sc->width;
//	bi.biHeight = -sc->height; //从上朝下扫描
//	bi.biPlanes = 1;
//	bi.biBitCount = sc->bitcount; //RGB
//	bi.biCompression = BI_RGB;
//	bi.biSizeImage = 0;
//	HDC hdc = GetDC(NULL); //屏幕DC
//	sc->memdc = CreateCompatibleDC(hdc);
//	sc->buffer = NULL;
//	sc->hbmp = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&sc->buffer, NULL, 0);
//	ReleaseDC(NULL, hdc);
//	SelectObject(sc->memdc, sc->hbmp); ///
//	sc->length = sc->height * (((sc->width*sc->bitcount / 8) + 3) / 4 * 4);
//	return 0;
//}
//
//HCURSOR FetchCursorHandle()
//{
//	CURSORINFO hCur;
//	ZeroMemory(&hCur, sizeof(hCur));
//	hCur.cbSize = sizeof(hCur);
//	GetCursorInfo(&hCur);
//	return hCur.hCursor;
//}
//
//void AddCursor(HDC hMemDC, POINT origin)
//{
//	POINT xPoint;
//	GetCursorPos(&xPoint);
//	xPoint.x -= origin.x;
//	xPoint.y -= origin.y;
//	if (xPoint.x < 0 || xPoint.y < 0)
//		return;
//	HCURSOR hcur = FetchCursorHandle();
//	ICONINFO iconinfo;
//	BOOL ret;
//	ret = GetIconInfo(hcur, &iconinfo);
//	if (ret)
//	{
//		xPoint.x -= iconinfo.xHotspot;
//		xPoint.y -= iconinfo.yHotspot;
//		if (iconinfo.hbmMask) DeleteObject(iconinfo.hbmMask);
//		if (iconinfo.hbmColor) DeleteObject(iconinfo.hbmColor);
//	}
//	DrawIcon(hMemDC, xPoint.x, xPoint.y, hcur);
//}
//
//HBITMAP cap_screen(struct cap_screen_t* sc)
//{
//	HDC hdc = GetDC(NULL);
//
//	BitBlt(sc->memdc, 0, 0, sc->width, sc->height, hdc, sc->left, sc->top, SRCCOPY); // 截屏
//	AddCursor(sc->memdc, POINT{ sc->left, sc->top }); // 增加鼠标进去
//	ReleaseDC(NULL, hdc);
//	return sc->hbmp;
//}