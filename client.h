#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "protocol.h"
#include "secondwindow.h"
#include "thirdwindow.h"
#include "drawingdispatcher.h"
#include "mainwindow.h"
#include "playbgm.h"

void run_client(int maxPlayer);
extern void disconnect_client();
bool recv_drawpacket(int fd, DrawPacket& pkt);
int retMyNum();
void send_answer(const std::string& ans);
void send_erase();
void send_coordinate(double x, double y, int penColor, int penWidth, int drawStatus);
void send_set_true_answer(const QString& word);
void send_timeover();

class SecondWindow;
extern SecondWindow* g_secondWindow;

class ThirdWindow;
extern ThirdWindow* g_thirdWindow;

#endif // CLIENT_H
