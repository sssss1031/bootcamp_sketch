#include "client.h"
#include "gpio_control.h"
#include "secondwindow.h"
#include "thirdwindow.h"
#include <iostream>
#include <thread>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <QDebug>
#include "secondwindow.h"
#include <QMessageBox>
#include "touchdrawingwidget.h"
#include "playercountdispatcher.h"

int sockfd = -1;
int my_Num = 0;

int desiredMaxPlayer = 2; // 기본값(2p)

extern MainWindow* g_mainWindow;

void send_string(int fd, const std::string& s) {
    uint32_t len = s.size();
    send(fd, &len, sizeof(len), 0);
    if (len > 0) send(fd, s.data(), len, 0);
}
std::string recv_string(int fd) {
    uint32_t len = 0;
    recv(fd, &len, sizeof(len), MSG_WAITALL);
    std::string s;
    if (len > 0) {
        s.resize(len);
        recv(fd, &s[0], len, MSG_WAITALL);
    }
    return s;
}

bool recv_playerpacket(int fd, PlayerNumPacket& pkt) {
    return recv(fd, &pkt, sizeof(pkt), MSG_WAITALL) == sizeof(pkt);
}

bool recv_playerCntpacket(int fd, PlayerCntPacket& pkt) {
    return recv(fd, &pkt, sizeof(pkt), MSG_WAITALL) == sizeof(pkt);
}

void send_drawpacket(int fd, const DrawPacket& pkt) {
    send(fd, &pkt, sizeof(pkt), 0);
}

bool recv_drawpacket(int fd, DrawPacket& pkt) {
    return recv(fd, &pkt, sizeof(pkt), MSG_WAITALL) == sizeof(pkt);
}

void send_answerpacket(int fd, const AnswerPacket& pkt) {
    send(fd, &pkt.type, sizeof(pkt.type), 0);
    send_string(fd, pkt.nickname);
    send_string(fd, pkt.answer);
}

void send_correctpacket(int fd, const CorrectPacket& pkt) {
    send(fd, &pkt.type, sizeof(pkt.type), 0);
    send_string(fd, pkt.nickname);
}

bool recv_correctpacket(int fd, CorrectPacket& pkt) {
    int header;
    if (recv(fd, &header, sizeof(header), MSG_WAITALL) != sizeof(header)) return false;
    pkt.type = header;
    pkt.nickname = recv_string(fd);
    return true;
}

void send_wrongpacket(int fd, const WrongPacket& pkt) {
    send(fd, &pkt.type, sizeof(pkt.type), 0);
    send_string(fd, pkt.message);
}

bool recv_wrongpacket(int fd, WrongPacket& pkt) {
    int header;
    if (recv(fd, &header, sizeof(header), MSG_WAITALL) != sizeof(header)) return false;
    pkt.type = header;
    pkt.nickname = recv_string(fd);
    pkt.message = recv_string(fd);
    return true;
}

bool recv_commonpacket(int fd, CommonPacket& pkt) {
    int header;
    if (recv(fd, &header, sizeof(header), MSG_WAITALL) != sizeof(header)) return false;
    pkt.type = header;
    pkt.nickname = recv_string(fd);
    pkt.message = recv_string(fd);
    return true;
}


int retMyNum() {
    //return 0 means connection error
    return my_Num;
}

std::atomic<bool> stop_draw{false};
std::atomic<bool> isRejected{false};

