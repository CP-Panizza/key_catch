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

#pragma comment(lib, "winmm.lib")//必须包含这个lib



struct WAVFILEHEADER
{
	// RIFF 头;
	char RiffName[4];
	unsigned long nRiffLength;
	// 数据类型标识符;
	char WavName[4];
	// 格式块中的块头;
	char FmtName[4];
	unsigned long nFmtLength;
	// 格式块中的块数据;
	unsigned short nAudioFormat;
	unsigned short nChannleNumber;
	unsigned long nSampleRate;
	unsigned long nBytesPerSecond;
	unsigned short nBytesPerSample;
	unsigned short nBitsPerSample;
	// 数据块中的块头;
	char    DATANAME[4];
	unsigned long   nDataLength;
};

int BUFFSIZE = 1024 * 1024; //环形缓冲区的大小，你可以定义大一些

int channelCount = 1;
int simpleSize = 16;
int simpleRate = 16000;

//#define min(x, y) ((x) < (y) ? (x) : (y))//这个函数在VS中有一个同样的宏，所以注释掉了。



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

//环形缓冲区的的数据结构
struct cycle_buffer {
	char *buf;
	unsigned int size;
	unsigned int in;
	unsigned int out;
};

static struct cycle_buffer *fifo = NULL;//定义全局FIFO
FILE *dat_fp = NULL;
CRITICAL_SECTION cs;

//初始化环形缓冲区
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

unsigned int fifo_get(char *buf, unsigned int len)  //从环形缓冲区中取数据
{
	unsigned int l;
	len = min(len, fifo->in - fifo->out);
	l = min(len, fifo->size - (fifo->out & (fifo->size - 1)));
	memcpy(buf, fifo->buf + (fifo->out & (fifo->size - 1)), l);
	memcpy(buf + l, fifo->buf, len - l);
	fifo->out += len;
	return len;
}

unsigned int fifo_put(char *buf, unsigned int len) //将数据放入环形缓冲区
{
	unsigned int l;
	len = min(len, fifo->size - fifo->in + fifo->out);
	l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));
	memcpy(fifo->buf + (fifo->in & (fifo->size - 1)), buf, l);
	memcpy(fifo->buf, buf + l, len - l);
	fifo->in += len;
	return len;
}

void WaveInitFormat(LPWAVEFORMATEX m_WaveFormat, WORD nCh, DWORD nSampleRate, WORD BitsPerSample)//初始化音频格式
{
	m_WaveFormat->wFormatTag = WAVE_FORMAT_PCM;
	m_WaveFormat->nChannels = nCh;
	m_WaveFormat->nSamplesPerSec = nSampleRate;
	m_WaveFormat->nAvgBytesPerSec = nSampleRate * nCh * BitsPerSample / 8;
	m_WaveFormat->nBlockAlign = m_WaveFormat->nChannels * BitsPerSample / 8;
	m_WaveFormat->wBitsPerSample = BitsPerSample;
	m_WaveFormat->cbSize = 0;
}

