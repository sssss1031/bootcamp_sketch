#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "protocol.h"
#include "secondwindow.h"
#include "drawingdispatcher.h"

void run_client();
extern void disconnect_client();
bool recv_drawpacket(int fd, DrawPacket& pkt);
int retMyNum();
void send_answer(const std::string& ans);
void send_coordinate(double x, double y, int penColor, int penWidth, int drawStatus);

#endif // CLIENT_H
