#ifndef SECONDWINDOW_H
#define SECONDWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include "touchdrawingwidget.h"

namespace Ui {
class SecondWindow;
}

class SecondWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SecondWindow(int maxPlayer = 2, QWidget *parent = nullptr);
    ~SecondWindow();


signals:
    void backToMain();

private:
    int m_maxPlayer;
    Ui::SecondWindow *ui;
    TouchDrawingWidget *drawingWidget;
    QTime ElapsedTime;
    QTimer *timer;
protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onLineEditReturnPressed();
    void backToMainRequested();
    void appendChatMessage(const QString& message);
    void onPenChanged(int color, int width);
    void updateTime();
};

#endif // SECONDWINDOW_H
