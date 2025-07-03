#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "secondwindow.h"
#include "client.h"
#include "playbgm.h"
#include <QPixmap>
#include <QPalette>
#include <QDebug>
#include <QScreen>

SecondWindow* g_secondWindow = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , secondWindow(nullptr)
{
    ui->setupUi(this);

    this->setAutoFillBackground(true);
    LoopBgm *bgm = new LoopBgm(this);
    bgm->startLoop("/tmp/bgm.wav", "hw:3,0");
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
    showSecondWindow();
}

void MainWindow::on_pushButton_2p_clicked()
{
    desiredMaxPlayer = 2;
    showSecondWindow();
}

void MainWindow::showSecondWindow()
{
    if (!secondWindow) {
        secondWindow = new SecondWindow(desiredMaxPlayer); //서버에 max_player값 전달
        QObject::disconnect(secondWindow, &SecondWindow::backToMain, nullptr, nullptr);
        connect(secondWindow, &SecondWindow::backToMain, this, [this]() {

            this->show();
            secondWindow->deleteLater();
            if (secondWindow) {
                secondWindow = nullptr;
                g_secondWindow = nullptr;
            }
        });
    }
    g_secondWindow = secondWindow;
    secondWindow->show();
    this->hide();
}
