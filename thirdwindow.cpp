#include "thirdwindow.h"
#include "ui_thirdwindow.h"
#include "touchdrawingwidget.h"
#include "drawingdispatcher.h"
#include "protocol.h"
#include "client.h"
#include "buttonmonitor.h"
#include "gpio_control.h"
#include <QDebug>
#include "chatmessagedispatcher.h"

ThirdWindow::ThirdWindow(int maxPlayer, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ThirdWindow),
    m_maxPlayer(maxPlayer),
    ElapsedTime(0,0,20),
    m_count(8),
    m_blinkStarted (false)
{
    ui->setupUi(this);
    this->setAutoFillBackground(true);

    // QFrame에 TouchDrawingWidget 생성 및 배치
    drawingWidget = new TouchDrawingWidget(ui->frame);
    drawingWidget->setGeometry(ui->frame->rect());
    drawingWidget->show();
    drawingWidget->setEnabled(false);
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

    connect(&ChatMessageDispatcher::instance(), &ChatMessageDispatcher::chatMessageArrived,
            this, &ThirdWindow::appendChatMessage);


    // button monitoring
    auto *btnMon = new ButtonMonitor("/dev/mydev", this);
        connect(btnMon, &ButtonMonitor::buttonPressed, this, [=](int idx){
            drawingWidget->erase();
        });

    QGridLayout* grid = qobject_cast<QGridLayout*>(ui->scoreboard->layout());
        if (grid) {
            grid->addWidget(ui->P1, 0, 0);
            grid->addWidget(ui->P1_score, 1, 0);
            grid->addWidget(ui->P2, 0, 1);
            grid->addWidget(ui->P2_score, 1, 1);
            grid->addWidget(ui->P3, 0, 2);
            grid->addWidget(ui->P3_score, 1, 2);
        }
        if (g_hasPendingScore) {
            updateScoreboard(g_pendingScoreList);
            g_hasPendingScore = false;
        }
    // timer
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ThirdWindow::updateTime);
    ui->timelabel->setText(ElapsedTime.toString("mm:ss"));
    ui->timelabel->setStyleSheet("color: black;");

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

void ThirdWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    updateScoreboard(g_pendingScoreList);
}


void ThirdWindow::onBeginRound()
{
    round_start = true;;
    timer->start(1000);

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
        nextRound(correct_num);
    });
}

void ThirdWindow::updateScoreboard(const ScoreList& players)
{


    QLabel* nameLabels[3] = {ui->P1, ui->P2, ui->P3};
    QLabel* scoreLabels[3] = {ui->P1_score, ui->P2_score, ui->P3_score};

    for (int i = 0; i < 3; ++i) {
            nameLabels[i]->setVisible(false);
            scoreLabels[i]->setVisible(false);
        }

     int n = players.size();
     int start_col = (3-n)/2;

     for (int i = 0; i < n; ++i) {
             int idx = start_col + i;
             if (idx < 0 || idx >= 3) continue;
             nameLabels[idx]->setText(players[i].first);
             scoreLabels[idx]->setText(QString::number(players[i].second));
             nameLabels[idx]->setVisible(true);
             scoreLabels[idx]->setVisible(true);
         }

         ui->scoreboard->update();
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
        nextRound(TIME_OVER);
    });
}

void ThirdWindow::nextRound(int correct_num)
{
    // widget
    m_blinkStarted = false;
    drawingWidget->erase();
    drawingWidget->reset();

    // timer
    ElapsedTime = QTime(0,0,20);
    ui->timelabel->setText(ElapsedTime.toString("mm:ss"));
    ui->timelabel->setStyleSheet("color: black;");

    // countdown timer
    m_count = 8;
    count_timer->stop();

    // change window UI
    if (correct_num == TIME_OVER) { this->hide(); this->show(); return; }
    if (correct_num == retMyNum()) { this->hide(); g_secondWindow->show(); }
    else { this->hide(); this->show(); }
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
        if (ElapsedTime == QTime(0,0,10) && !m_blinkStarted) {
                    m_blinkStarted = true;
                    handle_device_control_request(LED_TIMER);
         }
        //qDebug() << ElapsedTime.toString("mm:ss");
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
    }
}
