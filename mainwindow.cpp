#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "secondwindow.h"
#include "thirdwindow.h"
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
ThirdWindow* g_thirdWindow = nullptr;
bool isInWaitingState = false;
extern std::atomic<bool> isRejected;
extern int my_Num;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    //, secondWindow(nullptr)
    //, thirdWindow(nullptr)
{
    g_mainWindow = this;
    ui->setupUi(this);
    connect(&PlayerCountDispatcher::instance(), &PlayerCountDispatcher::playerCountUpdated, this, &MainWindow::onPlayerCountUpdated);

//    this->setAutoFillBackground(true);
//    LoopBgm *bgm = new LoopBgm(this);
//    bgm->startLoop("/mnt/nfs/bgm.wav", "hw:3,0");
//    SoundExecutor::playOnLoop("bgm.wav");

    PlayBgm::playOnLoop(PlayBgm::BGM);
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
    if (thirdWindow) delete thirdWindow;
}

void MainWindow::on_pushButton_3p_clicked()
{
    qDebug()<<"3pclicked!!!!!!!!!!";
    desiredMaxPlayer = 3;
    isInWaitingState = true;
    run_client(desiredMaxPlayer);
}

void MainWindow::on_pushButton_2p_clicked()
{
    qDebug()<<"2pclicked!!!!!!!!!!";
    desiredMaxPlayer = 2;
    isInWaitingState = true;
    run_client(desiredMaxPlayer);
}


void MainWindow::onPlayerCountUpdated(int current, int max) {
    currentPlayerCount = current;
    serverMaxPlayer = max;
    qDebug() << "current" << current << "Max" << max << "desire max" << desiredMaxPlayer;
    if (isInWaitingState) {
        ui->label_playerCount->setText(QString("Waiting... (%1/%2)").arg(current).arg(max));
    } else {
        ui->label_playerCount->setText("");
    }
}

void MainWindow::onSelectedPlayerNickname(const QString& nickname) {
    // "playerN"에서 N 추출
    QRegExp rx("player(\\d+)");
    int selectedNum = 0;
    if (rx.indexIn(nickname) != -1) {
        selectedNum = rx.cap(1).toInt();
    }
    if (!secondWindow) {
        qDebug()<<"make secondwindow!!!!!!!!!!!!!!";
        secondWindow = new SecondWindow(serverMaxPlayer); // maxPlayer로 생성
        connect(secondWindow, &SecondWindow::backToMain, this, [this]() {
            this->show();
            isInWaitingState = false;
            ui->label_playerCount->setText("");
            secondWindow->deleteLater();
            thirdWindow->deleteLater();
            if (secondWindow) {
                secondWindow->drawingWidget = nullptr;
                secondWindow = nullptr;
                g_secondWindow = nullptr;
            }
            if (thirdWindow) {
                thirdWindow->drawingWidget = nullptr;
                thirdWindow = nullptr;
                g_thirdWindow = nullptr;
            }
        });
        g_secondWindow = secondWindow;
        secondWindow->setMyNum(my_Num);
    }
    // ThirdWindow 띄우기
    if (!thirdWindow) {
        qDebug()<<"make thirdwindow!!!!!!!!!!!!!!";
        thirdWindow = new ThirdWindow(serverMaxPlayer);
        connect(thirdWindow, &ThirdWindow::backToMain, this, [this]() {
            this->show();
            isInWaitingState = false;
            ui->label_playerCount->setText("");
            thirdWindow->deleteLater();
            secondWindow->deleteLater();
            if (thirdWindow) {
                thirdWindow->drawingWidget = nullptr;
                thirdWindow = nullptr;
                g_thirdWindow = nullptr;
            }
            if (secondWindow) {
                secondWindow->drawingWidget = nullptr;
                secondWindow = nullptr;
                g_secondWindow = nullptr;
            }
        });
        g_thirdWindow = thirdWindow;
        thirdWindow->setMyNum(my_Num);
    }
    // 내 번호와 비교
    if (selectedNum == my_Num) {
        // SecondWindow 띄우기

        secondWindow->show();
        this->hide();
    } else {

        thirdWindow->show();
        this->hide();
    }
}

void MainWindow::showConnectionRejectedMessage() {
    QMessageBox::warning(this, "Fail", "Other play available");
}
