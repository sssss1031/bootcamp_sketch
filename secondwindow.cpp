#include "secondwindow.h"
#include "ui_secondwindow.h"
#include "touchdrawingwidget.h"
#include "drawingdispatcher.h"
#include "protocol.h"
#include "client.h"
#include "buttonmonitor.h"
#include <QDebug>

SecondWindow::SecondWindow(int maxPlayer, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SecondWindow),
    m_maxPlayer(maxPlayer),
    ElapsedTime(0,0,50)
{
    ui->setupUi(this);
    this->setAutoFillBackground(true);

    // QFrame에 TouchDrawingWidget 생성 및 배치
    drawingWidget = new TouchDrawingWidget(ui->frame);
    drawingWidget->setGeometry(ui->frame->rect());
    drawingWidget->show();

    // backbutton 클릭 시 backToMain 신호 발생
    connect(ui->backbutton, &QPushButton::clicked, this, &SecondWindow::backToMainRequested);

    // enter key
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &SecondWindow::onLineEditReturnPressed);

    // enterButton
    connect(ui->enterButton, &QPushButton::clicked, this, &SecondWindow::onLineEditReturnPressed);

    // send Packet
    connect(&DrawingDispatcher::instance(), &DrawingDispatcher::drawArrived, this,
        [this](int drawStatus, double x, double y, int color, int thick){
            if(drawingWidget) drawingWidget->onDrawPacket(drawStatus, x, y, color, thick);
        }
    );
    // button monitoring
    auto *btnMon = new ButtonMonitor("/dev/mydev", this);
        connect(btnMon, &ButtonMonitor::buttonPressed, this, [=](int idx){
            drawingWidget->erase();
        });
    
    // pen changed
    connect(drawingWidget, &TouchDrawingWidget::penChanged, this, &SecondWindow::onPenChanged);
    connect(ui->colorbutton, &QPushButton::clicked, this, [this]() {drawingWidget->colorClicked();});
    connect(ui->widthbutton, &QPushButton::clicked, this, [this]() {drawingWidget->widthClicked();});

    ui->buttoncover->raise();
    ui->colorbutton->raise();
    ui->widthbutton->raise();

    // timer
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SecondWindow::updateTime);
    ui->timelabel->setText(ElapsedTime.toString("mm:ss"));
    ui->timelabel->setStyleSheet("color: black;");
    // 임시 : 바로 시작, 게임 시작 시 start(1000) 호출
    timer->start(1000);
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

void SecondWindow::onPenChanged(int color, int width)
{
    QColor qc;
    switch (color) {
            case 1: qc = Qt::black; break;
            case 2: qc = Qt::yellow; break;
            case 3: qc = Qt::red; break;
            case 4: qc = Qt::blue; break;
            case 5: qc = Qt::white; break;
    }
    
    int qw;
    switch (width) {
                case 10: qw = 15; break;
                case 11: qw = 30; break;
                case 12: qw = 45; break;
    }
    qDebug() << qw;
    QString style = QString("background-color: %1; border-radius: 20px;").arg(QColor(qc).name());
    
    ui->colorbutton->setStyleSheet(style);
    ui->widthbutton->setFixedWidth(qw);
}
    
void SecondWindow::updateTime()
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
    else { timer->stop(); }
}
