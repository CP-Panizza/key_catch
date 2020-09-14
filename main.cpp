#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include "subwindow.h"
#include <QPushButton>
#include <QObject>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setApplication(&a);

    SubWindow float_pan;


    QObject::connect(float_pan.key_log_btn, &QPushButton::clicked, [&](){
        float_pan.key_log = !float_pan.key_log;
        w.key_log = !w.key_log;
        if(float_pan.key_log){
            float_pan.key_log_btn->setStyleSheet("border-image:url(:/kb.png); width:30px; height: 30px;");
        } else {
            float_pan.key_log_btn->setStyleSheet("border-image:url(:/_kb.png); width:30px; height: 30px;");
        }
    });

    QObject::connect(float_pan.cut_btn, &QPushButton::clicked, [&](){

    });
    w.show();
    float_pan.show();
    return a.exec();
}
