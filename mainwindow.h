#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "secondwindow.h"
#include "thirdwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void showConnectionRejectedMessage();
    void onSelectedPlayerNickname(const QString& nickname);

private slots:
    void onPlayerCountUpdated(int current, int max);
    void on_pushButton_3p_clicked();
    void on_pushButton_2p_clicked();

private:
    SecondWindow* secondWindow = nullptr;
    ThirdWindow* thirdWindow = nullptr;
    int serverMaxPlayer = 0;
    int currentPlayerCount = 0;
    int desiredMaxPlayer = 2;
    Ui::MainWindow *ui;
    void resizeEvent(QResizeEvent *event) override;
};

extern MainWindow* g_mainWindow;

#endif // MAINWINDOW_H


