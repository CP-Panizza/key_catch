#include "stdafx.h"
#include <windows.h>
#include <conio.h>
#include <errno.h>
#include <stdio.h>
#include <mmsystem.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <csignal>
#include <iostream>
#include "record_lib.h"

#pragma comment(lib, "winmm.lib")//����������lib



struct WAVFILEHEADER
{
	// RIFF ͷ;
	char RiffName[4];
	unsigned long nRiffLength;
	// �������ͱ�ʶ��;
	char WavName[4];
	// ��ʽ���еĿ�ͷ;
	char FmtName[4];
	unsigned long nFmtLength;
	// ��ʽ���еĿ�����;
	unsigned short nAudioFormat;
	unsigned short nChannleNumber;
	unsigned long nSampleRate;
	unsigned long nBytesPerSecond;
	unsigned short nBytesPerSample;
	unsigned short nBitsPerSample;
	// ���ݿ��еĿ�ͷ;
	char    DATANAME[4];
	unsigned long   nDataLength;
};

int BUFFSIZE = 1024 * 1024; //���λ������Ĵ�С������Զ����һЩ

int channelCount = 1;
int simpleSize = 16;
int simpleRate = 16000;

//#define min(x, y) ((x) < (y) ? (x) : (y))//���������VS����һ��ͬ���ĺ꣬����ע�͵��ˡ�



char * ReadFile(char * path, int *length)
{
	FILE * pfile;
	char * data;
	pfile = fopen(path, "rb");
	if (pfile == NULL)
	{
		return NULL;
	}
	fseek(pfile, 0, SEEK_END);
	*length = ftell(pfile);
	data = (char *)malloc((*length + 1) * sizeof(char));
	rewind(pfile);
	*length = fread(data, 1, *length, pfile);
	data[*length] = '\0';
	fclose(pfile);
	return data;
}

//���λ������ĵ����ݽṹ
struct cycle_buffer {
	char *buf;
	unsigned int size;
	unsigned int in;
	unsigned int out;
};

static struct cycle_buffer *fifo = NULL;//����ȫ��FIFO
FILE *dat_fp = NULL;
CRITICAL_SECTION cs;

//��ʼ�����λ�����
static int init_cycle_buffer(void)
{
	int size = BUFFSIZE, ret;

	ret = size & (size - 1);
	if (ret)
		return ret;
	fifo = (struct cycle_buffer *) malloc(sizeof(struct cycle_buffer));
	if (!fifo)
		return -1;

	memset(fifo, 0, sizeof(struct cycle_buffer));
	fifo->size = size;
	fifo->in = fifo->out = 0;
	fifo->buf = (char *)malloc(size);
	if (!fifo->buf)
		free(fifo);
	else
		memset(fifo->buf, 0, size);
	return 0;
}

unsigned int fifo_get(char *buf, unsigned int len)  //�ӻ��λ�������ȡ����
{
	unsigned int l;
	len = min(len, fifo->in - fifo->out);
	l = min(len, fifo->size - (fifo->out & (fifo->size - 1)));
	memcpy(buf, fifo->buf + (fifo->out & (fifo->size - 1)), l);
	memcpy(buf + l, fifo->buf, len - l);
	fifo->out += len;
	return len;
}

unsigned int fifo_put(char *buf, unsigned int len) //�����ݷ��뻷�λ�����
{
	unsigned int l;
	len = min(len, fifo->size - fifo->in + fifo->out);
	l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));
	memcpy(fifo->buf + (fifo->in & (fifo->size - 1)), buf, l);
	memcpy(fifo->buf, buf + l, len - l);
	fifo->in += len;
	return len;
}

void WaveInitFormat(LPWAVEFORMATEX m_WaveFormat, WORD nCh, DWORD nSampleRate, WORD BitsPerSample)//��ʼ����Ƶ��ʽ
{
	m_WaveFormat->wFormatTag = WAVE_FORMAT_PCM;
	m_WaveFormat->nChannels = nCh;
	m_WaveFormat->nSamplesPerSec = nSampleRate;
	m_WaveFormat->nAvgBytesPerSec = nSampleRate * nCh * BitsPerSample / 8;
	m_WaveFormat->nBlockAlign = m_WaveFormat->nChannels * BitsPerSample / 8;
	m_WaveFormat->wBitsPerSample = BitsPerSample;
	m_WaveFormat->cbSize = 0;
}

DWORD CALLBACK MicCallback(HWAVEIN hwavein, UINT uMsg, DWORD_PTR  dwInstance, DWORD_PTR  dwParam1, DWORD_PTR  dwParam2)//�ص����������ݻ���������ʱ��ͻᴥ�����ص�������ִ�������RecordWave����֮���൱�ڴ�����һ���߳�
{
	int len = 0;
	switch (uMsg)
	{
	case WIM_OPEN://���豸ʱ�����֧��ִ�С�
		printf("\n�豸�Ѿ���...\n");
		break;
	case WIM_DATA://������������ʱ�������֧��ִ�У���Ҫ�������֧�г���������䣬�ᶪ����	��waveform audio����û�л�����ơ�		
		printf("\n������%d����...\n", ((LPWAVEHDR)dwParam1)->dwUser);
		waveInAddBuffer(hwavein, (LPWAVEHDR)dwParam1, sizeof(WAVEHDR)); // api

		EnterCriticalSection(&cs); //�����ٽ���  // api
		len = fifo_put(((LPWAVEHDR)dwParam1)->lpData, 10240); //��������������д�뻷��fifo
		LeaveCriticalSection(&cs);//�˳��ٽ���  // api
		break;
	case WIM_CLOSE:
		printf("\n�豸�Ѿ��ر�...\n");
		break;
	default:
		break;
	}
	return 0;
}

