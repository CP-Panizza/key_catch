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


int main(int argc, char *argv[])
{
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
        TTipWidget::ShowMassage(&w,"holding <leftbutton> choose, <rightbutton/esc> quit!");
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
        TTipWidget::ShowMassage(&w,"holding <leftbutton> draw, <mousewheel> toggle color, <rightbutton/esc> quit!");
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


    QObject::connect(float_pan.record_btn, &QPushButton::clicked, [&](){
        float_pan.is_record = !float_pan.is_record;
        if(float_pan.is_record){
            TTipWidget::ShowMassage(&w, "start record!");
            if(w.screen_cap->m_stop){
                w.screen_cap->m_stop = false;
            }
            w.audio_recorder->startRecord();
            w.screen_cap->start();

            float_pan.record_btn->setToolTip("recording");
            float_pan.record_btn->setStyleSheet("QPushButton{border-image:url(:/recording.png); width:30px; height: 30px;}"+ HOVER_BORDER);
        } else {

            w.screen_cap->stop();
            w.audio_recorder->stopRecord();
            w.creating_video = true;
            w.check_creating_video->start(1000);
            std::thread mix_thread([&](){
                while(!w.screen_cap->m_is_return){
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
                qDebug() << "stop";
                QString strAppDir = QApplication::applicationDirPath();
                QProcess proc;
                QString avi_file = strAppDir + QString("/tmp.avi");
                QString wav_file = strAppDir + QString("/tmp.wav");
                QString ffmpeg_file = strAppDir + QString("/ffmpeg.exe");
                time_t now(0);
                now = time(NULL);
                struct tm *timeinfo;
                timeinfo = localtime(&now);
                char buf[60];
                ::strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M-%S", timeinfo);
                QString out_put_file = QString(buf) + "-screen_cap.avi";
                qDebug() << out_put_file;
                w.create_output_file = QApplication::applicationDirPath() + "/output/" + out_put_file;
                QString cmd = ffmpeg_file +" -y -i " + avi_file + " -i " + wav_file + " -vcodec copy -acodec copy " + QString("./output/") + out_put_file;
                qDebug() << cmd;
                proc.start(cmd);
                proc.waitForFinished();
                proc.close();
                w.creating_video = false;
            });
            mix_thread.detach();
            float_pan.record_btn->setToolTip("record");
            float_pan.record_btn->setStyleSheet("QPushButton{border-image:url(:/record.png); width:30px; height: 30px;}"+ HOVER_BORDER);
        }
    });

    w.show();
    float_pan.show();
    return a.exec();
}
