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
#include "ttipwidget.h"


int main(int argc, char *argv[])
{
    QApplication::setQuitOnLastWindowClosed(false);
    QApplication a(argc, argv);
    MainWindow w;
    w.setApplication(&a);

    SubWindow float_pan;

    w.float_pan_cb = [&](){  //set callback func
        float_pan.activateWindow();
    };

    auto open_key_log = [&](){
        if(!float_pan.key_log){
            w.key_log = true;
            float_pan.key_log = true;
            float_pan.key_log_btn->setStyleSheet("border-image:url(:/kb.png); width:30px; height: 30px;");
        }
    };
    QObject::connect(float_pan.key_log_btn, &QPushButton::clicked, [&](){
        if(w.stuta == MainWindow::ScreenStuta::PAINTING || w.stuta == MainWindow::ScreenStuta::DRAWING){
            return;
        }

        float_pan.key_log = !float_pan.key_log;
        w.key_log = !w.key_log;
        if(float_pan.key_log){
            TTipWidget::ShowMassage(&w, "key log on!");
            float_pan.key_log_btn->setStyleSheet("border-image:url(:/kb.png); width:30px; height: 30px;");
        } else {
            TTipWidget::ShowMassage(&w, "key log off!");
            w.keylog.clear();
            float_pan.key_log_btn->setStyleSheet("border-image:url(:/_kb.png); width:30px; height: 30px;");
        }
    });

    QObject::connect(float_pan.cut_btn, &QPushButton::clicked, [&](){
        if(w.stuta == MainWindow::ScreenStuta::PAINTING || w.stuta == MainWindow::ScreenStuta::DRAW_DONE_PAINTING){
            return;
        }

        open_key_log();
        TTipWidget::ShowMassage(&w,"hold mouse leftbutton choose, esc to quit!");
        QScreen *screen = QGuiApplication::primaryScreen();
        QPixmap screen_img = screen->grabWindow(0);
        w.hold_screen(screen_img);
        ::SetWindowLong((HWND)w.winId(), GWL_EXSTYLE, 256);  //cancel mouse penetrate
        if(w.stuta == MainWindow::NONE){
            w.stuta = MainWindow::ScreenStuta::PAINTING;
        } else if(w.stuta == MainWindow::ScreenStuta::DRAWING){
            w.stuta = MainWindow::ScreenStuta::DRAW_DONE_PAINTING;
        }
    });


    QObject::connect(float_pan.pan_btn, &QPushButton::clicked, [&](){
        TTipWidget::ShowMassage(&w,"hold mouse leftbutton draw, esc to quit!");
        open_key_log();
        w.init_canvas();
        ::SetWindowLong((HWND)w.winId(), GWL_EXSTYLE, 256);  //cancel mouse penetrate
        w.stuta = MainWindow::DRAWING;
    });


    w.show();
    float_pan.show();
    return a.exec();
}
