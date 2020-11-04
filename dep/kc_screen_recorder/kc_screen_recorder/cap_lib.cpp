// cap_lib.cpp : ���� DLL Ӧ�ó���ĵ���������
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
	srcxsize = GetSystemMetrics(SM_CXSCREEN);//��ȡ��Ļ�豸�ߴ���Ϣ
	srcysize = GetSystemMetrics(SM_CYSCREEN);
}


unsigned char * cap_screen(int *width, int *height, long *len) {
	
	CURSORINFO pci;//������ṹ����Ϣ
				   //��ȡ�ṹ���С����һ������Ҫ�����������޷���ȡ���
	pci.cbSize = sizeof(CURSORINFO);
	GetCursorInfo(&pci);//��ȡ�����Ϣ
	POINT point = pci.ptScreenPos;//��굱ǰ��������λ��

	ICONINFO iconinfo;

	GetIconInfo(pci.hCursor, &iconinfo);//��Ҫ���ڻ�ȡ����Ӧ��λͼ��������������Ļ������
	HDC hdc = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	HDC memdc = CreateCompatibleDC(hdc);//����������Ļ���ڴ�DC
	HDC memdc1 = CreateCompatibleDC(hdc);
	HDC memdc2 = CreateCompatibleDC(hdc);
	SelectObject(memdc, iconinfo.hbmMask);//�׵׺����
	SelectObject(memdc1, iconinfo.hbmColor);//�ڵײ�ɫ���


	
	//��������ĻDC���ݵ�DIBλͼ����ʼ���ߴ�Ϊ��Ļ�ߴ�
	HBITMAP bitmap = CreateCompatibleBitmap(hdc, srcxsize, srcysize);
	SelectObject(memdc2, bitmap);

	//����Ļλͼ�����������ڴ�DC�С�
	BitBlt(memdc2, 0, 0, srcxsize, srcysize, hdc, 0, 0, SRCCOPY);

	//����껭����Ļ����λ��
	BitBlt(memdc2, point.x, point.y, 20, 20, memdc, 0, 0, SRCAND);
	BitBlt(memdc2, point.x, point.y, 20, 20, memdc1, 0, 0, SRCPAINT);//������������Ļλͼץȡ

	DeleteDC(memdc2);
	DeleteDC(memdc1);
	DeleteDC(memdc);
	DeleteDC(hdc);

	BITMAP Bitmap;
	GetObject(bitmap, sizeof(BITMAP), (LPSTR)&Bitmap);

	*width = Bitmap.bmWidth;
	*height = Bitmap.bmHeight;
	long Count = Bitmap.bmHeight * Bitmap.bmWidthBytes; //�õ�buf��С 
	unsigned char *buffer = new unsigned char[Count];

	GetBitmapBits(bitmap, Count, buffer); //����BGR���ݵ�buffer
	int count = Count / 4;
	long length = count * 3;
	unsigned char *pixels = new unsigned char[length];

	//Bitmap���ص��ɫ��ͨ������˳����RGBA
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
//	int srcxsize = GetSystemMetrics(SM_CXSCREEN);//��ȡ��Ļ�豸�ߴ���Ϣ
//	int srcysize = GetSystemMetrics(SM_CYSCREEN);
//	sc->width = srcxsize;
//	sc->height = srcysize;
//	sc->bitcount = 16;
//	sc->left = 0;
//	sc->top = 0;
//	memset(&bi, 0, sizeof(bi));
//	bi.biSize = sizeof(bi);
//	bi.biWidth = sc->width;
//	bi.biHeight = -sc->height; //���ϳ���ɨ��
//	bi.biPlanes = 1;
//	bi.biBitCount = sc->bitcount; //RGB
//	bi.biCompression = BI_RGB;
//	bi.biSizeImage = 0;
//	HDC hdc = GetDC(NULL); //��ĻDC
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
//	BitBlt(sc->memdc, 0, 0, sc->width, sc->height, hdc, sc->left, sc->top, SRCCOPY); // ����
//	AddCursor(sc->memdc, POINT{ sc->left, sc->top }); // ��������ȥ
//	ReleaseDC(NULL, hdc);
//	return sc->hbmp;
//}