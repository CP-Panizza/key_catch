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
#include "kc_thread.h"
#include <mutex>

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



class ScreenCap : public kc_thread_bass
{
//    Q_OBJECT
public:

    typedef void (*VOIDFUNC)();
    typedef unsigned char * (*CapFunc)(int *width, int *height, long *len);
    typedef void (*FreeBuffer)(unsigned char * data);


//    ScreenCap(QWidget *parent = nullptr);
    ScreenCap();
    ~ScreenCap();
    void run();
    void stop();

    VOIDFUNC p_init_func = nullptr;
    CapFunc p_func = nullptr;
    FreeBuffer free_buffer = nullptr;
    QLibrary *cap_lib = nullptr;
    QByteArray m_buffer;
    bool m_stop = false;  //stop to capture
    bool m_is_return = false;
    QSize m_size;
    QString strAppDir;
    avi_t *FD = nullptr;

    const bool isRGB      = true;  // true = RGB image, else false = grayscale
    int quality             = 80;    // compression quality: 0 = worst, 100 = best, 80 to 90 are most often used
    const bool downsample = true; // false = save as YCbCr444 JPEG (better quality), true = YCbCr420 (smaller file)
    size_t thread_pool_size = 1;
    double m_fps = 6;

    bool handle_done = false;
    std::queue<Fream> record_data;
//    std::mutex m_mutex;
    spin_mutex m_mutex{200}; //try xxx time to lock
};


#endif // SCREENCAP_H
