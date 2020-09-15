#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QPoint>
#include <QPixmap>
#include <windows.h>
#include "matrix.hpp"
#include <QMap>
#include <vector>
#include "keylog.h"
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QApplication>


#define SHOW_TIME 5000
#define CIRCLE_R 60


LRESULT __stdcall CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
Matrix<int> * concat_matrix(Matrix<int> *left, Matrix<int> *right);
extern HHOOK g_hHook;
extern HHOOK g_hHook_mous;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setApplication(QApplication *app);

    void hold_screen(QPixmap &screen_img);


    enum CutScreenStuta{
        NONE,
        PAINTING,
        CHOOSING_RECT,
        CHOOSE_DONE
    };

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void closeEvent(QCloseEvent *event);


    void load_data();
    void paint(Matrix<int> *img,int alpha, int x, int y);
public slots:
    void active_tray();

private:

    QPoint windowPos;
    QPoint mousePos;
    QPoint dPos;


    Matrix<int> *welcome_img = nullptr;

public:

//    Inputer *inputer_alphabet = nullptr;
//    Inputer *inputer_control = nullptr;
//    Inputer *inputer_symbol = nullptr;
//    std::vector<int> keys;  //save down keys

//    std::vector<int> symbol{VK_OEM_1,VK_OEM_PLUS,VK_OEM_COMMA,VK_OEM_MINUS,VK_OEM_PERIOD,VK_OEM_2,VK_OEM_3,VK_OEM_4,VK_OEM_5,VK_OEM_6,VK_OEM_7,VK_OEM_8,'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
//    std::vector<int> alphabet{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
//    std::vector<int> control{VK_ESCAPE,VK_SHIFT, VK_CONTROL, VK_MENU, VK_SPACE, VK_TAB, VK_BACK, VK_RETURN,VK_LWIN,VK_INSERT,VK_DELETE,VK_HOME,VK_END, VK_LEFT, VK_UP, VK_DOWN, VK_RIGHT, VK_F1, VK_F2, VK_F3, VK_F4,VK_F5,VK_F6,VK_F7,VK_F8, VK_F9,VK_F10,VK_F11,VK_F12};

    Ui::MainWindow *ui;

    QMap<int, Matrix<int> *> img_map;   //key_img_data
    Keylog keylog;

    bool is_new_input = true;   //juge new input

    QApplication *app;

    QSystemTrayIcon *system_tray;
    QMenu *menu;
    QAction *action;


    int current_mouse_x = 0;
    int current_mouse_y = 0;


    bool cut_screen = false;
    bool key_log = true;


    QPixmap *screen_img = nullptr;

    CutScreenStuta stuta = CutScreenStuta::NONE;

    int cut_img_start_x = 0;
    int cut_img_start_y = 0;
    int cut_img_end_x = 0;
    int cut_img_end_y = 0;

};

extern MainWindow *g_wd;
#endif // MAINWINDOW_H
