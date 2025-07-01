#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "secondwindow.h"
#include <QPixmap>
#include <QPalette>
#include <QDebug>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , secondWindow(nullptr) // 초기화
{
    ui->setupUi(this);
//    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
//        this->resize(screenGeometry.width(), screenGeometry.height());
    this->setAutoFillBackground(true);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::on_pushButton_clicked);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QPixmap bkgnd(":/new/prefix1/background_gpt.png");
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

void MainWindow::on_pushButton_clicked()
{
    // 두 번째 창 생성 및 표시
    if (!secondWindow) {
        secondWindow = new SecondWindow();
        connect(secondWindow, &SecondWindow::backToMain, this, [this]() {
            this->show();
            secondWindow->hide();
        });
    }

    secondWindow->show();
    this->hide(); // 현재 메인 창 숨기기 (필요시)
}
