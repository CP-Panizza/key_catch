QT += core gui multimedia winextras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -pthread -O3 -Wall -g

CONFIG += c++11



# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    audiorecorder.cpp \
    avilib.c \
    item.cpp \
    keylog.cpp \
    main.cpp \
    mainwindow.cpp \
    mylineedit.cpp \
    screencap.cpp \
    subwindow.cpp \
    toojpeg.cpp \
    ttipwidget.cpp \
    utils.cpp

HEADERS += \
    audiorecorder.h \
    avilib.h \
    item.h \
    kc_thread.h \
    keylog.h \
    lock_free_queue.h \
    mainwindow.h \
    matrix.hpp \
    mylineedit.h \
    screencap.h \
    spin_mutex.h \
    subwindow.h \
    thread_pool.h \
    toojpeg.h \
    ttipwidget.h \
    utils.h


INCLUDEPATH += C:\Users\cmj\Desktop\Qt\key_catch_new\dep

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

OTHER_FILES += app_icon.rc
RC_FILE += app_icon.rc
