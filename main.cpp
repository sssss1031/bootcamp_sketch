#include "mainwindow.h"
#include <QApplication>
#include <signal.h>
#include <unistd.h>

void handle_sigint(int sig) {
    kill(-getpgrp(), SIGTERM);
    exit(0);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    setpgid(0, 0);
    signal(SIGINT, handle_sigint);

    return a.exec();
}
