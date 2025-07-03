#include "secondwindow.h"
#include "ui_secondwindow.h"
#include "touchdrawingwidget.h"
#include "drawingdispatcher.h"
#include "protocol.h"
#include "client.h"
#include <QDebug>

SecondWindow::SecondWindow(int maxPlayer, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SecondWindow),
    m_maxPlayer(maxPlayer)
{
    ui->setupUi(this);

    // 한글 폰트 지정
    //QFont hangulFont("NanumGothic");
    //ui->textEdit->setFont(hangulFont);
    //ui->lineEdit->setFont(hangulFont);

    // QFrame에 TouchDrawingWidget 생성 및 배치
        drawingWidget = new TouchDrawingWidget(ui->frame);
        drawingWidget->setGeometry(ui->frame->rect());
        drawingWidget->show();


    // backbutton 클릭 시 backToMain 신호 발생
    connect(ui->backbutton, &QPushButton::clicked, this, &SecondWindow::backToMainRequested);

    // lineEdit에서 엔터키를 누르면 onLineEditReturnPressed() 슬롯 호출
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &SecondWindow::onLineEditReturnPressed);

    // enterButton 엔터키를 누르면
    connect(ui->enterButton, &QPushButton::clicked, this, &SecondWindow::onLineEditReturnPressed);

    connect(&DrawingDispatcher::instance(), &DrawingDispatcher::drawArrived, this,
        [this](int drawStatus, double x, double y, int color, int thick){
            if(drawingWidget) drawingWidget->onDrawPacket(drawStatus, x, y, color, thick);
        }
    );
    //run_client();
    run_client(m_maxPlayer); // maxPlayer 인자 전달

}

SecondWindow::~SecondWindow()
{
    delete ui;
}

void SecondWindow::backToMainRequested() {
    disconnect_client();  // 연결 해제
    emit backToMain();  // 메인 윈도우로 돌아가기
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

//메시지 채팅
void SecondWindow::appendChatMessage(const QString& message) {
    qDebug() << "appendChatMessage called:" << message;
    ui->textEdit->append(message);
}
