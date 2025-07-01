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
        touchdrawingwidget.cpp

HEADERS  += mainwindow.h \
        secondwindow.h\
        touchdrawingwidget.h \
    touchdrawingwidget.h

FORMS    += mainwindow.ui \
        secondwindow.ui

RESOURCES += resources.qrc
