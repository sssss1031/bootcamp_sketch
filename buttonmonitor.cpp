#include "buttonmonitor.h"
#include <fcntl.h>
#include <unistd.h>
#include <QDebug>

ButtonMonitor::ButtonMonitor(const QString &devPath, QObject *parent) : QObject(parent) {
    fd = open(devPath.toLocal8Bit().constData(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        qDebug() << "Failed to open device\n";
        notifier = nullptr;
        return;
    }
    notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &ButtonMonitor::handleButtonEvent);
}

ButtonMonitor::~ButtonMonitor() {
    if (notifier) delete notifier;
    if (fd >= 0) close(fd);
}

void ButtonMonitor::handleButtonEvent() {
    int btn_idx;
    ssize_t n = read(fd, &btn_idx, sizeof(int));
    if (n == sizeof(int)) {
        emit buttonPressed(btn_idx);
    }
}
