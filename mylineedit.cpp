#include "mylineedit.h"
#include <QFontMetrics>
#include <QDebug>
#include <QMouseEvent>

MyLineEdit::MyLineEdit(QPoint pos, int _font_size, QString _text_color, QWidget *parent) :QLineEdit(parent), font_size(_font_size), text_color(_text_color)
{
    this->move(pos);
    this->setFocus();
    this->setAlignment(Qt::AlignCenter);
    this->setFixedSize(6, _font_size);
    auto style = QString("QLineEdit{background-color:transparent; border: 2px dashed gray; font-size: %1px; border-style: dashed; color: %2;}").arg(QString::number(_font_size), this->text_color);
    qDebug() << style;
    this->setStyleSheet(style);
    QObject::connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(onTextChanged(const QString &)));
    QObject::connect(this, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
    this->show();
}

void MyLineEdit::onTextChanged(const QString &content)
{
    QFontMetrics fm1(font());
    this->setFixedSize((fm1.width(content)) + 15, this->font_size);
}

void MyLineEdit::onReturnPressed()
{
    this->clearFocus();
}


void MyLineEdit::mousePressEvent(QMouseEvent *event)
{
    QLineEdit::mousePressEvent(event);
    if(event->button() == Qt::LeftButton)
    {
        m_isLMousePress = true;
        m_relativePos = event->globalPos() - pos();
    } else if(event->button() == Qt::RightButton){
        this->close();
    }
}

void MyLineEdit::mouseReleaseEvent(QMouseEvent *event)
{
    m_isLMousePress = false;
}


void MyLineEdit::mouseMoveEvent(QMouseEvent *event){
    if(m_isLMousePress){
        this->setCursor(Qt::SizeAllCursor);
        this->move(event->globalPos()-m_relativePos);//实现无边框移动
    }
}

void MyLineEdit::focusOutEvent(QFocusEvent *event)
{
    m_isLMousePress = false;
    QLineEdit::focusOutEvent(event);
    this->setStyleSheet(QString("QLineEdit{background-color:transparent; font-size: %1px; border-style: outset; color: %2;}").arg(QString::number(this->font_size), this->text_color));
}

void MyLineEdit::focusInEvent(QFocusEvent *event)
{
    QLineEdit::focusInEvent(event);
    this->setStyleSheet(QString("QLineEdit{background-color:transparent; border: 2px dashed gray; font-size: %1px; border-style: dashed; color: %2;}").arg(QString::number(this->font_size), this->text_color));
}


