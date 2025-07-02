#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "protocol.h"

void run_client();
extern void disconnect_client();
void send_answer(const std::string& ans);
void send_coordinate(double x, double y, int penColor, int penWidth);

#endif // CLIENT_H
