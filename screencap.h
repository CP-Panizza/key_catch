#ifndef SCREENCAP_H
#define SCREENCAP_H
#include <QByteArray>
#include <QLibrary>
#include <QSize>
#include <QTimer>
#include <windows.h>
#include <QThread>
#include "avilib.h"
#include <queue>
#include "spin_mutex.h"

struct Fream{
    Fream():data(nullptr){}
    long fream_idx;
    unsigned char * data;
    int w, h;
    long len;
    Fream(unsigned char *_data, long _fream_idx, int _w, int _h, long _len):
        data(_data), fream_idx(_fream_idx),w(_w),h(_h),len(_len)
    {};
};



class ScreenCap : public QThread
{
    Q_OBJECT
public:

    typedef void (*VOIDFUNC)();
    typedef unsigned char * (*CapFunc)(int *width, int *height, long *len);
    typedef void (*FreeBuffer)(unsigned char * data);


    ScreenCap(QWidget *parent);
    ~ScreenCap();
    void run();
    void stop();

    VOIDFUNC p_init_func = nullptr;
    CapFunc p_func = nullptr;
    FreeBuffer free_buffer = nullptr;
    QLibrary *cap_lib = nullptr;
    QByteArray m_buffer;
    bool m_stop = false;
    bool m_is_return = false;
    QSize m_size;
    QString strAppDir;
    avi_t *FD = nullptr;

    double m_fps = 10;

    std::queue<Fream> record_data;
    spin_mutex m_mutex;
};


void clear_queue(std::queue<HBITMAP>& );

#endif // SCREENCAP_H
