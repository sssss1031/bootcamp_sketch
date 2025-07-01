#include "secondwindow.h"
#include "ui_secondwindow.h"
#include "touchdrawingwidget.h"
#include "protocol.h"
#include "client.h"
#include <QDebug>

SecondWindow::SecondWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SecondWindow),
    drawingWidget(nullptr)
{
    ui->setupUi(this);

    // QFrame에 TouchDrawingWidget 생성 및 배치
        drawingWidget = new TouchDrawingWidget(ui->frame);
        drawingWidget->setGeometry(ui->frame->rect());
        drawingWidget->show();


    // backbutton 클릭 시 backToMain 신호 발생
    connect(ui->backbutton, &QPushButton::clicked, this, &SecondWindow::backToMain);

    // lineEdit에서 엔터키를 누르면 onLineEditReturnPressed() 슬롯 호출
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &SecondWindow::onLineEditReturnPressed);

    // enterButton 엔터키를 누르면
    connect(ui->enterButton, &QPushButton::clicked, this, &SecondWindow::onLineEditReturnPressed);

}

SecondWindow::~SecondWindow()
{
    delete ui;
}

// 리사이즈 이벤트에서 drawingWidget 크기 자동조정
void SecondWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (ui->frame && drawingWidget) {
        drawingWidget->setGeometry(ui->frame->rect());
    }
}

void SecondWindow::onLineEditReturnPressed()
{
    QString text = ui->lineEdit->text();
    if (text.isEmpty()) {
            qDebug() << "Nothing.";
            send_answer("Nothing");
        } else {
            qDebug() << "Answer:" << text;
            send_answer(text.toStdString());
            // 추가 동작 필요 시 여기에 작성
        }
}
