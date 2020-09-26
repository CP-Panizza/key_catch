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

    this->p_func = (CapFunc)this->cap_lib->resolve("cap_screen");
    if(this->p_func == nullptr){
        qDebug() << "load func err!";
        exit(-1);
    }


    this->p_free_func = (FreeFunc)this->cap_lib->resolve("free_img");
    if(this->p_free_func == nullptr){
        qDebug() << "load func err!";
        exit(-1);
    }
}

ScreenCap::~ScreenCap()
{
    this->cap_lib->unload();
}




void ScreenCap::run()
{
    long totle_fream = 0l;
    long totle_take_time = 0l;
    this->FD = AVI_open_output_file(this->strAppDir.toUtf8().data());
    AVI_set_video(this->FD, this->m_size.width(), this->m_size.height(), this->m_fps, QByteArray("MJPG").data());


    time_t start_exec{0};
    time_t end_exec{0};
    while(!this->m_stop){
        start_exec = clock();
        HBITMAP bitmap = this->p_func();
        QPixmap screen_img = QtWin::fromHBITMAP(bitmap);
        this->m_buffer.clear();
        QBuffer buffer(&(this->m_buffer));
        bool bOk = screen_img.save(&buffer, "jpg");

        if(!m_buffer.isEmpty() && bOk)
        {
            int iRet = AVI_write_frame(this->FD, this->m_buffer.data(), this->m_buffer.length(), 1);
//            qDebug() << iRet;
            this->m_buffer.clear();
        }
        this->p_free_func(bitmap);
        end_exec = clock();
        totle_take_time += (end_exec - start_exec);
        totle_fream++;
    }
    long avg = totle_take_time / totle_fream;
    double fps = 1000.0 / avg;
    AVI_set_video(this->FD, this->m_size.width(), this->m_size.height(), fps , QByteArray("MJPG").data());
    AVI_close(this->FD);
    this->FD = nullptr;
}

void ScreenCap::stop()
{
    this->m_stop = true;
}