HWAVEIN phwi;
WAVEHDR pwh1;
WAVEHDR pwh2;
void RecordWave()
{

	WAVEINCAPS waveIncaps;
	int count = 0;
	MMRESULT mmResult;
	count = waveInGetNumDevs();//��ȡϵͳ�ж��ٸ����� // api

	mmResult = waveInGetDevCaps(0, &waveIncaps, sizeof(WAVEINCAPS));//�鿴ϵͳ�����豸����������̫����������������

	printf("\ndevs count = %d\n", count);
	printf("\nwaveIncaps.szPname=%s\n", waveIncaps.szPname);

	if (MMSYSERR_NOERROR == mmResult)
	{
		WAVEFORMATEX pwfx;
		WaveInitFormat(&pwfx, channelCount, simpleRate, simpleSize);
		printf("\n�������Ƶ�����豸");
		mmResult = waveInOpen(&phwi, WAVE_MAPPER, &pwfx, (DWORD_PTR)(MicCallback), NULL, CALLBACK_FUNCTION);//����Ƶ�豸�����ûص�����

		if (MMSYSERR_NOERROR == mmResult)
		{

			char buffer1[10240];

			char buffer2[10240];

			pwh1.lpData = buffer1;
			pwh1.dwBufferLength = 10240;
			pwh1.dwUser = 1;
			pwh1.dwFlags = 0;
			mmResult = waveInPrepareHeader(phwi, &pwh1, sizeof(WAVEHDR));//׼��������
			printf("\n׼��������1");

			pwh2.lpData = buffer2;
			pwh2.dwBufferLength = 10240;
			pwh2.dwUser = 2;
			pwh2.dwFlags = 0;
			mmResult = waveInPrepareHeader(phwi, &pwh2, sizeof(WAVEHDR));//
			printf("\n׼��������2\n");

			if (MMSYSERR_NOERROR == mmResult)
			{
				mmResult = waveInAddBuffer(phwi, &pwh1, sizeof(WAVEHDR));//��ӻ�����
				mmResult = waveInAddBuffer(phwi, &pwh2, sizeof(WAVEHDR));

				printf("\n��������2������Ƶ�����豸\n");
				if (MMSYSERR_NOERROR == mmResult)
				{
					mmResult = waveInStart(phwi);
					printf("\n��ʼ¼��\n");
				}
			}
		}
	}
}

bool g_stop = false;

//����ж��źţ�����¼�����˳�����
void stop_record() {
	printf("enter stop\n");
	g_stop = true;
	Sleep(100);
	waveInStop(phwi);//ֹͣ¼��
					 //waveInReset(phwi);
	waveInClose(phwi);//�ر���Ƶ�豸
	waveInUnprepareHeader(phwi, &pwh1, sizeof(WAVEHDR));//�ͷ�buffer
	waveInUnprepareHeader(phwi, &pwh2, sizeof(WAVEHDR));
	printf("stop capture!\n");
	fflush(stdout);

	WAVFILEHEADER WavFileHeader;
	strcpy(WavFileHeader.RiffName, "RIFF");
	strcpy(WavFileHeader.WavName, "WAVE");
	strcpy(WavFileHeader.FmtName, "fmt ");

	WavFileHeader.nFmtLength = 16;  //  ��ʾ FMT �ĳ���
	WavFileHeader.nAudioFormat = 1; //�����ʾ PCM ����;
	strcpy(WavFileHeader.DATANAME, "data");
	WavFileHeader.nBitsPerSample = simpleSize;
	WavFileHeader.nBytesPerSample = channelCount * simpleSize / 8;
	WavFileHeader.nSampleRate = simpleRate;
	WavFileHeader.nBytesPerSecond = simpleRate * (channelCount*simpleSize / 8);
	WavFileHeader.nChannleNumber = channelCount;

	auto out_file = fopen("tmp.wav", "wb");
	int nFileLen = 0;
	char * data = ReadFile("tmp.dat", &nFileLen);

	int nSize = sizeof(WavFileHeader);
	WavFileHeader.nRiffLength = nFileLen - 8 + nSize;
	WavFileHeader.nDataLength = nFileLen;
	fwrite((char *)&WavFileHeader, nSize, 1, out_file);
	fwrite(data, nFileLen, 1, out_file);
	fclose(dat_fp);
	fclose(out_file);
	delete[](data);
}


void forRec()//�½�����һ���߳����ڽ�����д���ļ�
{
	char buff[10240] = { 0 };
	int len = 0;
	while (!g_stop)
	{
		EnterCriticalSection(&cs); //�����ٽ���			
		len = fifo_get(buff, 10240);//��fifo�л�ȡ����			
		LeaveCriticalSection(&cs);//�뿪�ٽ���
		fwrite(buff, len, 1, dat_fp);//����Ƶ����д����Ƶ�ļ�				
		Sleep(100);
	}
}

bool start_record() {
	//signal(SIGINT, stopRecord);
	char buff[10240] = { 0 };
	InitializeCriticalSection(&cs);//��ʼ���ٽ��� api
	init_cycle_buffer();//��ʼ��������

	dat_fp = fopen("tmp.dat", "wb");//����Ƶ�ļ�
	if (NULL == dat_fp)
	{
		printf("open %s error.\n", "tmp.wav");
		return false;
	}
	std::thread t(forRec);//�����߳�
	RecordWave();//����¼����һ��¼������buffer�����ͻᴥ���ص�����
	t.detach();
	return true;
}