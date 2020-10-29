#include "screencap.h"
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QtWin>
#include <QBuffer>
#include <ctime>
#include <QLibrary>
#include <thread>
#include "toojpeg.h"
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include "utils.h"
#include <mutex>

using namespace rapidjson;

ScreenCap::ScreenCap(QWidget *parent): QThread(parent)
{
    this->strAppDir = QApplication::applicationDirPath() + "/tmp.avi";
    QRect screenRect = QApplication::desktop()->screenGeometry();
    this->m_size = screenRect.size();
    this->m_buffer.clear();
    this->cap_lib = new QLibrary("cap_lib.dll");
    if(!this->cap_lib->load()){
        qDebug() << "load fail";
        exit(-1);
    }


    this->p_init_func = (VOIDFUNC)this->cap_lib->resolve("init_cap_screen");
    if(this->p_init_func == nullptr){
        qDebug() << "load init_cap_screen err!";
        exit(-1);
    }

    this->p_func = (CapFunc)this->cap_lib->resolve("cap_screen");
    if(this->p_func == nullptr){
        qDebug() << "load cap_screen err!";
        exit(-1);
    }

    this->free_buffer = (FreeBuffer)this->cap_lib->resolve("free_buffer");
    if(this->free_buffer == nullptr){
        qDebug() << "load free_buffer err!";
        exit(-1);
    }


    QString strAppDir = QApplication::applicationDirPath();
    std::string data = read_file(strAppDir.toStdString() + std::string("/config.json"));
    if(!data.empty()){
        qDebug() << "config:" << QString::fromStdString(data);
        Document d;
        if(d.Parse(data.c_str()).HasParseError()){
            qDebug() << "parse config.json err!";
            return;
        }

        if(d.HasMember("video_quality")){
            this->quality = d["video_quality"].GetInt();
            if(this->quality > 100 || this->quality <= 0){
                this->thread_pool_size = 50;
            }
            qDebug() << "quality: " << this->quality;
        }

        if(d.HasMember("fps")){
            this->m_fps = d["fps"].GetDouble();
            if(this->m_fps > 10 || this->m_fps <= 0){
                this->thread_pool_size = 10;
            }
            qDebug() << "fps: " << this->m_fps;
        }

        if(d.HasMember("create_video_thread_num")){
            int num = d["create_video_thread_num"].GetInt();
            this->thread_pool_size = num;
            if(num > 5 || num <= 0){
                this->thread_pool_size = 2;
            }
            qDebug() << "threads: " << this->thread_pool_size;
        }
    }
}

ScreenCap::~ScreenCap()
{
    this->cap_lib->unload();
}


void clear_queue(std::queue<Fream>& q) {
    std::queue<Fream> empty;
    swap(empty, q);
}


void ScreenCap::run()
{
    this->handle_done = false;
    this->m_stop = false;
//    if(!this->record_data.empty()){
//        qDebug() << "clear_queue";
//        clear_queue(this->record_data);
//    }

    this->p_init_func();  //init_cap_screen
    long totle_fream = 0l;
    long totle_take_time = 0l;
    time_t start_exec{0};
    time_t end_exec{0};

    std::vector<std::thread> threads(this->thread_pool_size);
    for(int i = 0; i < this->thread_pool_size; i++){
        threads[i] = std::thread([this](int id){
            qDebug() << "thread:" << id << "run";
            while(true){
                this->m_mutex.lock();
                if(this->record_data.empty()){
                    this->m_mutex.unlock();
                    if(this->handle_done && this->record_data.empty()){
                        qDebug() << "thread:" << id << "done";
                        break;
                    }
                    continue;
                }
                auto data = this->record_data.front();
                this->record_data.pop();
                this->m_mutex.unlock();
                std::ofstream m_file("tmp/" + std::to_string(data.fream_idx) +".jpg", std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

                TooJpeg::writeJpeg([&m_file](unsigned char c){
                    m_file << c;
                }, data.data, data.w , data.h, this->isRGB, this->quality, this->downsample);

                m_file.close();
                this->free_buffer(data.data);
            }
        }, std::forward<int>(i));
    }

    time_t sub_time = 1000.0 / this->m_fps;
    //ffmpeg -f image2 -r 13.1579 -i %d.jpg -i ..\release\tmp.wav -vcodec libx264 -acodec copy  out.mkv
    time_t start_time = clock() - (sub_time + 100);
    long len = 0;
    int w = 0, h = 0;
    unsigned char * data = nullptr;
    std::queue<Fream> data_buffer;

    time_t move_data_buffer_to_record_data_take_time{0};
    while(!this->m_stop){
        auto sub = (clock() - start_time);
        if(sub < sub_time){
            if(sub < move_data_buffer_to_record_data_take_time){
                continue;
            }
            time_t begin = clock();
            if(!data_buffer.empty() && this->m_mutex.try_lock()){
                this->record_data.push(data_buffer.front());
                this->m_mutex.unlock();
                data_buffer.pop();
            }
            move_data_buffer_to_record_data_take_time = clock() - begin;
            continue;
        }
        start_time = clock();
        data = this->p_func(&w, &h, &len);
        if(this->m_mutex.try_lock()){
            this->record_data.push(std::move(Fream(data, totle_fream, w, h, len)));
            this->m_mutex.unlock();
        } else {
            data_buffer.push(std::move(Fream(data, totle_fream, w, h, len)));
        }
        totle_fream++;
    }

    while(!data_buffer.empty()){
        this->m_mutex.lock();
        this->record_data.push(data_buffer.front());
        this->m_mutex.unlock();
        data_buffer.pop();
    }

    this->handle_done = true;

    //wait all thread done
    for(auto &t : threads){
        t.join();
    }
    this->m_is_return = true;
    qDebug() << "all thread done";
}

void ScreenCap::stop()
{
    this->m_stop = true;
}
