#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPainter>
#include <QColor>
#include <QPen>
#include <QRect>
#include <QDesktopWidget>
#include "utils.h"
#include <QTimer>
#include <QFile>
#include <string>
#include <vector>
#include "matrix.hpp"
#include <QSystemTrayIcon>
#include <QIcon>
#include <QMenu>





HHOOK g_hHook = nullptr;
HHOOK g_hHook_mous = nullptr;
MainWindow *g_wd = nullptr;
time_t last_enter_time = 0;
DWORD last_key = 0;
bool long_tap = false;
bool shift_down = false;
LRESULT __stdcall CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0)
    {
        return CallNextHookEx(g_hHook, nCode, wParam, lParam);
    }
    else if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT k = *(KBDLLHOOKSTRUCT *)lParam;
        if(wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            last_key = 0;
            long_tap = false;
            if(GetAsyncKeyState(VK_SHIFT) & 0x8000){
                shift_down = true;
            } else {
                shift_down = false;
            }
        } else if(wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            time_t enter_time = clock();
            if(last_enter_time == 0){
                last_enter_time = enter_time;
            } else if(enter_time - last_enter_time < 500){
                g_wd->is_new_input = false;
            } else {
                g_wd->is_new_input = true;
            }
            last_enter_time = enter_time;

//            qDebug() << "vkCode:" << k.vkCode;

            if(last_key == k.vkCode){
//                qDebug() << "long_tap:" << k.vkCode;
                long_tap = true;
                g_wd->keylog.get_end()->show_time = clock();
            } else {
//                qDebug() << "new key:" << k.vkCode;
                long_tap = false;
            }

            if(GetAsyncKeyState(VK_SHIFT) & 0x8000){
                shift_down = true;
            } else {
                shift_down = false;
            }

            if(g_wd->is_new_input && !long_tap){
                if(shift_down && g_wd->img_map.count(k.vkCode + 300)){
                    g_wd->keylog.add(new Item(g_wd->img_map[k.vkCode + 300]->Copy(), clock()));
                    goto end;
                }

                if(g_wd->img_map.count(k.vkCode)){
                    g_wd->keylog.add(new Item(g_wd->img_map[k.vkCode]->Copy(), clock()));
                }
            } else if(!g_wd->is_new_input && !long_tap) {
                if(shift_down && g_wd->img_map.count(k.vkCode + 300)){
                    Matrix<int> *con = concat_matrix(g_wd->keylog.get_end()->img, g_wd->img_map[k.vkCode + 300]);
                    delete g_wd->keylog.get_end()->img;
                    g_wd->keylog.get_end()->img = con;
                    g_wd->keylog.get_end()->show_time = clock();
                    g_wd->keylog.update();
                    goto end;
                }
                if(g_wd->img_map.count(k.vkCode)){
                    Matrix<int> *con = concat_matrix(g_wd->keylog.get_end()->img, g_wd->img_map[k.vkCode]);
                    delete g_wd->keylog.get_end()->img;
                    g_wd->keylog.get_end()->img = con;
                    g_wd->keylog.get_end()->show_time = clock();
                    g_wd->keylog.update();
                }
            }
            end:
            last_key = k.vkCode;
        }
    }
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

LRESULT __stdcall CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    PMSLLHOOKSTRUCT hook_struct = (PMSLLHOOKSTRUCT)lParam;
//    qDebug() << "x:" << hook_struct->pt.x << " y:" << hook_struct->pt.y;
    if(g_wd != nullptr){
        g_wd->current_mouse_x = hook_struct->pt.x;
        g_wd->current_mouse_y = hook_struct->pt.y;
    }
    return CallNextHookEx(g_hHook_mous, nCode, wParam, lParam);
}



Matrix<int> * concat_matrix(Matrix<int> *left, Matrix<int> *right){
    int new_width = left->width + right->width;
    Matrix<int> *out = new Matrix<int>(left->height, new_width);
    for(int h = 0; h < left->height; h++){
        for(int w = 0; w < new_width; w++){
            int val = 0;
            if(w < left->width){
                val = left->Get(h, w);
            } else {
                val = right->Get(h, w - left->width);
            }
            out->Set(h, w, val);
        }
    }
    return out;
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{



    g_wd = this;

    this->load_data();
    g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyboardProc, GetModuleHandle(NULL), 0);
    if(g_hHook == nullptr){
        qDebug() << "SetWindowsHookEx KeyboardProc err";
    } else {
        qDebug() << "SetWindowsHookEx KeyboardProc success";
    }

    g_hHook_mous = SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)MouseProc, GetModuleHandle(NULL), 0);
    if(g_hHook_mous == nullptr){
        qDebug() << "SetWindowsHookEx MouseProc err";
    } else {
        qDebug() << "SetWindowsHookEx MouseProc success";
    }



