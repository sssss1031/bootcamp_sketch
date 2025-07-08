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
    m_blinkStarted (false), // led timer blink
    dotCount(0),
    round_start(false),
    onBlink(false) // screen timer blink
{
    ui->setupUi(this);
    this->setAutoFillBackground(true);

    // QFrame에 TouchDrawingWidget 생성 및 배치
    drawingWidget = new TouchDrawingWidget(ui->frame);
    drawingWidget->setGeometry(ui->frame->rect());
    drawingWidget->show();
    drawingWidget->setEnabled(false);
    ui->countdown->hide();
    ui->waiting->raise();

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

    // waiting timer
    waiting_timer = new QTimer(this);
    connect(waiting_timer, &QTimer::timeout, this, &ThirdWindow::updateWaiting);
    waiting_timer->start(1000);

    // blink timer
    blink_timer = nullptr;

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

void ThirdWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    QTimer::singleShot(0, this, [this]() {
            ui->waiting->show();
            updateScoreboard(g_pendingScoreList);
    });
}


void ThirdWindow::onBeginRound()
{
    round_start = true;
    ui->waiting->hide();
    qDebug()<<"onBeginRound";
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

void ThirdWindow::setMyNum(int num) {
    qDebug() << "setMyNum called, num=" << num;
    ui->label_myNum->setText(QString("I'm Player: %1").arg(num));
}

void ThirdWindow::timeoverRound(){
    qDebug() << "time over";

    // 스타일 적용
    ui->timeover->setStyleSheet(R"(
        QLabel {
            color: #fff;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #ff5f6d, stop:1 #ffc371);
            border: 3px solid #fff;
            border-radius: 30px;
            padding: 30px;
            font-size: 32px;
            font-weight: bold;
            qproperty-alignment: AlignCenter;
        }
        )");

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

void ThirdWindow::showTimeOverAnswer(const QString& answer) {
    qDebug() << "Time over, 정답:" << answer;
    ui->timeover->setText("Time Over!\n정답: " + answer);
    ui->timeover->raise();
    ui->timeover->show();
    timer->stop();

    // 스타일 적용
    ui->countdown->setStyleSheet(R"(
        QLabel {
            color: #333366;
            background: rgba(255,255,255,0.85);
            border: 2px solid #77aaff;
            border-radius: 20px;
            padding: 20px 40px;
            font-size: 30px;
            font-weight: 600;
            letter-spacing: 2px;
            qproperty-alignment: AlignCenter;
        }
    )");

    count_timer->start(1000);
    ui->countdown->setText("NEXT ROUND STARTS IN: " + (QString::number(m_count)) + " Secs..");
    ui->countdown->raise();
    ui->countdown->show();

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
        qDebug() << "ThirdWindow address:" << this;
        if (ElapsedTime < QTime(0,0,31))
        {
            ui->timelabel->setStyleSheet("color: red;");
        }        
        if (ElapsedTime <= QTime(0,0,10))
        {
            if(!blink_timer){
                // blink timer
                blink_timer = new QTimer(this);
                connect(blink_timer, &QTimer::timeout, this, &ThirdWindow::updateBlink);
                blink_timer->start(250);
            }
        }
        if (ElapsedTime == QTime(0,0,10) && !m_blinkStarted) {
                    m_blinkStarted = true;
                    handle_device_control_request(LED_TIMER);
         }
        if (!timer->isActive())
        {
            if(blink_timer){
                blink_timer->stop();
                blink_timer->deleteLater();
                blink_timer = nullptr;
                ui->timelabel->setStyleSheet("color: red;");
            }
        }
        //qDebug() << ElapsedTime.toString("mm:ss");
    }
    else {
        blink_timer->stop();
        blink_timer->deleteLater();
        blink_timer = nullptr;
        ui->timelabel->setStyleSheet("color: red;");
        timeoverRound();
    }
}

void ThirdWindow::updateBlink()
{
    onBlink = !onBlink;
    if (onBlink)
    {
        ui->timelabel->setStyleSheet("color: red;");
    }
    else
    {
        ui->timelabel->setStyleSheet("color: transparent;");
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

void ThirdWindow::updateWaiting()
{
    dotCount = ((dotCount + 1) % 4 );
    QString dots(dotCount, '.');
    ui->waiting->setText("Your opponent is thinking" + dots);
}
