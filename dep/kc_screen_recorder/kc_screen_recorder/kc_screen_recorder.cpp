// kc_screen_recorder.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <fstream>
#include <csignal>
#include <iostream>
#include <string.h>
#include <vector>
#include <thread>
#include <queue>
#include <string>
#include <Windows.h>
#include "dirnet.h"
#include <sys/stat.h>
#include "spin_mutex.h"
#include "kc_timer.h"
#include "record_lib.h"
#include "kc_signal.h"
bool m_stop = false;

bool dir_exists(std::string path) {
	DIR *dir;
	if ((dir = opendir(path.c_str())) == NULL) {
		return false;
	}
	closedir(dir);
	return true;
}

std::string &replace_all(std::string &str, const std::string &old_value, const std::string &new_value) {
	while (true) {
		std::string::size_type pos(0);
		if ((pos = str.find(old_value)) != std::string::npos) {
			str.replace(pos, old_value.length(), new_value);
		}
		else { break; }
	}
	return str;
}

LPCWSTR stringToLPCWSTR(std::string orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t *wcstring = (wchar_t *)malloc(sizeof(wchar_t)*(orig.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);
	return wcstring; 
}

void CreateFileDir(std::string sPath)
{
	replace_all(sPath, "/", "\\");
	int nPos = sPath.find('\\');
	while (nPos != -1)
	{
		CreateDirectory(stringToLPCWSTR(std::string(sPath.begin(), sPath.begin() + nPos)), NULL);
		nPos = sPath.find('\\', nPos + 1);
	}
}


void sigint_handler(int sig) {
	// ctrl+c退出时执行的代码
	std::cout << "ctrl+c pressed!" << std::endl;
	m_stop = true;
}

struct Fream {
	Fream() :data(nullptr) {}
	long fream_idx;
	unsigned char * data;
	int w, h;
	long len;
	Fream(unsigned char *_data, long _fream_idx, int _w, int _h, long _len) :
		data(_data), fream_idx(_fream_idx), w(_w), h(_h), len(_len)
	{};
};

void set_signal_handle() {
	signal(SIGINT, sigint_handler);
	signal(SIGTERM, sigint_handler);
	signal(SIGABRT, sigint_handler);
}

int main(int argc, char **argv) {
	::SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	kc_signal process_signal(std::string("kc_event"), kc_signal::RECEIVER);
	//process_signal.active();
	set_signal_handle();
	int quality = 80;    // compression quality: 0 = worst, 100 = best, 80 to 90 are most often used
	int fps = 5;
	int thread_pool_size = 2;
	char *outPath = "tmp/";
	
	for (int argi = 1; argi < argc && *argv[argi] == '-'; argi++)
	{
		if (!strcmp(argv[argi], "-t") && argi + 1 < argc)
		{
			thread_pool_size = atoi(argv[++argi]);
		}
		else if (!strcmp(argv[argi], "-q") && argi + 1 < argc)
		{
			quality = atoi(argv[++argi]);
		}
		else if (!strcmp(argv[argi], "-o") && argi + 1 < argc)
		{
			outPath = argv[++argi];
		}
		else if (!strcmp(argv[argi], "-f") && argi + 1 < argc)
		{
			fps = atoi(argv[++argi]);
			if (fps == 0)
			{
				fprintf(stderr, "Error: Invalid FPS value `%s'.\n", argv[argi]);
				return 255;
			}
		}
		else
		{
			return 0;
		}
	}

	if (!dir_exists(outPath)) {
		CreateFileDir(outPath);
	}
	init_cap_screen();
	long totle_fream = 0l;
	long totle_take_time = 0l;
	int64_t start_exec{ 0 };
	int64_t end_exec{ 0 };

	const bool isRGB = true;  // true = RGB image, else false = grayscale
	
	const bool downsample = true; // false = save as YCbCr444 JPEG (better quality), true = YCbCr420 (smaller file)
	
	

	bool handle_done = false;
	std::queue<Fream> record_data;
	//    std::mutex m_mutex;
	spin_mutex m_mutex{ 1 }; //try xxx time to lock

	std::vector<std::thread> threads(thread_pool_size);
	for (int i = 0; i < thread_pool_size; i++) {
		threads[i] = std::thread([&](int id) {
			std::cout << "thread:" << id << "run";
			while (true) {
				m_mutex.lock();
				if (record_data.empty()) {
					m_mutex.unlock();
					if (handle_done && record_data.empty()) {
						std::cout << "thread:" << id << "done";
						break;
					}
					continue;
				}
				auto data = record_data.front();
				record_data.pop();
				m_mutex.unlock();
				std::ofstream m_file(std::string(outPath) + std::to_string(data.fream_idx) + ".jpg", std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

				TooJpeg::writeJpeg([&m_file](unsigned char c) {
					m_file << c;
				}, data.data, data.w, data.h, isRGB, quality, downsample);

				m_file.close();
				free_buffer(data.data);
				//std::cout << "thread " << id << " create done\n";
			}
		}, std::forward<int>(i));
		::SetThreadPriority((HANDLE)threads[i].native_handle(), THREAD_PRIORITY_TIME_CRITICAL);
	}

	int64_t sub_time = (1000000000000 / fps); //单位 皮秒
	int64_t next_sub_time = sub_time;
	int64_t start_time = kc_timer::current_time<kc_timer::picoseconds>() - (sub_time + 1);
	long len = 0;
	int w = 0, h = 0;
	unsigned char * data = nullptr;
	std::queue<Fream> data_buffer;
	int64_t fragment_time = 0;  //每一帧截屏代码运行的时间碎片，当时间碎片大于或等于两帧图片之间的时间，就清零，且调整一帧（丢弃）

	int64_t move_data_buffer_to_record_data_take_time{ 0 };
	bool is_move_data_buffer_to_record_data_take_time = false;


	std::thread loop([&]() {
		while (!m_stop) {
			auto sub = (kc_timer::current_time<kc_timer::picoseconds>() - start_time);
			if (sub < sub_time) {
				if ((sub + (move_data_buffer_to_record_data_take_time * 3)) > sub_time
					&& is_move_data_buffer_to_record_data_take_time) {
					continue;
				}
				int64_t begin = kc_timer::current_time<kc_timer::picoseconds>();
				if (!data_buffer.empty() && m_mutex.try_lock()) {
					record_data.push(data_buffer.front());
					m_mutex.unlock();
					data_buffer.pop();
					is_move_data_buffer_to_record_data_take_time = true;
				}
				else {
					is_move_data_buffer_to_record_data_take_time = false;
				}
				move_data_buffer_to_record_data_take_time = kc_timer::current_time<kc_timer::picoseconds>() - begin;
				continue;
			}


			if (fragment_time >= (sub_time * 1)) {
				start_time = kc_timer::current_time<kc_timer::picoseconds>();
				fragment_time = 0;
				continue;
			}
			int64_t tmp_time = (sub - sub_time);
			if (tmp_time > 0) {
				fragment_time += tmp_time;
			}
			start_time = kc_timer::current_time<kc_timer::picoseconds>();
			data = cap_screen(&w, &h, &len);
			if (m_mutex.try_lock()) {
				record_data.push(Fream(data, totle_fream, w, h, len));
				//std::cout << "record_data.push:" << totle_fream << "\n";
				m_mutex.unlock();
			}
			else {
				data_buffer.push(Fream(data, totle_fream, w, h, len));
				//std::cout << "data_buffer.push:" << totle_fream << "\n";
			}
			totle_fream++;
		}
	});
	::SetThreadPriority((HANDLE)loop.native_handle(), THREAD_PRIORITY_TIME_CRITICAL);
	
	loop.detach();
	
	process_signal.wait();
	m_stop = true;
	process_signal.un_active();
	while (!data_buffer.empty()) {
		m_mutex.lock();
		record_data.push(data_buffer.front());
		m_mutex.unlock();
		data_buffer.pop();
	}
	handle_done = true;
	//wait all thread done
	for (auto &t : threads) {
		t.join();
	}
	std::cout << "all done!\n";
	process_signal.active();
    return 0;
}