//    QRect deskRect = QApplication::desktop()->availableGeometry();

    QRect screenRect = QApplication::desktop()->screenGeometry();

    this->setMouseTracking(true);
    ui->setupUi(this);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setWindowFlag(Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setFixedSize(screenRect.width(), screenRect.height());

    ::SetWindowLong((HWND)winId(), GWL_EXSTYLE, ::GetWindowLong((HWND)winId(), GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_LAYERED); //set window mouse penetrate
    QTimer *qtimer1 = new QTimer(this);
    QObject::connect(qtimer1, &QTimer::timeout,[this]{
        this->repaint();
    });
    qtimer1->start(10);


    this->system_tray = new QSystemTrayIcon(this);
    this->system_tray->setIcon(QIcon(":/icon.ico"));
    this->system_tray->setToolTip("key catch");

    this->menu = new QMenu(this);
    this->action = new QAction(this);
    this->action->setText("exit");
    this->menu->addAction(this->action);
    this->system_tray->setContextMenu(this->menu);
    connect(this->system_tray, &QSystemTrayIcon::activated, this, &MainWindow::active_tray);
    connect(this->action, &QAction::triggered, [this]{
        qDebug() << "exit";
        UnhookWindowsHookEx(g_hHook);
        UnhookWindowsHookEx(g_hHook_mous);
        this->app->quit();
    });
    this->system_tray->show();



    if(this->welcome_img != nullptr){
        this->keylog.add(new Item(this->welcome_img, clock() + 5000));
    }
}

void MainWindow::active_tray()
{
    this->menu->move(this->current_mouse_x - 40,this->current_mouse_y - 40);
    this->menu->show();
}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setApplication(QApplication *app)
{
    this->app = app;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    this->windowPos = this->pos();
    this->mousePos = event->globalPos();
    this->dPos = mousePos - windowPos;
}


void MainWindow::mouseMoveEvent(QMouseEvent *event){
    this->move(event->globalPos() - this->dPos);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    if(this->current_mouse_x != 0 && this->current_mouse_y != 0){
        QPainter painter(this);
        painter.setPen(QPen(Qt::transparent,0));
        painter.setBrush(QColor(255,0,0, 70));
        painter.drawEllipse(this->current_mouse_x - (CIRCLE_R / 2), this->current_mouse_y - (CIRCLE_R / 2), CIRCLE_R, CIRCLE_R);
    }

    if(!this->keylog.empty()){
        for(int i = 0; i <= this->keylog.end_index; i++){
            time_t now = clock();
            if(now - this->keylog.key_logs[i]->show_time <= SHOW_TIME){
                this->paint(this->keylog.key_logs[i]->img, 230, this->keylog.key_logs[i]->x, this->keylog.key_logs[i]->y);
            }
        }
    }
}

void MainWindow::load_data()
{
    QFile img_data(":/data.txt");
    bool isOK = img_data.open(QIODevice::ReadOnly);
    if(isOK){
        auto data = img_data.readAll();
        std::string str_data(data.data());
        std::vector<std::string> v_str = split(str_data, "\r\n");

        foreach(auto line, v_str){
            std::vector<std::string> line_data = split(line, " ");
            int height = 40;
            int width = (line_data.size() - 1) / height;
            Matrix<int> *mat = new Matrix<int>(height, width);
            for (int i = 1; i < height * width; i++) {
                mat->data[i - 1] = atoi(line_data[i].c_str());
            }
            this->img_map[atoi(line_data[0].c_str())] = mat;
        }
    } else {
        qDebug() << "load img err";
    }
    img_data.close();


    QFile img_welcome_data(":/welcome.txt");
    isOK = img_welcome_data.open(QIODevice::ReadOnly);
    if(isOK){
        auto data = img_welcome_data.readAll();
        std::string str_data(data.data());
        std::vector<std::string> v_str = split(str_data, " ");
        int height = 40;
        int width = v_str.size() / height;
        Matrix<int> *mat = new Matrix<int>(height, width);
        for (int i = 0; i < height * width; i++) {
            mat->data[i] = atoi(v_str[i].c_str());
        }

        this->welcome_img = mat;
    } else {
        qDebug() << "load welcome_img err";
    }
    img_welcome_data.close();
}


void MainWindow::paint(Matrix<int> *img, int alpha,  int x, int y)
{
    QPainter painter(this);
    int r = 10;
    double distance = 0.0;
    for (int i = 0; i < img->height; ++i)
    {
        for (int j = 0; j < img->width; ++j)
        {
            if(i <= r && j <= r){
                distance = ::pow((pow(i - r, 2)+pow(j - r,2)), 0.5);
                if(distance > r) continue;
            } else if(i <= r && j >= img->width - r){
                distance = ::pow((pow(i - r, 2)+pow(j - (img->width - r),2)), 0.5);
                if(distance > r) continue;
            } else if(i >= img->height - r && j <= r){
                distance = ::pow((pow(i - (img->height - r), 2)+pow(j - r,2)), 0.5);
                if(distance > r) continue;
            } else if(i >= img->height - r && j >= img->width - r){
                distance = ::pow((pow(i - (img->height - r), 2)+pow(j - (img->width - r),2)), 0.5);
                if(distance > r) continue;
            }
            int v = img->Get(i,j);
            painter.setPen(QColor(v,v,v, alpha));
            painter.drawPoint(j+x,i+y);
        }
    }


//    for (int i=0; i < img->height;i++) {
//        for (int j=0;j < img->width; j++) {
//            int v = img->Get(i,j);
//            painter.setPen(QColor(v,v,v, alpha));
//            painter.drawPoint(j+x,i+y);
//        }
//    }
}



