#ifndef SECONDWINDOW_H
#define SECONDWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QTimer>
#include "touchdrawingwidget.h"
<<<<<<< HEAD
#include "scorelist.h"

=======
#define TIME_OVER 9999
>>>>>>> d95dbc4ae6759911a7ab48a3d3b7983cb5041fe4
namespace Ui {
class SecondWindow;
}

class SecondWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SecondWindow(int maxPlayer = 2, QWidget *parent = nullptr);
    TouchDrawingWidget *drawingWidget;
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
    QTimer *timer;
    QTimer *count_timer;
    void timeoverRound();
    void nextRound(int correct_num);

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
<<<<<<< HEAD

public slots:
    void updateScoreboard(const ScoreList& players);
=======
    void resetPenButtons();
>>>>>>> d95dbc4ae6759911a7ab48a3d3b7983cb5041fe4
};

#endif // SECONDWINDOW_H
