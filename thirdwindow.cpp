#include "thirdwindow.h"
#include "ui_thirdwindow.h"
#include "touchdrawingwidget.h"
#include "drawingdispatcher.h"
#include "protocol.h"
#include "client.h"
#include "buttonmonitor.h"
#include <QDebug>

ThirdWindow::ThirdWindow(int maxPlayer, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ThirdWindow),
    m_maxPlayer(maxPlayer),
    ElapsedTime(0,0,20),
    m_count(8)
{
    ui->setupUi(this);
    this->setAutoFillBackground(true);

    // QFrame에 TouchDrawingWidget 생성 및 배치
    drawingWidget = new TouchDrawingWidget(ui->frame);
    drawingWidget->setGeometry(ui->frame->rect());
    drawingWidget->show();
    ui->countdown->hide();

    // backbutton 클릭 시 backToMain 신호 발생
    connect(ui->backbutton, &QPushButton::clicked, this, &ThirdWindow::backToMainRequested);

    // enter key
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &ThirdWindow::onLineEditReturnPressed);

    // enterButton
    connect(ui->enterButton, &QPushButton::clicked, this, &ThirdWindow::onLineEditReturnPressed);

    // send Packet
    connect(&DrawingDispatcher::instance(), &DrawingDispatcher::drawArrived, this,
        [this](int drawStatus, double x, double y, int color, int thick){
            if(drawingWidget) drawingWidget->onDrawPacket(drawStatus, x, y, color, thick);
        }
    );

    // timer
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ThirdWindow::updateTime);
    ui->timelabel->setText(ElapsedTime.toString("mm:ss"));
    ui->timelabel->setStyleSheet("color: black;");
    // 임시 : 바로 시작, 게임 시작 시 start(1000) 호출
    timer->start(1000);

    // countdown timer
    count_timer = new QTimer(this);
    connect(count_timer, &QTimer::timeout, this, &ThirdWindow::updateCountdown);

    //run_client();
    run_client(m_maxPlayer); // maxPlayer 인자 전달
    qDebug() << "third";
}

ThirdWindow::~ThirdWindow()
{
    delete ui;
}

void ThirdWindow::backToMainRequested() {
    disconnect_client();  // 연결 해제
    emit backToMain();  // 메인 윈도우로 돌아가기
}

// 리사이즈 이벤트에서 drawingWidget 크기 자동조정
void ThirdWindow::resizeEvent(QResizeEvent *event)
{
    // 배경이미지 설정
    QPixmap bkgnd(":/new/prefix1/background2_gpt.png");
    if (bkgnd.isNull()) {
        qDebug() << "Can not load Image";
    } else {
        bkgnd = bkgnd.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPalette palette;
        palette.setBrush(QPalette::Window, bkgnd);
        this->setPalette(palette);
    }
    QMainWindow::resizeEvent(event);
    if (ui->frame && drawingWidget) {
        drawingWidget->setGeometry(ui->frame->rect());
    }
}

void ThirdWindow::onLineEditReturnPressed()
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
void ThirdWindow::appendChatMessage(const QString& message) {
    qDebug() << "appendChatMessage called:" << message;
    ui->textEdit->append(message);
}

void ThirdWindow::correctRound(const QString& message){
    qDebug() << "correct! msg: " << message;

    // CORRECT : change questioner
    int correct_num = message.mid(16,1).toInt();
    int colon = message.indexOf(':');

    ui->correct->setText("correct! : Player " + QString::number(correct_num) + "\nANSWER : " +
                         message.mid(colon + 1).trimmed());
    ui->correct->raise();
    ui->correct->show();
    timer->stop();

    count_timer->start(1000);
    ui->countdown->setText("NEXT ROUND STARTS IN: " + (QString::number(m_count)) + " Secs..");
    ui->countdown->raise();
    ui->countdown->show();

    // after 8 secs, next round begins
    QTimer::singleShot(9000, this, [=](){
        ui->correct->hide();
        ui->countdown->hide();
        nextRound();
    });
}

void ThirdWindow::timeoverRound(){
    qDebug() << "time over";

    ui->timeover->raise();
    ui->timeover->show();
    timer->stop();

    count_timer->start(1000);
    ui->countdown->setText("NEXT ROUND STARTS IN: " + (QString::number(m_count)) + " Secs..");
    ui->countdown->raise();
    ui->countdown->show();

    // after 8 secs, next round begins
    QTimer::singleShot(9000, this, [=](){
        ui->timeover->hide();
        ui->countdown->hide();
        nextRound();
    });
}

void ThirdWindow::nextRound()
{
    // widget
    drawingWidget->erase();
    drawingWidget->reset();

    // timer
    ElapsedTime = QTime(0,0,20);
    ui->timelabel->setText(ElapsedTime.toString("mm:ss"));
    ui->timelabel->setStyleSheet("color: black;");
    // 임시 : 바로 시작, 게임 시작 시 start(1000) 호출
    timer->start(1000);

    // countdown timer
    m_count = 8;
    count_timer->stop();
}

void ThirdWindow::updateTime()
{
    if (ElapsedTime > QTime(0, 0, 0))
    {
        ElapsedTime = ElapsedTime.addSecs(-1);
        ui->timelabel->setText(ElapsedTime.toString("mm:ss"));
        if (ElapsedTime < QTime(0,0,31))
        {
            ui->timelabel->setStyleSheet("color: red;");
        }
        qDebug() << ElapsedTime.toString("mm:ss");
    }
    else {
        timeoverRound();
    }
}

void ThirdWindow::updateCountdown()
{

    if (m_count > 0)
    {
        m_count--;
        ui->countdown->setText("NEXT ROUND STARTS IN: " + (QString::number(m_count)) + " Secs..");
        qDebug() << "m_count:" << m_count;
    }
}
