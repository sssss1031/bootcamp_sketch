#ifndef SECONDWINDOW_H
#define SECONDWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include "touchdrawingwidget.h"
#include "scorelist.h"

#define TIME_OVER 9999
#define BACKTOMAIN 10000
#define MAX_ROUND 1

namespace Ui {
class SecondWindow;
}

class SecondWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SecondWindow(int maxPlayer = 2, QWidget *parent = nullptr);
    TouchDrawingWidget *drawingWidget;
    void roundinc();
    ~SecondWindow();

public slots:
    void setMyNum(int num);

signals:
    void backToMain();

private:
    int m_maxPlayer;
    Ui::SecondWindow *ui;
    QTime ElapsedTime;
    int m_count;
    bool m_blinkStarted;
    QTimer *timer;
    QTimer *count_timer;
    QTimer *blink_timer;
    bool onBlink;
    int current_round;

    void timeoverRound();
    void nextRound(int correct_num);
    void showResult();

    const int originX = 110;
    const int originY = 20;
    const int originW = 61;
    const int originH = 61;
    const int centerX = originX + originW / 2;
    const int centerY = originY + originH / 2;
protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    //void onLineEditReturnPressed();
    void backToMainRequested();
    void appendChatMessage(const QString& message);
    void correctRound(const QString &message);
    void onPenChanged(int color, int width);
    void updateTime();
    void updateCountdown();
    void updateBlink();

public slots:
    void updateScoreboard(const ScoreList& players);
    void updateResultboard(const ScoreList& players);
    void resetPenButtons();
    void showTimeOverAnswer(const QString& answer);
};

#endif // SECONDWINDOW_H
