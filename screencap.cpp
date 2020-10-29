#include "screencap.h"
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QtWin>
#include <QBuffer>
#include <ctime>
#include <QLibrary>
#include "avilib.h"
#include <thread>
#include "toojpeg.h"
#include <fstream>


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
    const bool isRGB      = true;  // true = RGB image, else false = grayscale
    const auto quality    = 20;    // compression quality: 0 = worst, 100 = best, 80 to 90 are most often used
    const bool downsample = true; // false = save as YCbCr444 JPEG (better quality), true = YCbCr420 (smaller file)
    size_t thread_pool_size = 2;
//    ThreadPool tp(thread_pool_size);
    std::vector<bool> flags(thread_pool_size);
    for(int i = 0; i < thread_pool_size; i++){
        flags[i] = false;
        std::thread thread_writer([this, &flags](int id){
            qDebug() << "thread:" << id << "run";
            while(true){
                this->m_mutex.lock();
                if(this->record_data.empty()){
                    this->m_mutex.unlock();
                    if(this->m_stop && this->record_data.empty()){
                        qDebug() << "thread:" << id << "done";
                        flags[id] = true;
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
                }, data.data, data.w , data.h, isRGB, quality, downsample);

                m_file.close();
                this->free_buffer(data.data);
            }
        }, std::forward<int>(i));
        thread_writer.detach();
    }

    time_t sub_time = 1000.0 / 10;
    //ffmpeg -f image2 -r 13.1579 -i %d.jpg -i ..\release\tmp.wav -vcodec libx264 -acodec copy  out.mkv
    time_t start_time = clock() - (sub_time + 100);
    long len = 0;
    int w = 0, h = 0;
    unsigned char * data = nullptr;
    while(!this->m_stop){
        if((clock() - start_time) < sub_time){
            continue;
        }
        start_time = clock();
//        start_exec = clock();
        data = this->p_func(&w, &h, &len);
        this->m_mutex.lock();
        this->record_data.push(std::move(Fream(data, totle_fream, w, h, len)));
        this->m_mutex.unlock();
        data = nullptr;
//        end_exec = clock();
//        totle_take_time += (end_exec - start_exec);
        totle_fream++;
    }
//    long avg = totle_take_time / totle_fream;
//    double fps = 1000.0 / avg;
//    this->m_fps = fps;
//    qDebug() << "fps:" << fps;

    std::thread check_thread_pool_is_done([this, &flags, thread_pool_size](){
        while(true){
            std::this_thread::sleep_for(std::chrono::seconds(2));
            bool done = true;
            for(int i = 0; i < thread_pool_size; i++){
                if(!flags[i]){
                    done = false;
                    qDebug() << "thread:" << i << "not done";
                    break;
                }
            }
            if(done){
                this->m_is_return = true;
                qDebug() << "all thread done";
                return;
            }
        }
    });
    check_thread_pool_is_done.join();
}

void ScreenCap::stop()
{
    this->m_stop = true;
}
