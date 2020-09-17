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
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <cmath>
#include <QGuiApplication>
#include <QScreen>
#include <QPainterPath>
#include "ttipwidget.h"
#include <chrono>


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



        if(!g_wd->key_log){
            return CallNextHookEx(g_hHook, nCode, wParam, lParam);
        }
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
            if((g_wd->stuta == MainWindow::PAINTING || g_wd->stuta == MainWindow::DRAWING || g_wd->stuta == MainWindow::DRAW_DONE_PAINTING) && k.vkCode == 27){
                delete g_wd->screen_img;
                g_wd->screen_img = nullptr;
                g_wd->cut_img_start_x = 0;
                g_wd->cut_img_start_y = 0;
                g_wd->cut_img_end_x = 0;
                g_wd->cut_img_end_y = 0;
                if(g_wd->stuta == MainWindow::PAINTING || g_wd->stuta == MainWindow::DRAWING){
                      g_wd->stuta = MainWindow::NONE;
                      g_wd->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
                      ::SetWindowLong((HWND)g_wd->winId(), GWL_EXSTYLE, ::GetWindowLong((HWND)g_wd->winId(), GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_LAYERED);
                } else if(g_wd->stuta == MainWindow::DRAW_DONE_PAINTING){
                      g_wd->stuta = MainWindow::DRAWING;
                }
            }
//            qDebug() << "vkCode:" << k.vkCode;
            time_t enter_time = clock();
            if(last_enter_time == 0){
                last_enter_time = enter_time;
            } else if(enter_time - last_enter_time < 500){
                g_wd->is_new_input = false;
            } else {
                g_wd->is_new_input = true;
            }
            last_enter_time = enter_time;



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

    if(wParam == WM_MOUSEWHEEL && g_wd->stuta == MainWindow::DRAWING){
//        qDebug() << "wParam:" << wParam;
        short x = HIWORD(hook_struct->mouseData);
//        qDebug() << "x:" << x;
//        qDebug() << hook_struct->flags << " " << hook_struct->mouseData << " " << hook_struct->dwExtraInfo;
        if(x > 0){
            g_wd->draw_color_index = ++g_wd->draw_color_index % g_wd->draw_pan_colors.size();
            TTipWidget::ShowMassage(g_wd, g_wd->draw_pan_colors_name[g_wd->draw_color_index]);
        } else {
            g_wd->draw_color_index = --g_wd->draw_color_index % g_wd->draw_pan_colors.size();
            TTipWidget::ShowMassage(g_wd, g_wd->draw_pan_colors_name[g_wd->draw_color_index]);
        }
    }

    if(wParam == WM_LBUTTONDOWN && g_wd->stuta == MainWindow::DRIVING_NAIL){
        POINT point;
        if(GetCursorPos(&point)){
            HWND hwnd = WindowFromPoint(point);
            if(hwnd == NULL || hwnd == INVALID_HANDLE_VALUE){

            } else {
                if(have(g_wd->self_hwnd, hwnd)){
                     TTipWidget::ShowMassage(g_wd, "can not set self pos!");
                    goto end;
                }


                if(have(g_wd->top_most_hwnd, hwnd)){
                    TTipWidget::ShowMassage(g_wd, "cancel top successed!");
                    g_wd->m_mutex.lock();
                    std::vector<HWND>::iterator temp_it;
                    for(auto it = g_wd->top_most_hwnd.begin(); it != g_wd->top_most_hwnd.end(); it++){
                        if(*it == hwnd){
                            temp_it = it;
                            break;
                        }
                    }
                    g_wd->top_most_hwnd.erase(temp_it);
                    ::SetWindowPos(hwnd, HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
                    g_wd->m_mutex.unlock();
                    goto end;
                } else if(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST){
                    qDebug() << "already top:" << hwnd;
                     TTipWidget::ShowMassage(g_wd, "cancel top successed!");
                    ::SetWindowPos(hwnd, HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
                    goto end;
                }
                qDebug() << "set top:" << hwnd;
                TTipWidget::ShowMassage(g_wd, "set top successed!");
                g_wd->m_mutex.lock();
                if(!have(g_wd->top_most_hwnd, hwnd)){
                    g_wd->top_most_hwnd.push_back(hwnd);
                }
                g_wd->m_mutex.unlock();
            }
        }
    } else if(wParam == WM_LBUTTONUP && g_wd->stuta == MainWindow::DRIVING_NAIL){
        g_wd->stuta = MainWindow::NONE;
    }
    end:
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

    sht_hwnd_top_thread = new std::thread([this](){
        while(true){
            std::this_thread::sleep_for(std::chrono::seconds(1));
            this->m_mutex.lock();
            for(auto &hwnd : this->top_most_hwnd){
                if(this->stuta == MainWindow::NONE){
                    ::SetWindowPos(hwnd, HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
                } else {
                    ::SetWindowPos(hwnd, HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
                }
            }
            this->m_mutex.unlock();
        }
    });
    sht_hwnd_top_thread->detach();
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
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
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

void MainWindow::hold_screen(QPixmap &screen_img)
{
    if(this->screen_img != nullptr){
        delete(this->screen_img);
    } else {
        this->screen_img = new QPixmap(screen_img);
    }
}

void MainWindow::init_canvas()
{
    if(this->canvas != nullptr){
        delete this->canvas;
        this->canvas = nullptr;
    }


    QScreen *screen = QGuiApplication::primaryScreen();
    QPixmap screen_img = screen->grabWindow(0);
    this->canvas = new QImage(screen_img.toImage());

}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(this->stuta == DRAWING){
        if(event->button() == Qt::LeftButton)
        {
            lastPoint = event->pos();
        }
        return;
    }

    this->cut_img_start_x = event->globalPos().x();
    this->cut_img_start_y = event->globalPos().y();

    if(this->stuta == MainWindow::PAINTING){
        this->stuta = MainWindow::CHOOSING_RECT;
    } else if(this->stuta == MainWindow::DRAW_DONE_PAINTING){
        this->stuta = MainWindow::DRAW_DONE_CHOOSING_RECT;
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(this->stuta == MainWindow::CHOOSING_RECT || this->stuta == MainWindow::DRAW_DONE_CHOOSING_RECT){
        qDebug() << "choose done";
        this->cut_img_end_x = this->current_mouse_x;
        this->cut_img_end_y = this->current_mouse_y;
        if(this->stuta == MainWindow::CHOOSING_RECT){
            this->stuta = MainWindow::ScreenStuta::NONE;
        } else if(this->stuta == MainWindow::DRAW_DONE_CHOOSING_RECT){
            this->stuta = MainWindow::DRAWING;
        }

        QString fileName = QFileDialog::getSaveFileName(nullptr,
                ("save file"),
                "",
                ("save (*.png)"));

        if (!fileName.isNull())
        {
            int width = ::abs(this->cut_img_end_x - this->cut_img_start_x);
            int height = ::abs(this->cut_img_end_y - this->cut_img_start_y);
            int start_x = this->cut_img_start_x < this->cut_img_end_x ? this->cut_img_start_x : this->cut_img_end_x;
            int start_y = this->cut_img_start_y < this->cut_img_end_y ? this->cut_img_start_y : this->cut_img_end_y;
            QRect rect(start_x, start_y, width, height);
            QPixmap cropped = this->screen_img->copy(rect);
            if(this->stuta == MainWindow::NONE){
                this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
                ::SetWindowLong((HWND)this->winId(), GWL_EXSTYLE, ::GetWindowLong((HWND)this->winId(), GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_LAYERED);
            }

            if(!cropped.save(fileName, "png"))
            {
                TTipWidget::ShowMassage(this, "save image error!");
            } else {
                TTipWidget::ShowMassage(this,"save image successed!");
            }
            if(this->screen_img != nullptr){
                delete this->screen_img;
                this->screen_img = nullptr;
            }
            this->cut_img_start_x = 0;
            this->cut_img_start_y = 0;
            this->cut_img_end_x = 0;
            this->cut_img_end_y = 0;


        } else {  //cannel save cut image
            if(this->stuta == MainWindow::NONE){
                this->stuta = MainWindow::PAINTING;
            } else if(this->stuta == MainWindow::DRAWING){
                this->stuta = MainWindow::DRAW_DONE_PAINTING;
            }
            this->cut_img_start_x = 0;
            this->cut_img_start_y = 0;
            this->cut_img_end_x = 0;
            this->cut_img_end_y = 0;
            TTipWidget::ShowMassage(this,"cancel save cap image, esc to quit!");
        }
    }

    if(this->stuta == MainWindow::DRAWING){  //when a drawing done,mouse release,active float_pan
        this->float_pan_cb();
    }
}


void MainWindow::mouseMoveEvent(QMouseEvent *event){

    if(this->stuta == DRAWING){
        if(event->buttons()&Qt::LeftButton){
            QPainter painter(this->canvas);
            painter.setPen(QPen(this->draw_pan_colors[draw_color_index], 5));
            endPoint = event->pos();
            painter.drawLine(lastPoint,endPoint);
            lastPoint = endPoint;
        }
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{

    if(this->current_mouse_x != 0 && this->current_mouse_y != 0){
        QPainter painter(this);
        painter.setPen(QPen(Qt::transparent,0));
        painter.setBrush(QColor(255,0,0, 70));
        painter.drawEllipse(this->current_mouse_x - (CIRCLE_R / 2), this->current_mouse_y - (CIRCLE_R / 2), CIRCLE_R, CIRCLE_R);
    }

    if(this->key_log){
        if(!this->keylog.empty()){
            for(int i = 0; i <= this->keylog.end_index; i++){
                time_t now = clock();
                if(now - this->keylog.key_logs[i]->show_time <= SHOW_TIME){
                    this->paint(this->keylog.key_logs[i]->img, 230, this->keylog.key_logs[i]->x, this->keylog.key_logs[i]->y);
                }
            }
        }
    }

    if(this->screen_img != nullptr && (this->stuta == MainWindow::PAINTING || this->stuta == MainWindow::DRAW_DONE_PAINTING)){
        QPixmap img(":/cut.png");
        QPixmap r = img.scaled(20, 20);
        this->setCursor(QCursor(r, -1 , -1));
        QPainter painter(this);
        painter.drawPixmap(0, 0, width(), height(), *this->screen_img);
    }



    if((this->stuta == DRAWING || this->stuta == DRAW_DONE_CHOOSING_RECT || this->stuta == DRAW_DONE_PAINTING) && this->canvas != nullptr){
        if(this->stuta == DRAWING){
            QPixmap img(":/pan.png");
            QPixmap r = img.scaled(20, 20);
            this->setCursor(QCursor(r, 0 , 15));
        }

        QPainter painter(this);
        painter.drawImage(0,0, *this->canvas);
    }

    if(this->stuta == CHOOSING_RECT || this->stuta == MainWindow::DRAW_DONE_CHOOSING_RECT){
        qDebug() << "chooseing";
        QPainter painter(this);
        painter.setBrush(Qt::red);
        painter.setPen(Qt::red);

        QPoint left_up(this->cut_img_start_x, this->cut_img_start_y);
        QPoint left_down(this->cut_img_start_x, this->current_mouse_y);
        QPoint right_up(this->current_mouse_x, this->cut_img_start_y);
        QPoint right_down(this->current_mouse_x, this->current_mouse_y);

        painter.drawLine(left_up, left_down);
        painter.drawLine(left_up, right_up);
        painter.drawLine(left_down, right_down);
        painter.drawLine(right_up, right_down);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    exit(0);
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
    bool is_in_area = false;
    for (int i = 0; i < img->height; ++i)
    {
        for (int j = 0; j < img->width; ++j)
        {
            is_in_area = false;
            int p1 = 0, p2 = 0;
            if(i <= r && j <= r){
                is_in_area = true;
                p1 = i - r;
                p2 = j - r;
            } else if(i <= r && j >= img->width - r){
                is_in_area = true;
                p1 = i - r;
                p2 = j - (img->width - r);
            } else if(i >= img->height - r && j <= r){
                is_in_area = true;
                p1 = i - (img->height - r);
                p2 = j - r;
            } else if(i >= img->height - r && j >= img->width - r){
                is_in_area = true;
                p1 = i - (img->height - r);
                p2 = j - (img->width - r);
            }

            if(is_in_area){
                distance = ::pow((::pow(p1, 2)+::pow(p2,2)), 0.5);
                if(distance > r) continue;
            }

            int v = img->Get(i,j);
            painter.setPen(QColor(v,v,v, alpha));
            painter.drawPoint(j+x,i+y);
        }
    }
}



