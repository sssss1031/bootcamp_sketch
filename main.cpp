#include "mainwindow.h"
#include <QApplication>

SecondWindow* g_secondWindow = nullptr;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
