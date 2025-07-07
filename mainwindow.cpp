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

    //PlayBgm::playOnLoop("bgm.wav");
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
    desiredMaxPlayer = 3;
    isInWaitingState = true;
    run_client(desiredMaxPlayer);
}

void MainWindow::on_pushButton_2p_clicked()
{
    desiredMaxPlayer = 1;
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
    // 내 번호와 비교
    selectedNum = 2;
    if (selectedNum == my_Num) {
        // SecondWindow 띄우기
        if (!secondWindow) {
            secondWindow = new SecondWindow(serverMaxPlayer); // maxPlayer로 생성
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
        }
        secondWindow->show();
        this->hide();
    } else {
        // ThirdWindow 띄우기
        if (!thirdWindow) {
            thirdWindow = new ThirdWindow(serverMaxPlayer);
            connect(thirdWindow, &ThirdWindow::backToMain, this, [this]() {
                this->show();
                isInWaitingState = false;
                ui->label_playerCount->setText("");
                thirdWindow->deleteLater();
                if (thirdWindow) {
                    thirdWindow = nullptr;
                    g_thirdWindow = nullptr;
                }
            });
            g_thirdWindow = thirdWindow;
        }
        thirdWindow->show();
        this->hide();
    }
}

void MainWindow::showConnectionRejectedMessage() {
    QMessageBox::warning(this, "Fail", "Other play available");
}
