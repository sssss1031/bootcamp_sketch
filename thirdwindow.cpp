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
#include <QRegularExpression>

ThirdWindow::ThirdWindow(int maxPlayer, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ThirdWindow),
    m_maxPlayer(maxPlayer),
    ElapsedTime(0,0,30),
    m_count(5),
    m_blinkStarted (false), // led timer blink
    dotCount(0),
    round_start(false),
    onBlink(false), // screen timer blink
    hintFrame(nullptr),
    hintLabel(nullptr),
    touchLabel(nullptr),
    current_round(1)
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
    ui->resultwidget->hide();

    hintFrame = ui->hintFrame;
    hintLabel = ui->hintLabel;
    touchLabel = ui->touchLabel;
    hintFrame->setVisible(false);
    hintLabel->setVisible(false);
    touchLabel->setVisible(true);

    ui->hintFrame->installEventFilter(this);
    //ui->hintLabel->installEventFilter(this);
    //ui->touchLabel->installEventFilter(this);

    ui->waiting->move(70, 220);
    ui->waiting->resize(871, 200);

    ui->waiting->setStyleSheet(R"(
        QLabel {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                       stop:0 #c9e7ff, stop:1 #e0ecff);
            color: #223366;
            border: 4px solid #7dcfff;
            border-radius: 32px;
            padding: 24px 40px;
            font-size: 32px;
            font-weight: 700;
            letter-spacing: 2px;
            background-color: rgba(255,255,255,0.92);
        }
    )");

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

    ui->roundboard->setText(QString::number(current_round)
                            + "/" + QString::number(MAX_ROUND));
    ui->roundboard->raise();
    ui->roundboard->show();

    //run_client();
    run_client(m_maxPlayer); // maxPlayer 인자 전달
    qDebug() << "third";

}

ThirdWindow::~ThirdWindow()
{
    delete ui;
}

void ThirdWindow::backToMainRequested() {
    current_round=1;
    g_secondWindow->roundinit();
    ui->textEdit->clear();
    disconnect_client();  // 연결 해제
    emit backToMain();  // 메인 윈도우로 돌아가기
}

// 리사이즈 이벤트에서 drawingWidget 크기 자동조정
void ThirdWindow::resizeEvent(QResizeEvent *event)
{
    drawingWidget->setAttribute(Qt::WA_TranslucentBackground, true);
    drawingWidget->setStyleSheet("background: transparent; border: none;");

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

        QRect boardRect = ui->frame->rect();
        double widthRatio = 0.94;
        double heightRatio = 0.90;

        int newWidth = static_cast<int>(boardRect.width() * widthRatio);
        int newHeight = static_cast<int>(boardRect.height() * heightRatio);

        int newX = (boardRect.width() - newWidth) / 2;
        int newY = (boardRect.height() - newHeight) / 2;

        drawingWidget->setGeometry(newX, newY, newWidth, newHeight);
    }
}

void ThirdWindow::onLineEditReturnPressed()
{
    QString text = ui->lineEdit->text();
    if (!text.isEmpty()) {
        qDebug() << "Answer:" << text;
        send_answer(text.toStdString());
            // 추가 동작 필요 시 여기에 작성
        ui->lineEdit->clear();
    }
}

void ThirdWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    drawingWidget->erase();
    drawingWidget->reset();
    QTimer::singleShot(0, this, [this]() {

            ui->waiting->show();
            ui->hintFrame->hide();
            updateScoreboard(g_pendingScoreList);

            ui->roundboard->setText(QString::number(current_round)
                                    + "/" + QString::number(MAX_ROUND));
    });
}

QString ThirdWindow::makeHint(const QString& answer) const {
    if (answer.isEmpty()) return "";
    QString hint = answer.left(1);
    for (int i = 1; i < answer.size(); ++i)
        hint += " _";
    return hint;
}

void ThirdWindow::showHint(const QString& answer)
{
    QString hint = makeHint(answer);
    hintLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QFontMetrics fm(hintLabel->font());
    int textWidth = fm.horizontalAdvance(hint);
    int frameWidth = textWidth + 40;
    int frameHeight = fm.height() + 20;
    QRect origGeo = hintFrame->geometry();
    int x = origGeo.x();
    int y = origGeo.y();

    hintLabel->setMinimumWidth(frameWidth);
    hintLabel->setMinimumHeight(frameHeight);
    hintFrame->raise();

    hintLabel->setVisible(true);
    touchLabel->setVisible(false);
    hintFrame->setVisible(true);
}

bool ThirdWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->hintFrame || obj == ui->hintLabel || obj == ui->touchLabel) {
           if (ElapsedTime > QTime(0,0,10)) {
               if (event->type() == QEvent::MouseButtonPress) {
                   showHint(m_answerStr);
                   return true;
               }
               if (event->type() == QEvent::MouseButtonRelease) {
                    hideHint();
                    return true;
               }
           } else {
                    showHint(m_answerStr);
                    return true;
               }
          }
   return QMainWindow::eventFilter(obj, event);
}

