#ifndef PLAYBGM
#define PLAYBGM

#include <QProcess>
#include <QObject>
#include <QCoreApplication>

class LoopBgm : public QObject {
    Q_OBJECT
public:
    explicit LoopBgm(QObject *parent = nullptr)
        : QObject(parent), process(nullptr) {}

    void startLoop(const QString &wavFile, const QString &device = "hw:3,0") {
        this->wavFile = wavFile;
        this->device  = device;
        playOnce();
    }

private slots:
    void playOnce() {
        if (process) {
            process->deleteLater();
            process = nullptr;
        }
        process = new QProcess(this);
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &LoopBgm::playOnce);
        process->start("aplay", QStringList() << "-D" << device << wavFile);
    }

private:
    QString wavFile;
    QString device;
    QProcess *process;
};


#endif // PLAYBGM

