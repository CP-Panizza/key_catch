// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� IMG_LIB_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// IMG_LIB_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef IMG_LIB_EXPORTS
#define IMG_LIB_API __declspec(dllexport)
#else
#define IMG_LIB_API __declspec(dllimport)
#endif


extern "C" IMG_LIB_API double* CapImg(int *, int *);
extern "C" IMG_LIB_API void DLLFree(void *);

