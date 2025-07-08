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
<<<<<<< HEAD
    bool m_blinkStarted;
=======
    int dotCount;
>>>>>>> 36760ad20a0c7687ad95d761da599236d5283b70
    QTimer *timer;
    QTimer *count_timer;
    QTimer *waiting_timer;
    QTimer *blink_timer;

    bool round_start;
    bool onBlink;
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
    void updateWaiting();
    void updateBlink();

public slots:
    void updateScoreboard(const ScoreList& players);
    void onBeginRound();
<<<<<<< HEAD
=======
    void setMyNum(int num);
    void showTimeOverAnswer(const QString& answer);
>>>>>>> 36760ad20a0c7687ad95d761da599236d5283b70
};

#endif // THIRDWINDOW_H
