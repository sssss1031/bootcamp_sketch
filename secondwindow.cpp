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
    m_blinkStarted(false), //timer led blink
    onBlink(false), // screen timer blink
    current_round(1)
{
    ui->setupUi(this);
    this->setAutoFillBackground(true);

    // QFrame에 TouchDrawingWidget 생성 및 배치
    drawingWidget = new TouchDrawingWidget(ui->frame);
    //drawingWidget->setGeometry(ui->frame->rect());
    drawingWidget->show();
    drawingWidget->setEnabled(true);

    ui->countdown->hide();
    ui->resultwidget->hide();
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

    ui->countdown->setStyleSheet(R"(
            QLabel {
                color: #333366;
                background: rgba(255,255,255,0.85);
                border: 2px solid #77aaff;
                border-radius: 20px;
                padding: 20px 40px;
                font-size: 32px;
                font-weight: 600;
                letter-spacing: 2px;
                qproperty-alignment: AlignCenter;
            }
        )");

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

    // blink timer
    blink_timer = nullptr;

    ui->roundboard->setText(QString::number(current_round)
                            + "/" + QString::number(MAX_ROUND));
    ui->roundboard->raise();
    ui->roundboard->show();

    qDebug() << "second";

    ui->correct->setStyleSheet(R"(
        QLabel {
            color: #fff;
           background: qlineargradient(
                       x1:0, y1:0, x2:1, y2:1,
                       stop:0 rgba(79,214,114,0.85),   /* #4fd672 with alpha */
                       stop:1 rgba(34,153,119,0.85)    /* #229977 with alpha */
                   );
            border: 5px solid #3fa65b;
            border-radius: 36px;
            padding: 48px;
            font-size: 40px;
            font-weight: bold;
            qproperty-alignment: AlignCenter;
            box-shadow: 0px 8px 24px rgba(0,0,0,0.4);
        }ui->correct
    )");

}

SecondWindow::~SecondWindow()
{
    delete ui;
}

void SecondWindow::backToMainRequested() {
    current_round=1;
    g_thirdWindow->roundinit();
    disconnect_client();  // 연결 해제
    emit backToMain();  // 메인 윈도우로 돌아가기
}

