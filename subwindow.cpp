#include "subwindow.h"
#include <QMouseEvent>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QFont>
#include <QPainter>
#include <QBitmap>
#include <QPushButton>

#include <QPropertyAnimation>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QDebug>

SubWindow::SubWindow(QWidget *parent) : QWidget(parent)
{
    QRect rect = QApplication::desktop()->screenGeometry();
    m_screenWidth = rect.width();
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
    this->key_log_btn->setFlat(true);
    this->key_log_btn->move(20, 40);
    this->key_log_btn->setFocusPolicy(Qt::NoFocus);


    this->cut_btn = new QPushButton(this);
    this->cut_btn->setStyleSheet("border-image:url(:/cut.png); width:30px; height: 30px;");
    this->cut_btn->setFlat(true);
    this->cut_btn->move(60, 40);
    this->cut_btn->setFocusPolicy(Qt::NoFocus);


    auto shut_btn = new QPushButton(this);
    shut_btn->setObjectName("shut_btn");
    shut_btn->setStyleSheet("QPushButton#shut_btn{border-image:url(:/shut.png); width:25px; height: 25px;}");
    shut_btn->setFlat(true);
    shut_btn->move(100, 43);
    shut_btn->setFocusPolicy(Qt::NoFocus);
    shut_btn->setToolTip("exit");
    connect(shut_btn, &QPushButton::clicked, [](){
        exit(0);
    });
}

SubWindow::~SubWindow()
{

}

bool SubWindow::isWindowInScreen(QPoint pos)
{
    if(pos.x()<5){
        m_hp = HP_Left;
        return false;
    }
    else if(pos.x()>m_screenWidth-5){
        m_hp = HP_Right;
        return false;
    }
    else if(pos.y()<5){
        m_hp = HP_Top;
        return false;
    }
    else{
        m_hp = HP_None;
        return true;
    }
}

void SubWindow::hideWindow()
{
    QPropertyAnimation * animation = new QPropertyAnimation(this, "geometry");
    animation->setStartValue(QRect(x(),y(),width(),height()));
    if(m_hp == HP_None)
        return;
    else if(m_hp == HP_Top)
        animation->setEndValue(QRect(x(),25-height(),width(),height()));
    else if(m_hp == HP_Left)
        animation->setEndValue(QRect(25-width(),y(),width(),height()));
    else if(m_hp == HP_Right)
        animation->setEndValue(QRect(m_screenWidth-25,y(),width(),height()));

    animation->setDuration(250);
    animation->start();
}

void SubWindow::showWindow()
{
    QPropertyAnimation * animation = new QPropertyAnimation(this, "geometry");
    animation->setStartValue(QRect(x(),y(),width(),height()));
    if(m_hp == HP_None)
        return;
    else if(m_hp == HP_Top)
        animation->setEndValue(QRect(x(),0,width(),height()));
    else if(m_hp == HP_Left)
        animation->setEndValue(QRect(0,y(),width(),height()));
    else if(m_hp == HP_Right)
        animation->setEndValue(QRect(m_screenWidth-width(),y(),width(),height())); 
    animation->setDuration(250);
    animation->start();
}



void SubWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_isLMousePress = true;
        m_relativePos = event->globalPos() - pos();//记录相对位置
    }
}

void SubWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_isLMousePress = false;
}


void SubWindow::mouseMoveEvent(QMouseEvent *event){
    if(m_isLMousePress && isWindowInScreen(event->globalPos()))
        move(event->globalPos()-m_relativePos);//实现无边框移动
    else if(m_isLMousePress && !isWindowInScreen(event->globalPos()))
    {
        //特殊位置，移动规则不同
        int x = event->globalPos().x();
        int y = event->globalPos().y();
        if(m_hp == HP_Top)//比如当前鼠标位置为屏幕最上面时，将纵坐标拉至鼠标处，此后只改变横坐标
            move(x-m_relativePos.x(),y);
        else if(m_hp == HP_Left)
            move(x,y-m_relativePos.y());
        else if(m_hp == HP_Right)
            move(x-width(),y-m_relativePos.y());
    }
}

void SubWindow::paintEvent(QPaintEvent *event)
{

}

void SubWindow::enterEvent(QEvent *event)
{
    if(m_hp != HP_None)
        showWindow();
}

void SubWindow::leaveEvent(QEvent *event)
{
    if(m_hp != HP_None)
        hideWindow();
}
