#ifndef BUTTONMONITOR
#define BUTTONMONITOR

#include <QObject>
#include <QSocketNotifier>

class ButtonMonitor : public QObject {
    Q_OBJECT
public:
    explicit ButtonMonitor(const QString &devPath, QObject *parent = nullptr);
    ~ButtonMonitor();

signals:
    void buttonPressed(int idx);

private slots:
    void handleButtonEvent();

private:
    int fd;
    QSocketNotifier *notifier;
};

#endif // BUTTONMONITOR

