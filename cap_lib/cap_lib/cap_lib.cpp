// cap_lib.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"

#include "cap_lib.h"

HBITMAP cap_screen() {
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


	int srcxsize = GetSystemMetrics(SM_CXSCREEN);//获取屏幕设备尺寸信息
	int srcysize = GetSystemMetrics(SM_CYSCREEN);
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
	return bitmap;
}

void free_img(HBITMAP data) {
	DeleteObject(data);
}
