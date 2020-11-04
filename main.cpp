#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include "subwindow.h"
#include <QPushButton>
#include <QObject>
#include <QDebug>
#include <QGuiApplication>
#include <QDateTime>
#include <QScreen>
#include <QPixmap>
#include <QDir>
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <thread>
#include <QProcess>
#include "ttipwidget.h"
#include "audiorecorder.h"
#include <time.h>
#include "toojpeg.h"
#include "utils.h"
#include <QProcess>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <chrono>
#include "kc_signal.h"


//ffmpeg -f gdigrab -framerate 30 -offset_x 0 -offset_y 0 -video_size 1366x768 -i desktop out.mpg
int main(int argc, char *argv[])
{

    if(!::SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS)){
        qDebug() << "SetPriorityClass fail!";
    }
    unsigned long proc_id =  GetCurrentProcessId();
    qDebug() << "CurrentProcessId: " << proc_id;

    if(!dir_exists("tmp/")){
        CreateFileDir("tmp/");
    }

    if(!dir_exists("output/")){
        CreateFileDir("output/");
    }

    const char * str = ".\\tmp\\*";
    delete_file(str, NULL);
    QApplication::setQuitOnLastWindowClosed(false);
    QApplication a(argc, argv);
    MainWindow w;
    w.setApplication(&a);

    SubWindow float_pan;


    w.self_hwnd.push_back((HWND)w.winId());
    w.self_hwnd.push_back((HWND)float_pan.winId());
    w.self_hwnd.push_back((HWND)TTipWidget::Instance().winId());
    w.float_pan_cb = [&](){  //set callback func
        float_pan.activateWindow();
    };

    auto open_key_log = [&](){
        if(!float_pan.key_log){
            w.key_log = true;
            float_pan.key_log = true;
            float_pan.key_log_btn->setStyleSheet("QPushButton{border-image:url(:/kb.png); width:30px; height: 30px;}" + HOVER_BORDER);
        }
    };

    auto recover_mainwindow_style = [&](){
        ::SetWindowLong((HWND)w.winId(), GWL_EXSTYLE, 256);  //cancel mouse penetrate
        w.activateWindow();
    };
    QObject::connect(float_pan.key_log_btn, &QPushButton::clicked, [&](){
        if(w.stuta == MainWindow::ScreenStuta::PAINTING || w.stuta == MainWindow::ScreenStuta::DRAWING){
            return;
        }
        w.activateWindow();
        float_pan.key_log = !float_pan.key_log;
        w.key_log = !w.key_log;
        if(float_pan.key_log){
            TTipWidget::ShowMassage(&w, "key log on!");
            float_pan.key_log_btn->setStyleSheet("QPushButton{border-image:url(:/kb.png); width:30px; height: 30px;}" + HOVER_BORDER);
        } else {
            TTipWidget::ShowMassage(&w, "key log off!");
            w.keylog.clear();
            float_pan.key_log_btn->setStyleSheet("QPushButton{border-image:url(:/_kb.png); width:30px; height: 30px;}" + HOVER_BORDER);
        }
    });

    QObject::connect(float_pan.cut_btn, &QPushButton::clicked, [&](){
        if(w.stuta == MainWindow::ScreenStuta::PAINTING || w.stuta == MainWindow::ScreenStuta::DRAW_DONE_PAINTING){
            return;
        }

        open_key_log();
        TTipWidget::ShowMassage(&w,"hold <leftbutton> choose, <rightbutton/esc> quit!");
        QScreen *screen = QGuiApplication::primaryScreen();
        QPixmap screen_img = screen->grabWindow(0);
        w.hold_screen(screen_img);
        recover_mainwindow_style();
        if(w.stuta == MainWindow::NONE){
            w.stuta = MainWindow::ScreenStuta::PAINTING;
        } else if(w.stuta == MainWindow::ScreenStuta::DRAWING){
            w.stuta = MainWindow::ScreenStuta::DRAW_DONE_PAINTING;
        }
    });


    QObject::connect(float_pan.pan_btn, &QPushButton::clicked, [&](){
        TTipWidget::ShowMassage(&w,"hold <leftbutton> draw, <mousewheel> toggle color, <rightbutton/esc> quit!");
        open_key_log();
        w.init_canvas();
        recover_mainwindow_style();
        w.stuta = MainWindow::DRAWING;
    });

    QObject::connect(float_pan.nail_btn, &QPushButton::clicked, [&](){
        if(w.stuta != MainWindow::NONE){
            return;
        }
        w.stuta = MainWindow::DRIVING_NAIL;
        w.activateWindow();
        TTipWidget::ShowMassage(&w, "click window toggle top!");
    });


    QString strAppDir = QApplication::applicationDirPath();
    kc_signal process_signal(std::string("kc_event"), kc_signal::Type::SENDER);;
    QProcess screen_recorder;
    QObject::connect(float_pan.record_btn, &QPushButton::clicked, [&](){
        float_pan.is_record = !float_pan.is_record;
        if(float_pan.is_record){
            TTipWidget::ShowMassage(&w, "start record!");
            process_signal.un_active();
            QString exe = strAppDir + QString("/kc_screen_recorder.exe");
            QString cmd = exe + " -f " + QString::number(w.screen_cap->m_fps)
                    + " -q " + QString::number(w.screen_cap->quality)
                    + " -t " + QString::number(w.screen_cap->thread_pool_size)
                    + " -o tmp/";
            w.audio_recorder->startRecord();
            screen_recorder.start(cmd);
            //process_signal.wait();

            //process_signal.un_active();
            //w.screen_cap->start(THREAD_PRIORITY_TIME_CRITICAL);

            float_pan.record_btn->setToolTip("recording");
            float_pan.record_btn->setStyleSheet("QPushButton{border-image:url(:/recording.png); width:30px; height: 30px;}"+ HOVER_BORDER);
            float_pan.m_hp = HIDEPOSATION::HP_Right;
            float_pan.hideWindow();
        } else {
            qDebug() << "click stop";
            process_signal.active();
            //w.screen_cap->stop();
            w.audio_recorder->stopRecord();
            w.creating_video = true;
            w.check_creating_video->start(1000);
            std::thread mix_thread([&](){
//                while(!w.screen_cap->m_is_return){
//                    std::this_thread::sleep_for(std::chrono::seconds(1));
//                }
                process_signal.wait();
                qDebug() << "stop";

                QProcess proc;
                //ffmpeg -f image2 -r 13.1579 -i ./tmp/%d.jpg -i ./tmp.wav -vcodec libx264 -acodec copy  out.mkv
                QString avi_file = strAppDir + QString("/tmp.avi");
                QString wav_file = strAppDir + QString("/tmp.wav");
                QString ffmpeg_file = strAppDir + QString("/ffmpeg.exe");

                QString cmd = ffmpeg_file + " -y -i " + wav_file + " -f mp2 " + strAppDir + "/tmp.mp3";
                qDebug() << cmd;
                proc.start(cmd);
                proc.waitForFinished();
                proc.close();


                QRect screenRect = QApplication::desktop()->screenGeometry();
                QString jpg2avi = strAppDir +  QString("/JPEG2AVI.exe");
                time_t now(0);
                now = time(NULL);
                struct tm *timeinfo;
                timeinfo = localtime(&now);
                char buf[60];
                ::strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M-%S", timeinfo);
                QString out_put_file = QString(buf) + "-screen_cap.avi";
                qDebug() << out_put_file;
                w.create_output_file = QApplication::applicationDirPath() + "/output/" + out_put_file;
                QString cmd_2 = jpg2avi + " -i ./tmp/ -s " + strAppDir + "/tmp.mp3 -f " + QString::number(w.screen_cap->m_fps)
                        + " -h " + QString::number(screenRect.height())
                        + " -w " + QString::number(screenRect.width())
                        + " -o ./output/" + out_put_file;
                QProcess proc_2;
                qDebug() << cmd_2;
                proc_2.start(cmd_2);
                proc_2.waitForFinished();
                proc_2.close();


                const char * str = ".\\tmp\\*";
                delete_file(str, NULL);
                w.creating_video = false;
                w.screen_cap->m_is_return = false;
            });
            mix_thread.detach();
            float_pan.record_btn->setToolTip("record");
            float_pan.record_btn->setStyleSheet("QPushButton{border-image:url(:/record.png); width:30px; height: 30px;}"+ HOVER_BORDER);
        }
    });

    QObject::connect(float_pan.sucker_btn, &QPushButton::clicked, [&](){
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        w.sucker_color = !w.sucker_color;
        if(w.sucker_color){
            w.show_color_str_timer->start(500);
        } else {
            w.show_color_str_timer->stop();
        }
    });


    QObject::connect(float_pan.open_file_btn, &QPushButton::clicked, [&](){
        QString strAppDir = QApplication::applicationDirPath();
        QUrl url(strAppDir + "/output");
        bool ok = QDesktopServices::openUrl(url);

        if(!ok){
            TTipWidget::ShowMassage(&w, "con not open: " + url.url());
        }
    });


    if(!w.set_hook()){
        a.quit();
    }
    w.show();
    float_pan.show();
    ::SetWindowPos(HWND(w.winId()), HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
    ::SetWindowLong(HWND(w.winId()), GWL_EXSTYLE, ::GetWindowLong(HWND(w.winId()), GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_LAYERED); //set window mouse penetrate
    return a.exec();
}
