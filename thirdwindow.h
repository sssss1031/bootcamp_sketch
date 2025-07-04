#ifndef THIRDWINDOW_H
#define THIRDWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include "touchdrawingwidget.h"

namespace Ui {
class ThirdWindow;
}

class ThirdWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ThirdWindow(int maxPlayer = 2, QWidget *parent = nullptr);
    ~ThirdWindow();


signals:
    void backToMain();

private:
    int m_maxPlayer;
    Ui::ThirdWindow *ui;
    TouchDrawingWidget *drawingWidget;
    QTime ElapsedTime;
    QTimer *timer;
    void timeoverRound();
    void nextRound();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onLineEditReturnPressed();
    void backToMainRequested();
    void appendChatMessage(const QString& message);
    void correctRound(const QString &message);
    void updateTime();
};

#endif // THIRDWINDOW_H

