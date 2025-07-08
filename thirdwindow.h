#ifndef THIRDWINDOW_H
#define THIRDWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include <QLabel>
#include <QFrame>
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
    int dotCount;
    QTimer *timer;
    QTimer *count_timer;
    QTimer *waiting_timer;
    QTimer *blink_timer;
    QFrame* hintFrame;
    QLabel* hintLabel;
    QLabel* touchLabel;

    bool round_start;
    bool onBlink;
    void timeoverRound();
    void nextRound(int correct_num);
    QString makeHint(const QString& answer) const;

    QString m_answerStr;
    void showHint(const QString& answer);
    void hideHint();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

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
    void onBeginRound(const QString& answer);
    void setMyNum(int num);
    void showTimeOverAnswer(const QString& answer);
};

#endif // THIRDWINDOW_H
