#ifndef SUBWINDOW_H
#define SUBWINDOW_H

#include <QWidget>
#include <QPoint>
#include <QPaintEvent>
#include <QPushButton>

#define HOVER_BORDER QString("QPushButton:hover{border: 1px solid black;}")

enum HIDEPOSATION //hide position
{
    HP_None = 0,
    HP_Top = 1,
    HP_Left = 2,
    HP_Right = 3
};

class SubWindow : public QWidget
{
    Q_OBJECT
public:
    SubWindow(QWidget *parent = nullptr);
    ~SubWindow();

    bool isWindowInScreen(QPoint pos);
    void hideWindow();
    void showWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

public:
    QPoint windowPos;
    QPoint mousePos;
    QPoint dPos;

    QPushButton *key_log_btn;

    QPushButton *cut_btn;
    QPushButton *pan_btn;

    QPushButton *nail_btn;

    bool cut_screen = false;
    bool key_log = true;

    int m_screenWidth;
    bool m_isLMousePress;
    QPoint m_relativePos;
    HIDEPOSATION m_hp = HP_None;
};

#endif // SUBWINDOW_H
