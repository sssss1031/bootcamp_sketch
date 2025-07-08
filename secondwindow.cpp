#include "secondwindow.h"
#include "ui_secondwindow.h"
#include "touchdrawingwidget.h"
#include "drawingdispatcher.h"
#include "protocol.h"
#include "client.h"
#include "buttonmonitor.h"
#include "gpio_control.h"
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include "chatmessagedispatcher.h"

SecondWindow::SecondWindow(int maxPlayer, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SecondWindow),
    m_maxPlayer(maxPlayer),
    ElapsedTime(0,0,20),
    m_count(8),
    m_blinkStarted(false)

{
    ui->setupUi(this);
    this->setAutoFillBackground(true);

    // QFrame에 TouchDrawingWidget 생성 및 배치
    drawingWidget = new TouchDrawingWidget(ui->frame);
    drawingWidget->setGeometry(ui->frame->rect());
    drawingWidget->show();
    drawingWidget->setEnabled(true);

    ui->countdown->hide();
    // backbutton 클릭 시 backToMain 신호 발생
    connect(ui->backbutton, &QPushButton::clicked, this, &SecondWindow::backToMainRequested);

    // send Packet
    connect(&DrawingDispatcher::instance(), &DrawingDispatcher::drawArrived, this,
        [this](int drawStatus, double x, double y, int color, int thick){
            if(drawingWidget) drawingWidget->onDrawPacket(drawStatus, x, y, color, thick);
        }
    );

    connect(&ChatMessageDispatcher::instance(), &ChatMessageDispatcher::chatMessageArrived,
            this, &SecondWindow::appendChatMessage);

    // button monitoring
    auto *btnMon = new ButtonMonitor("/dev/mydev", this);
        connect(btnMon, &ButtonMonitor::buttonPressed, this, [=](int idx){
            switch(idx) {
                           case 0:
                                drawingWidget->setEraser();break;
                           case 1:
                                drawingWidget->erase();
                                send_erase();
                                qDebug() << "eraseall";
                                break;
                           case 2: drawingWidget->colorClicked(); break;
                           case 3: drawingWidget->widthUp(); break;
                           case 4: drawingWidget->widthDown(); break;
                           default : break;
                       };
        });
    
    // pen changed
    connect(drawingWidget, &TouchDrawingWidget::penChanged, this, &SecondWindow::onPenChanged);
    connect(ui->colorbutton, &QPushButton::clicked, this, [this]() {drawingWidget->colorClicked();});
    connect(ui->widthbutton, &QPushButton::clicked, this, [this]() {drawingWidget->widthClicked();});

    QIcon colorIcon(QString(":/new/prefix1/black.png"));
    ui->colorbutton->setIcon(colorIcon);
    ui->colorbutton->setIconSize(QSize(55,55));

    int btnX = centerX - 30 / 2;
    int btnY = centerY - 30 / 2;
    ui->widthbutton->setGeometry(btnX, btnY, 30, 30);
    ui->widthbutton->setIconSize(QSize(30, 30));
    ui->widthbutton->setStyleSheet(
        QString("border-radius: %1px; background: black; border: 2px solid #888;")
            .arg(30/2)
    );

    ui->buttoncover->raise();
    ui->colorbutton->raise();
    ui->widthbutton->raise();

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
    connect(timer, &QTimer::timeout, this, &SecondWindow::updateTime);
    ui->timelabel->setText(ElapsedTime.toString("mm:ss"));
    ui->timelabel->setStyleSheet("color: black;");
    // 임시 : 바로 시작, 게임 시작 시 start(1000) 호출

    count_timer = new QTimer(this);
    connect(count_timer, &QTimer::timeout, this, &SecondWindow::updateCountdown);

    qDebug() << "second";

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

void SecondWindow::updateScoreboard(const ScoreList& players)
{

    qDebug() << "updateScoreboard called, players:" << players.size();

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
//내 player 닉네임 띄우기
void SecondWindow::setMyNum(int num) {
    qDebug() << "setMyNum called, num=" << num;
    ui->label_myNum->setText(QString("I'm Player: %1").arg(num));
}

//입력창 띄우기
void SecondWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);

    updateScoreboard(g_pendingScoreList);

    QTimer::singleShot(0, this, [this]() {
            bool ok = false;
            QString word;
            while (!ok || word.isEmpty()) {

                QInputDialog inputDialog(this);
                inputDialog.setWindowTitle("INPUT Value");
                inputDialog.setLabelText("Input The Word to Draw:");
                inputDialog.setInputMode(QInputDialog::TextInput);
                inputDialog.setTextValue("Nothin");
                inputDialog.resize(400, 400);

                // 폰트 크기, 색상 등 전체 적용
                inputDialog.setStyleSheet(R"(
                    * { font-size: 30px; }
                    QLabel { color: #333366; font-weight: bold; }
                    QLineEdit { background: #f3f3fa; border: 2px solid #888; }
                    QPushButton { background: #77aaff; color: white; font-size: 36px; border-radius: 10px; min-width: 80px; min-height: 40px; }
                )");

                if (inputDialog.exec() == QDialog::Accepted) {
                    word = inputDialog.textValue();
                    ok = true;
                } else {
                    QMessageBox::warning(this, "Notice", "Input the word to start game");
                }
            }
            // === 정답 라벨에 표시 ===
            ui->label_answer->setText(QString("Your answer: %1").arg(word));
            send_set_true_answer(word);
            timer->start(1000);
        });
    }


//메시지 채팅
void SecondWindow::appendChatMessage(const QString& message) {
    qDebug() << "appendChatMessage called:" << message;
    ui->textEdit->append(message);
}

void SecondWindow::correctRound(const QString& message){
    qDebug() << "correct! msg: " << message;

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

void SecondWindow::timeoverRound(){
    qDebug() << "time over";

    drawingWidget->setEnabled(false); //그림 안그려지게

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

void SecondWindow::nextRound(int correct_num)
{
    m_blinkStarted = false;
    drawingWidget->setEnabled(true); //그림 그려지게 활성화
    // widget
    drawingWidget->erase();
    drawingWidget->reset();

    resetPenButtons();

    // timer
    ElapsedTime = QTime(0,0,20);
    ui->timelabel->setText(ElapsedTime.toString("mm:ss"));
    ui->timelabel->setStyleSheet("color: black;");

    // countdown timer
    m_count = 8;
    count_timer->stop();

    // change window UI
    if (correct_num == TIME_OVER) { this->hide(); this->show(); return; }
    if (correct_num == retMyNum()) { this->hide(); this->show(); }
    else { this->hide(); g_thirdWindow->show(); }
}

void SecondWindow::onPenChanged(int color, int width)
{
    QString colorImg;
        switch (color) {
            case 1: colorImg = ":/new/prefix1/black.png"; break;
            case 2: colorImg = ":/new/prefix1/yellow.png"; break;
            case 3: colorImg = ":/new/prefix1/red.png"; break;
            case 4: colorImg = ":/new/prefix1/green.png"; break;
            case 5: colorImg = ":/new/prefix1/blue.png"; break;
            case 6: colorImg = ":/new/prefix1/white.png"; break;
            case 7: colorImg = ":/new/prefix1/eraser.png"; break;
            default: colorImg = ":/new/prefix1/black.png"; break;
        }
    
    int qw;
    switch (width) {
                case 10: qw = 15; break;
                case 11: qw = 30; break;
                case 12: qw = 45; break;
    }
    qDebug() << qw;
    QIcon colorIcon(colorImg);
    ui->colorbutton->setIcon(colorIcon);
    ui->colorbutton->setIconSize(QSize(55,55));

    int btnX = centerX - qw / 2;
    int btnY = centerY - qw / 2;
    ui->widthbutton->setGeometry(btnX, btnY, qw, qw);

    ui->widthbutton->setIconSize(QSize(qw, qw));
    ui->widthbutton->setStyleSheet(
        QString("border-radius: %1px; background: black; border: 2px solid #888;")
            .arg(qw/2)
    );

    // countdown timer
    m_count = 8;
    count_timer->stop();
}


void SecondWindow::resetPenButtons()
{
    // 색상 버튼(초기: 검정)
    QIcon colorIcon(QString(":/new/prefix1/black.png"));
    ui->colorbutton->setIcon(colorIcon);
    ui->colorbutton->setIconSize(QSize(55,55));

    // 굵기 버튼(초기: 15)
    int qw = 15;
    int btnX = centerX - qw / 2;
    int btnY = centerY - qw / 2;
    ui->widthbutton->setGeometry(btnX, btnY, qw, qw);
    ui->widthbutton->setIconSize(QSize(qw, qw));
    ui->widthbutton->setStyleSheet(
        QString("border-radius: %1px; background: black; border: 2px solid #888;")
            .arg(qw/2)
    );
}

void SecondWindow::updateTime()
{
    if (ElapsedTime > QTime(0, 0, 0))
    {
        ElapsedTime = ElapsedTime.addSecs(-1);
        ui->timelabel->setText(ElapsedTime.toString("mm:ss"));
        qDebug() << "secondWindow address:" << this;
        if (ElapsedTime < QTime(0,0,31))
        {
            ui->timelabel->setStyleSheet("color: red;");
        }
        if (ElapsedTime == QTime(0,0,10) && !m_blinkStarted) {
                    m_blinkStarted = true;
                    handle_device_control_request(LED_TIMER);
         }

    }
    else {
        timeoverRound();
    }
}

void SecondWindow::updateCountdown()
{

    if (m_count > 0)
    {
        m_count--;
        ui->countdown->setText("NEXT ROUND STARTS IN: " + (QString::number(m_count)) + " Secs..");
        //qDebug() << "m_count:" << m_count;
    }
}

