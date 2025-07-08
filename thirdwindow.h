#ifndef THIRDWINDOW_H
#define THIRDWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include "touchdrawingwidget.h"
#include "scorelist.h"

#define TIME_OVER 9999
namespace Ui {
class ThirdWindow;
}

class ThirdWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ThirdWindow(int maxPlayer = 2, QWidget *parent = nullptr);
    TouchDrawingWidget *drawingWidget;
    ~ThirdWindow();


signals:
    void backToMain();

private:
    int m_maxPlayer;
    Ui::ThirdWindow *ui;

    QTime ElapsedTime;
    int m_count;
    bool m_blinkStarted;
    QTimer *timer;
    QTimer *count_timer;
    bool round_start = false;

    void timeoverRound();
    void nextRound(int correct_num);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onLineEditReturnPressed();
    void backToMainRequested();
    void appendChatMessage(const QString& message);
    void correctRound(const QString &message);
    void updateTime();
    void updateCountdown();

public slots:
    void updateScoreboard(const ScoreList& players);
    void onBeginRound();
};

#endif // THIRDWINDOW_H
