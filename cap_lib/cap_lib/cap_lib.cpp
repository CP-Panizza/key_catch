// cap_lib.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

#include "cap_lib.h"

HBITMAP cap_screen() {
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


	int srcxsize = GetSystemMetrics(SM_CXSCREEN);//��ȡ��Ļ�豸�ߴ���Ϣ
	int srcysize = GetSystemMetrics(SM_CYSCREEN);
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
	return bitmap;
}

void free_img(HBITMAP data) {
	DeleteObject(data);
}
