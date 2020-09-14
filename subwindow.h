#ifndef SUBWINDOW_H
#define SUBWINDOW_H

#include <QWidget>
#include <QPoint>
#include <QPaintEvent>
#include <QPushButton>


class SubWindow : public QWidget
{
    Q_OBJECT
public:
    SubWindow(QWidget *parent = nullptr);
    ~SubWindow();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

public:
    QPoint windowPos;
    QPoint mousePos;
    QPoint dPos;

    QPushButton *key_log_btn;

    QPushButton *cut_btn;

    bool cut_screen = false;
    bool key_log = true;
};

#endif // SUBWINDOW_H