// 리사이즈 이벤트에서 drawingWidget 크기 자동조정
void SecondWindow::resizeEvent(QResizeEvent *event)
{
    drawingWidget->setAttribute(Qt::WA_TranslucentBackground, true);
    drawingWidget->setStyleSheet("background: transparent; border: none;");

    // 배경이미지 설정 (기존 코드 유지)
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

        // 2. frame 영역에서 border만큼 안쪽에 위치하도록 사각형 생성
        QRect boardRect = ui->frame->rect();
        // 원하는 비율 (예: 가로 90%, 세로 60%)
        double widthRatio = 0.94;
        double heightRatio = 0.90;

        int newWidth = static_cast<int>(boardRect.width() * widthRatio);
        int newHeight = static_cast<int>(boardRect.height() * heightRatio);

        int newX = (boardRect.width() - newWidth) / 2;
        int newY = (boardRect.height() - newHeight) / 2;

        drawingWidget->setGeometry(newX, newY, newWidth, newHeight);
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


void SecondWindow::updateResultboard(const ScoreList& players)
{
    QLabel* resultLabels[3] = {ui->result_1, ui->result_2, ui->result_3};

    for (int i = 0; i < 3; ++i) {
       resultLabels[i]->clear();
       resultLabels[i]->setVisible(false);
    }

    int n = players.size();
    for (int i = 0; i < n && i < 3; ++i) {
       const QString& name = players[i].first;
       int score = players[i].second;
       resultLabels[i]->setText(QString("%1 : %2").arg(name).arg(score));
       resultLabels[i]->setVisible(true);
    }
    ui->resultwidget->raise();
    ui->resultwidget->show();
    ui->correct->hide();
    ui->timeover->hide();
    ui->countdown->hide();

}

//내 player 닉네임 띄우기
void SecondWindow::setMyNum(int num) {
    qDebug() << "setMyNum called, num=" << num;
    ui->label_myNum->setText(QString("I'm Player: %1").arg(num));
}

//입력창 띄우기
void SecondWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    drawingWidget->erase();
    drawingWidget->reset();
    updateScoreboard(g_pendingScoreList);
    ui->roundboard->setText(QString::number(current_round)
                            + "/" + QString::number(MAX_ROUND));

    QTimer::singleShot(0, this, [this]() {
            bool ok = false;
            QString word;
            while (!ok || word.isEmpty()) {

                QInputDialog inputDialog(this);
                inputDialog.setWindowTitle("INPUT Value");
                inputDialog.setLabelText("Input The Word to Draw:");
                inputDialog.setInputMode(QInputDialog::TextInput);
                inputDialog.setTextValue("Nothing");
                inputDialog.resize(460, 420);
                inputDialog.move(240, 200);
                
                // 폰트 크기, 색상 등 전체 적용
                inputDialog.setStyleSheet(R"(
                    QDialog {
                        background: rgba(255,255,255,0.85);
                        border: 4px solid #9ec9ff;
                        border-radius: 22px;
                    }
                    * { font-size: 24px; font-family: 'Segoe UI', 'Pretendard', 'Noto Sans KR', sans-serif; }
                    QLabel {
                        color: #224488;
                        font-weight: 600;
                        padding-bottom: 10px;
                    }
                    QLineEdit {
                        background: #fff;
                        border: 2px solid #aac9e6;
                        border-radius: 10px;
                        padding: 10px 16px;
                        font-size: 26px;
                        color: #223355;
                    }
                    QPushButton {
                        background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #6db3fa, stop:1 #1e69de);
                        color: white;
                        font-size: 24px;
                        border-radius: 14px;
                        min-width: 90px;
                        min-height: 44px;
                        font-weight: bold;
                        padding: 8px 28px;
                        margin: 8px 16px 0 0;
                    }
                    QPushButton:pressed {
                        background: #184c99;
                    }
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


    if(blink_timer){

        blink_timer->stop();
        blink_timer->deleteLater();
        blink_timer = nullptr;
        ui->timelabel->setStyleSheet("color: red;");
    }

    int correct_num = message.mid(16,1).toInt();
    int colon = message.indexOf(':');


    ui->correct->setText("correct! : Player " + QString::number(correct_num) + "\nANSWER : " +
                         message.mid(colon + 1).trimmed());

    ui->correct->resize(590, 280);
    ui->correct->move(140, 50);

    ui->correct->raise();
    ui->correct->show();
    timer->stop();

    count_timer->start(1000);
    if (current_round == MAX_ROUND) {
        QTimer::singleShot(5000, this, [=](){
            updateResultboard(g_pendingScoreList);
        });
        return; }
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


void SecondWindow::showTimeOverAnswer(const QString& answer) {
    // 기존 timeoverRound에서 정답을 받아 표시하도록 수정
    qDebug() << "Time over, 정답:" << answer;
    if(blink_timer){

        blink_timer->stop();
        blink_timer->deleteLater();
        blink_timer = nullptr;
        ui->timelabel->setStyleSheet("color: red;");
    }
    drawingWidget->setEnabled(false);

    ui->timeover->resize(590, 280);
    ui->timeover->move(170, 50);

    ui->timeover->setStyleSheet(R"(
        QLabel {
            color: #fff;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(255, 95, 109, 0.7),  stop:1 rgba(255, 195, 113, 0.7) );
            border: 6px solid #444;
            border-radius: 36px;
            padding: 48px;
            font-size: 48px;
            font-weight: bold;
            qproperty-alignment: AlignCenter;
        }
        )");

            ui->timeover->setText(
                QString("<span style='font-size:48px;'>Time Over!</span><br>"
                        "<span style='font-size:36px;'>Answer is.. <b>%1</b></span>")
                        .arg(answer)
            ); // 정답 표시
    ui->timeover->raise();
    ui->timeover->show();
    timer->stop();

    count_timer->start(1000);
    if (current_round == MAX_ROUND) {
        QTimer::singleShot(5000, this, [=](){
            updateResultboard(g_pendingScoreList);
        });
        return; }
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

    // count round
    current_round += 1;
    g_thirdWindow->roundinc();
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
                case 13: qw = 55; break;
    }

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

    // 굵기 버튼(초기: 30)
    int qw = 30;
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
        if (ElapsedTime < QTime(0,0,31))
        {
            ui->timelabel->setStyleSheet("color: red;");
        }
        if (ElapsedTime <= QTime(0,0,10))
        {
            if (ElapsedTime == QTime(0, 0, 10))
            {
                PlayBgm::playOnce(PlayBgm::TIMER);
            }

            if(!m_blinkStarted)
            {
                m_blinkStarted = true;
                handle_device_control_request(LED_TIMER);
            }

            if(!blink_timer){
                // blink timer
                blink_timer = new QTimer(this);
                connect(blink_timer, &QTimer::timeout, this, &SecondWindow::updateBlink);
                blink_timer->start(250);
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
        }

        //qDebug() << ElapsedTime.toString("mm:ss");
    }
    else {
        blink_timer->stop();
        blink_timer->deleteLater();
        blink_timer = nullptr;
        ui->timelabel->setStyleSheet("color: red;");

        send_timeover();
    }
}

void SecondWindow::updateBlink()
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

void SecondWindow::updateCountdown()
{

    if (m_count > 0)
    {
        m_count--;
        ui->countdown->setText("NEXT ROUND STARTS IN: " + (QString::number(m_count)) + " Secs..");
        //qDebug() << "m_count:" << m_count;
    }
}

void SecondWindow::roundinc()
{
    current_round += 1;
}

void SecondWindow::roundinit()
{
    current_round = 1;
}
