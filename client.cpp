#include "client.h"
#include "gpio_control.h"
#include "secondwindow.h"
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

#include "touchdrawingwidget.h"

int sockfd;
int my_Num = 0;


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

void recv_thread(int sockfd) {

    while (true) {
        int msg_type = 0;
        ssize_t n = recv(sockfd, &msg_type, sizeof(int), MSG_PEEK);
        if (n <= 0) break;

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
            QMetaObject::invokeMethod(g_secondWindow, "appendChatMessage", Qt::QueuedConnection, Q_ARG(QString, qmsg));
            handle_device_control_request(LED_CORRECT);
        } else if (msg_type == MSG_WRONG) {
            CommonPacket pkt;
            if (!recv_commonpacket(sockfd, pkt)) break;
            std::cout << "[Wrong] " << pkt.message << std::endl;
            std::cout << pkt.message << std::endl;
            QString qmsg = QString("[Wrong] %1's Answer : %2").arg(QString::fromStdString(pkt.nickname)).arg(QString::fromStdString(pkt.message));
            qDebug() << "g_secondWindow is" << (g_secondWindow == nullptr ? "nullptr" : "valid");
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
            if (!recv_playerCntpacket(sockfd, pkt)) break;
            if(pkt.currentPlayer_cnt > pkt.maxPlayer){
                std::cout << "Out of capacity - " << "Cur Players : " << pkt.currentPlayer_cnt <<" Max Players : " << pkt.maxPlayer << std::endl;
                if (g_secondWindow) {
                            QMetaObject::invokeMethod(g_secondWindow, "backToMainRequested", Qt::QueuedConnection);
                        }
                break;
            }
            //TODO: handle player count
            std::cout << "Cur Players : " << pkt.currentPlayer_cnt <<" Max Players : " << pkt.maxPlayer << std::endl;

         } else {
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
    }
}