DWORD CALLBACK MicCallback(HWAVEIN hwavein, UINT uMsg, DWORD_PTR  dwInstance, DWORD_PTR  dwParam1, DWORD_PTR  dwParam2)//回调函数当数据缓冲区满的时候就会触发，回调函数，执行下面的RecordWave函数之后相当于创建了一个线程
{
	int len = 0;
	switch (uMsg)
	{
	case WIM_OPEN://打开设备时这个分支会执行。
		printf("\n设备已经打开...\n");
		break;
	case WIM_DATA://当缓冲区满的时候这个分支会执行，不要再这个分支中出现阻塞语句，会丢数据	，waveform audio本身没有缓冲机制。		
		printf("\n缓冲区%d存满...\n", ((LPWAVEHDR)dwParam1)->dwUser);
		waveInAddBuffer(hwavein, (LPWAVEHDR)dwParam1, sizeof(WAVEHDR)); // api

		EnterCriticalSection(&cs); //进入临界区  // api
		len = fifo_put(((LPWAVEHDR)dwParam1)->lpData, 10240); //将缓冲区的数据写入环形fifo
		LeaveCriticalSection(&cs);//退出临界区  // api
		break;
	case WIM_CLOSE:
		printf("\n设备已经关闭...\n");
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
	count = waveInGetNumDevs();//获取系统有多少个声卡 // api

	mmResult = waveInGetDevCaps(0, &waveIncaps, sizeof(WAVEINCAPS));//查看系统声卡设备参数，不用太在意这两个函数。

	printf("\ndevs count = %d\n", count);
	printf("\nwaveIncaps.szPname=%s\n", waveIncaps.szPname);

	if (MMSYSERR_NOERROR == mmResult)
	{
		WAVEFORMATEX pwfx;
		WaveInitFormat(&pwfx, channelCount, simpleRate, simpleSize);
		printf("\n请求打开音频输入设备");
		mmResult = waveInOpen(&phwi, WAVE_MAPPER, &pwfx, (DWORD_PTR)(MicCallback), NULL, CALLBACK_FUNCTION);//打开音频设备，设置回调函数

		if (MMSYSERR_NOERROR == mmResult)
		{

			char buffer1[10240];

			char buffer2[10240];

			pwh1.lpData = buffer1;
			pwh1.dwBufferLength = 10240;
			pwh1.dwUser = 1;
			pwh1.dwFlags = 0;
			mmResult = waveInPrepareHeader(phwi, &pwh1, sizeof(WAVEHDR));//准备缓冲区
			printf("\n准备缓冲区1");

			pwh2.lpData = buffer2;
			pwh2.dwBufferLength = 10240;
			pwh2.dwUser = 2;
			pwh2.dwFlags = 0;
			mmResult = waveInPrepareHeader(phwi, &pwh2, sizeof(WAVEHDR));//
			printf("\n准备缓冲区2\n");

			if (MMSYSERR_NOERROR == mmResult)
			{
				mmResult = waveInAddBuffer(phwi, &pwh1, sizeof(WAVEHDR));//添加缓冲区
				mmResult = waveInAddBuffer(phwi, &pwh2, sizeof(WAVEHDR));

				printf("\n将缓冲区2加入音频输入设备\n");
				if (MMSYSERR_NOERROR == mmResult)
				{
					mmResult = waveInStart(phwi);
					printf("\n开始录音\n");
				}
			}
		}
	}
}

bool g_stop = false;

//检测中断信号，保存录音并退出程序
void stop_record() {
	printf("enter stop\n");
	g_stop = true;
	Sleep(100);
	waveInStop(phwi);//停止录音
					 //waveInReset(phwi);
	waveInClose(phwi);//关闭音频设备
	waveInUnprepareHeader(phwi, &pwh1, sizeof(WAVEHDR));//释放buffer
	waveInUnprepareHeader(phwi, &pwh2, sizeof(WAVEHDR));
	printf("stop capture!\n");
	fflush(stdout);

	WAVFILEHEADER WavFileHeader;
	strcpy(WavFileHeader.RiffName, "RIFF");
	strcpy(WavFileHeader.WavName, "WAVE");
	strcpy(WavFileHeader.FmtName, "fmt ");

	WavFileHeader.nFmtLength = 16;  //  表示 FMT 的长度
	WavFileHeader.nAudioFormat = 1; //这个表示 PCM 编码;
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


void forRec()//新建的另一个线程用于将数据写入文件
{
	char buff[10240] = { 0 };
	int len = 0;
	while (!g_stop)
	{
		EnterCriticalSection(&cs); //进入临界区			
		len = fifo_get(buff, 10240);//从fifo中获取数据			
		LeaveCriticalSection(&cs);//离开临界区
		fwrite(buff, len, 1, dat_fp);//将音频数据写入音频文件				
		Sleep(100);
	}
}

bool start_record() {
	//signal(SIGINT, stopRecord);
	char buff[10240] = { 0 };
	InitializeCriticalSection(&cs);//初始化临界区 api
	init_cycle_buffer();//初始化缓冲区

	dat_fp = fopen("tmp.dat", "wb");//打开音频文件
	if (NULL == dat_fp)
	{
		printf("open %s error.\n", "tmp.wav");
		return false;
	}
	std::thread t(forRec);//创建线程
	RecordWave();//开启录音，一旦录音数据buffer满，就会触发回调函数
	t.detach();
	return true;
}