void ThirdWindow::hideHint()
{
    if (ElapsedTime <= QTime(0,0,10)) return;
    hintLabel->setVisible(false);
    touchLabel->setVisible(true);
    hintFrame->setVisible(true);
}

void ThirdWindow::onBeginRound(const QString& answer)
{
    round_start = true;
    ui->lineEdit->setEnabled(true);
    ui->enterButton->setEnabled(true);

    ui->waiting->hide();
    qDebug()<<"onBeginRound";
    timer->start(1000);
    ui->hintFrame->show();

    m_answerStr = answer;
    QString hint = makeHint(answer);
    hintLabel->setText(hint);

}

//메시지 채팅
void ThirdWindow::appendChatMessage(const QString& message) {
    qDebug() << "appendChatMessage called:" << message;
    ui->textEdit->append(message);
}

void ThirdWindow::correctRound(const QString& message){
    ui->lineEdit->setEnabled(false);
    ui->enterButton->setEnabled(false);

    if(blink_timer){

        blink_timer->stop();
        blink_timer->deleteLater();
        blink_timer = nullptr;
        ui->timelabel->setStyleSheet("color: red;");
    }
    // CORRECT : change questioner
    int correct_num = message.mid(16,1).toInt();
    int colon = message.indexOf(':');

    ui->correct->resize(590, 280);
    ui->correct->move(140, 50);

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

    ui->correct->setText("Correct! : Player " + QString::number(correct_num) + "\nANSWER : " +
                         message.mid(colon + 1).trimmed());
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
    QTimer::singleShot(6000, this, [=](){
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

         // playerX에서 숫자만 추출
         QString playerName = players[i].first;
         QRegularExpression re("\\d+");
         QRegularExpressionMatch match = re.match(playerName);
         QString numberStr = match.hasMatch() ? match.captured() : QString::number(i+1);

         // "P" + 숫자 형태로 표시
         nameLabels[idx]->setText("P" + numberStr);
         scoreLabels[idx]->setText(QString::number(players[i].second));
         nameLabels[idx]->setVisible(true);
         scoreLabels[idx]->setVisible(true);
     }

         ui->scoreboard->update();
}

void ThirdWindow::updateResultboard(const ScoreList& players)
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

void ThirdWindow::setMyNum(int num) {

    ui->label_myNum->setText(QString("I'm Player: %1").arg(num));
}

void ThirdWindow::showTimeOverAnswer(const QString& answer) {
    qDebug() << "Time over, 정답:" << answer;
    drawingWidget->setEnabled(false);
    ui->lineEdit->setEnabled(false);
    ui->enterButton->setEnabled(false);

    if(blink_timer){

        blink_timer->stop();
        blink_timer->deleteLater();
        blink_timer = nullptr;
        ui->timelabel->setStyleSheet("color: red;");
    }
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

    if(blink_timer){

        blink_timer->stop();
        blink_timer->deleteLater();
        blink_timer = nullptr;
        ui->timelabel->setStyleSheet("color: red;");
    }

    ui->timeover->resize(590, 270);
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
            box-shadow: 0px 8px 24px rgba(0,0,0,0.4);
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
    if (current_round == MAX_ROUND) {
        QTimer::singleShot(5000, this, [=](){
            updateResultboard(g_pendingScoreList);
        });
        return; }
    ui->countdown->setText("NEXT ROUND STARTS IN: " + (QString::number(m_count)) + " Secs..");
    ui->countdown->raise();
    ui->countdown->show();

    QTimer::singleShot(6000, this, [=](){
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

    hintFrame->setVisible(false);
    hintLabel->setVisible(false);
    touchLabel->setVisible(true);

    ui->lineEdit->clear();

    // timer
    ElapsedTime = QTime(0,0,30);
    ui->timelabel->setText(ElapsedTime.toString("mm:ss"));
    ui->timelabel->setStyleSheet("color: black;");

    // countdown timer
    m_count = 5;
    count_timer->stop();

    // count round
    current_round += 1;
    g_secondWindow->roundinc();
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
        if (ElapsedTime < QTime(0,0,21))
        {
            ui->timelabel->setStyleSheet("color: red;");
        }        
        if (ElapsedTime <= QTime(0,0,10))
        {          
            if (ElapsedTime == QTime(0, 0, 10))
            {
                PlayBgm::playOnce(PlayBgm::TIMER);
                showHint(m_answerStr);
            }

            if(!m_blinkStarted)
            {
                m_blinkStarted = true;
                handle_device_control_request(LED_TIMER);
            }

            if(!blink_timer){
                // blink timer
                blink_timer = new QTimer(this);
                connect(blink_timer, &QTimer::timeout, this, &ThirdWindow::updateBlink);
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
        //timeoverRound();
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
    ui->waiting->setText("Drawer is thinking" + dots);
}


void ThirdWindow::roundinc()
{
    current_round += 1;
}

void ThirdWindow::roundinit()
{
    ui->textEdit->clear();
    current_round = 1;
}
