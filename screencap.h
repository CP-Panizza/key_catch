#ifndef SCREENCAP_H
#define SCREENCAP_H
#include <QByteArray>
#include <QLibrary>
#include <QSize>
#include <QTimer>
#include <windows.h>
#include <QThread>
#include "avilib.h"

class ScreenCap : public QThread
{
    Q_OBJECT
public:

    typedef HBITMAP (*CapFunc)();
    typedef void (*FreeFunc)(HBITMAP);

    ScreenCap(QWidget *parent);
    ~ScreenCap();
    void run();
    void stop();

    CapFunc p_func = nullptr;
    FreeFunc p_free_func = nullptr;
    QLibrary *cap_lib = nullptr;
    QByteArray m_buffer;
    bool m_stop = false;
    bool m_is_return = false;
    QSize m_size;
    QString strAppDir;
    avi_t *FD = nullptr;

    double m_fps = 7.03;
};

#endif // SCREENCAP_H
