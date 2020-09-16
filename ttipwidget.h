#ifndef TTIPWIDGET_H
#define TTIPWIDGET_H
#include <QWidget>
#include <QLabel>

class TTipWidget :public QLabel
{
    Q_OBJECT

public:
    static TTipWidget &Instance();
    void SetMesseage(const QString &strMessage, const QPoint *pPoint = nullptr);
    static void ShowMassage(QWidget *parent, const QString &strMessage);
private:
    TTipWidget();
    ~TTipWidget();

    void OnTimer();
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

private:
    QWidget *mpParent;
    QTimer  *mpTimer;
    bool     mbEnter;
    int      mnTransparent;
};

#endif // TTIPWIDGET_H
