#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "secondwindow.h"
#include "client.h"
#include "playbgm.h"
#include <QPixmap>
#include <QPalette>
#include <QDebug>
#include <QScreen>
#include "playercountdispatcher.h"
#include <QMessageBox>

MainWindow* g_mainWindow = nullptr;
SecondWindow* g_secondWindow = nullptr;
bool isInWaitingState = false;
extern std::atomic<bool> isRejected;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , secondWindow(nullptr)
{
    g_mainWindow = this;
    ui->setupUi(this);
<<<<<<< HEAD
    connect(&PlayerCountDispatcher::instance(), &PlayerCountDispatcher::playerCountUpdated, this, &MainWindow::onPlayerCountUpdated);
=======

>>>>>>> f2de1044ab4111b03c148399b405b98c57bbace2
    this->setAutoFillBackground(true);
    LoopBgm *bgm = new LoopBgm(this);
    bgm->startLoop("/mnt/nfs/bgm.wav", "hw:3,0");
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QPixmap bkgnd(":/new/prefix1/background3_gpt.jpeg");
    if (bkgnd.isNull()) {
        qDebug() << "Can not load Image";
    } else {
        bkgnd = bkgnd.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                QPalette palette;
                palette.setBrush(QPalette::Window, bkgnd);
                this->setPalette(palette);
            }
            QMainWindow::resizeEvent(event);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (secondWindow) delete secondWindow;
}

//void MainWindow::on_pushButton_clicked()
//{
//    // 두 번째 창 생성 및 표시
//    if (!secondWindow) {
//        secondWindow = new SecondWindow();
//        QObject::disconnect(secondWindow, &SecondWindow::backToMain, nullptr, nullptr);
//        connect(secondWindow, &SecondWindow::backToMain, this, [this]() {

//            this->show();
//            secondWindow->deleteLater();
//            if (secondWindow) {
//                secondWindow = nullptr;
//                g_secondWindow = nullptr;
//            }
//        });
//    }
//    g_secondWindow = secondWindow;
//    secondWindow->show();

//    this->hide(); // 현재 메인 창 숨기기 (필요시)
//}
void MainWindow::on_pushButton_3p_clicked()
{
    desiredMaxPlayer = 3;
    isInWaitingState = true;
    run_client(desiredMaxPlayer);
}

void MainWindow::on_pushButton_2p_clicked()
{

    desiredMaxPlayer = 2;
    isInWaitingState = true;
    run_client(desiredMaxPlayer);
}

//void MainWindow::showSecondWindow()
//{
//    if (!secondWindow) {
//        secondWindow = new SecondWindow(desiredMaxPlayer); //서버에 max_player값 전달
//        QObject::disconnect(secondWindow, &SecondWindow::backToMain, nullptr, nullptr);
//        connect(secondWindow, &SecondWindow::backToMain, this, [this]() {
//            this->show();
//            secondWindow->deleteLater();
//            if (secondWindow) {
//                secondWindow = nullptr;
//                g_secondWindow = nullptr;
//            }
//        });
//    }
//    g_secondWindow = secondWindow;
//    secondWindow->show();
//    this->hide();
//}

void MainWindow::onPlayerCountUpdated(int current, int max) {
    currentPlayerCount = current;
    serverMaxPlayer = max;
    qDebug() << "current"<< current << "Max" << max << "desire max"<< desiredMaxPlayer;
    if (isInWaitingState) {
        ui->label_playerCount->setText(QString("Waiting... (%1/%2)").arg(current).arg(max));
    } else {
        ui->label_playerCount->setText("");
    }

    if (current == max && !secondWindow && desiredMaxPlayer == max) {
        isInWaitingState = false; // 게임 시작이므로 waiting 상태 해제
        secondWindow = new SecondWindow(max);
        ui->label_playerCount->setText("");
        QObject::disconnect(secondWindow, &SecondWindow::backToMain, nullptr, nullptr);
        connect(secondWindow, &SecondWindow::backToMain, this, [this]() {

            this->show();
            isInWaitingState = false;
            ui->label_playerCount->setText("");
            secondWindow->deleteLater();
            if (secondWindow) {
                secondWindow = nullptr;
                g_secondWindow = nullptr;
            }
        });
        g_secondWindow = secondWindow;
        secondWindow->show();
        this->hide();
    }
}


void MainWindow::showConnectionRejectedMessage() {
    QMessageBox::warning(this, "Fail", "Other play available");
}