void recv_thread(int sockfd) {

    bool serverDisconnected = false;

    while (true) {
        int msg_type = 0;
        ssize_t n = recv(sockfd, &msg_type, sizeof(int), MSG_PEEK);
        if (n <= 0) {
            // 서버가 MSG_REJECTED 없이 정상적으로 끊은 경우 메시지박스 X
            if (isRejected) {
                //QMetaObject::invokeMethod(g_mainWindow, "showConnectionRejectedMessage", Qt::QueuedConnection);
break;
}
        }
        if (msg_type == MSG_DRAW) {
            DrawPacket pkt;
            if (!recv_drawpacket(sockfd, pkt)) break;
            QMetaObject::invokeMethod(
                    &DrawingDispatcher::instance(),
                    [pkt](){
                        emit DrawingDispatcher::instance().drawArrived(pkt.drawStatus, pkt.x, pkt.y, pkt.color, pkt.thick);
                    },
                    Qt::QueuedConnection
                );
        } else if (msg_type == MSG_CORRECT) {
            CommonPacket pkt;
            if (!recv_commonpacket(sockfd, pkt)) break;
            std::cout << "[Correct] " << pkt.nickname << "Player Correct!\n";
            QString qmsg = QString("[Correct] %1's Answer : %2").arg(QString::fromStdString(pkt.nickname)).arg(QString::fromStdString(pkt.message));
            if (g_secondWindow){
                QMetaObject::invokeMethod(g_secondWindow, "appendChatMessage", Qt::QueuedConnection, Q_ARG(QString, qmsg));
                QMetaObject::invokeMethod(g_secondWindow, "endRound", Qt::QueuedConnection, Q_ARG(QString, qmsg));
            }
            handle_device_control_request(LED_CORRECT);
        } else if (msg_type == MSG_WRONG) {
            CommonPacket pkt;
            if (!recv_commonpacket(sockfd, pkt)) break;
            std::cout << "[Wrong] " << pkt.message << std::endl;
            std::cout << pkt.message << std::endl;
            QString qmsg = QString("[Wrong] %1's Answer : %2").arg(QString::fromStdString(pkt.nickname)).arg(QString::fromStdString(pkt.message));
            qDebug() << "g_secondWindow is" << (g_secondWindow == nullptr ? "nullptr" : "valid");
            if (g_secondWindow)
                QMetaObject::invokeMethod(g_secondWindow, "appendChatMessage", Qt::QueuedConnection, Q_ARG(QString, qmsg));
            handle_device_control_request(LED_WRONG);

        } else if (msg_type == MSG_PLAYER_NUM) {
            PlayerNumPacket pkt;
            if (!recv_playerpacket(sockfd, pkt)) break;
            std::cout << "You are player" << pkt.player_num << std::endl;
            my_Num = pkt.player_num;

        }
        else if (msg_type == MSG_PLAYER_CNT) {
            PlayerCntPacket pkt;
            if(!recv_playerCntpacket(sockfd, pkt)) break;

            // 서버에서 받은 maxPlayer와 내가 원하는 값이 다르면,
                // 메시지박스 띄우고, disconnect 후 recv_thread 종료
            if (serverDisconnected) {
                    break;
                }

            bool ok = QMetaObject::invokeMethod(
                &PlayerCountDispatcher::instance(), "playerCountUpdated",
                Qt::QueuedConnection,
                Q_ARG(int, pkt.currentPlayer_cnt),
                Q_ARG(int, pkt.maxPlayer)
            );
            qDebug() << "[invokeMethod] playerCountUpdated ok?" << ok;
            if(pkt.currentPlayer_cnt > pkt.maxPlayer){
                std::cout << "Out of capacity - " << "Cur Players : " << pkt.currentPlayer_cnt <<" Max Players : " << pkt.maxPlayer << std::endl;
                if (g_secondWindow) {
                            QMetaObject::invokeMethod(g_secondWindow, "backToMainRequested", Qt::QueuedConnection);
                        }
                break;
            }
            //TODO: handle player count
            std::cout << "Cur Players : " << pkt.currentPlayer_cnt <<" Max Players : " << pkt.maxPlayer << std::endl;

         }
        else if (msg_type == MSG_REJECTED) {
    int dummy;
    recv(sockfd, &dummy, sizeof(dummy), 0); // consume
    isRejected = true;
    QMetaObject::invokeMethod(g_mainWindow, "showConnectionRejectedMessage", Qt::QueuedConnection);
    disconnect_client();
    break;
    }else if (msg_type == MSG_SELECTED_PLAYER) {
            int header;
            if (recv(sockfd, &header, sizeof(header), MSG_WAITALL) != sizeof(header)) break;
            std::string selected_nickname = recv_string(sockfd);
            std::cout << "[Selected Player] " << selected_nickname << " is selected!\n";
            // Qt UI에 전달 (MainWindow 슬롯 호출)
            QMetaObject::invokeMethod(
                g_mainWindow,
                "onSelectedPlayerNickname",
                Qt::QueuedConnection,
                Q_ARG(QString, QString::fromStdString(selected_nickname))
            );
        }	 else {
            char buf[256];
            recv(sockfd, buf, sizeof(buf), 0);
        }
    }
    std::cout << "Sever Disconnected\n";

}

void send_coordinate(double x, double y, int penColor, int penWidth, int drawStatus) {
    DrawPacket pkt{};
    pkt.type = MSG_DRAW;
    pkt.x = x; pkt.y = y; pkt.color = penColor; pkt.thick = penWidth; pkt.drawStatus = drawStatus;
    send_drawpacket(sockfd, pkt);
}

void send_answer(const std::string& ans){

    AnswerPacket apkt{};
    apkt.type = MSG_ANSWER;
    apkt.nickname = ""; // 서버에서 부여
    apkt.answer = ans;
    send_answerpacket(sockfd, apkt);
    std::cout << "[Send answer] : " << apkt.answer << std::endl;
}

void run_client(int maxPlayer) {
    if (sockfd > 0) {
            //qDebug() << "Already connected!";
            return;
        }
    desiredMaxPlayer = maxPlayer;
    qDebug() << "run_client called, player count =" << maxPlayer;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect"); exit(1);
    }

    // 서버에 maxPlayer 값 전송
        int msgType = 9999;
        send(sockfd, &msgType, sizeof(msgType), 0);
        send(sockfd, &maxPlayer, sizeof(maxPlayer), 0);

    std::thread(recv_thread, sockfd).detach();
}

void disconnect_client() {
    if (sockfd >= 0) {
      int msg = MSG_DISCONNECT;
      send(sockfd, &msg, sizeof(msg), 0);
      qDebug() << "Disconnected";
      close(sockfd);
      sockfd = -1;
    }
}
