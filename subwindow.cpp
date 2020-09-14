#include "subwindow.h"
#include <QMouseEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QFont>
#include <QPainter>
#include <QBitmap>
#include <QPushButton>

SubWindow::SubWindow(QWidget *parent) : QWidget(parent)
{
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
    this->setFixedSize(150,100);
    QFrame *frame = new QFrame(this);
    frame->setStyleSheet("QFrame{background-color: rgb(255, 255, 255);border-radius:10px}"); //设置圆角与背景透明
    frame->setGeometry(5, 5, this->width() - 15, this->height() - 15);//设置有效范围框
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, 0);
    shadow_effect->setColor(Qt::black);
    shadow_effect->setBlurRadius(10);
    frame->setGraphicsEffect(shadow_effect);
    this->setAttribute(Qt::WA_TranslucentBackground);


    QLabel *title = new QLabel(this);
    title->setText("Key Catch");
    title->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    title->move(40, 15);
    title->show();



    this->key_log_btn = new QPushButton(this);
    this->key_log_btn->setStyleSheet("border-image:url(:/kb.png); width:30px; height: 30px;");
//    this->key_log_btn->setFlat(true);
    this->key_log_btn->move(20, 40);

    this->cut_btn = new QPushButton(this);
    this->cut_btn->setStyleSheet("border-image:url(:/cut.png); width:30px; height: 30px;");
//    this->cut_btn->setFlat(true);
    this->cut_btn->move(60, 40);

    auto shut_btn = new QPushButton(this);
    shut_btn->setObjectName("shut_btn");
    shut_btn->setStyleSheet("QPushButton#shut_btn{border-image:url(:/shut.png); width:25px; height: 25px;}");
//    shut_btn->setFlat(true);
    shut_btn->move(100, 43);

    connect(shut_btn, &QPushButton::clicked, [](){
        exit(0);
    });
}

SubWindow::~SubWindow()
{

}



void SubWindow::mousePressEvent(QMouseEvent *event)
{
    this->windowPos = this->pos();
    this->mousePos = event->globalPos();
    this->dPos = mousePos - windowPos;
}


void SubWindow::mouseMoveEvent(QMouseEvent *event){
    this->move(event->globalPos() - this->dPos);
}

void SubWindow::paintEvent(QPaintEvent *event)
{

}
