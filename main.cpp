#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include "subwindow.h"
#include <QPushButton>
#include <QObject>
#include <QDebug>
#include <QGuiApplication>
#include <QDateTime>
#include <QScreen>
#include <QPixmap>
#include <QDir>
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>


int main(int argc, char *argv[])
{
    QApplication::setQuitOnLastWindowClosed(false);
    QApplication a(argc, argv);
    MainWindow w;
    w.setApplication(&a);

    SubWindow float_pan;


    QObject::connect(float_pan.key_log_btn, &QPushButton::clicked, [&](){
        if(w.stuta == MainWindow::CutScreenStuta::PAINTING){
            return;
        }
        float_pan.key_log = !float_pan.key_log;
        w.key_log = !w.key_log;
        if(float_pan.key_log){
            float_pan.key_log_btn->setStyleSheet("border-image:url(:/kb.png); width:30px; height: 30px;");
        } else {
            w.keylog.clear();
            float_pan.key_log_btn->setStyleSheet("border-image:url(:/_kb.png); width:30px; height: 30px;");
        }
    });

    QObject::connect(float_pan.cut_btn, &QPushButton::clicked, [&](){
        if(w.stuta == MainWindow::CutScreenStuta::PAINTING){
            return;
        }
        if(!float_pan.key_log){
            qDebug() << "key log is shut";
            w.key_log = true;
            float_pan.key_log = true;
            float_pan.key_log_btn->setStyleSheet("border-image:url(:/kb.png); width:30px; height: 30px;");
        }
        QScreen *screen = QGuiApplication::primaryScreen();
        QPixmap screen_img = screen->grabWindow(0);
        w.hold_screen(screen_img);
        ::SetWindowLong((HWND)w.winId(), GWL_EXSTYLE, 256);  //cancel mouse penetrate
        w.stuta = MainWindow::CutScreenStuta::PAINTING;
    });


    w.show();
    float_pan.show();
    return a.exec();
}
