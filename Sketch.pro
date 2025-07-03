#-------------------------------------------------
#
# Project created by QtCreator 2025-07-01T09:36:59
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Sketch
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
        secondwindow.cpp\
        touchdrawingwidget.cpp \
    client.cpp \
    gpio_control.cpp \
    buttonmonitor.cpp

HEADERS  += mainwindow.h \
        secondwindow.h\
        touchdrawingwidget.h \
    touchdrawingwidget.h \
    client.h \
    protocol.h \
    gpio_control.h \
    custom_ioctl.h \
    drawingdispatcher.h \
    buttonmonitor.h \
    playbgm.h

FORMS    += mainwindow.ui \
        secondwindow.ui

RESOURCES += resources.qrc
