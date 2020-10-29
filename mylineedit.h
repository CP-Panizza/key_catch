#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QPoint>
class MyLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    MyLineEdit(QPoint pos, int _font_size, QString _text_color, QWidget *parent = Q_NULLPTR);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void focusOutEvent(QFocusEvent *) override;
    void focusInEvent(QFocusEvent *) override;

public slots:
    void onTextChanged(const QString &);
    void onReturnPressed();


public:
    int font_size;
    QString text_color;
    QPoint windowPos;
    QPoint mousePos;
    QPoint dPos;

    bool m_isLMousePress = false;
    QPoint m_relativePos;
};

#endif // MYLINEEDIT_H
