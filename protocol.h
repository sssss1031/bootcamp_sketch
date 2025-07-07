#ifndef PROTOCOL_H
#define PROTOCOL_H
#define SERVER_IP "192.168.10.2"
#define SERVER_PORT 25000

#include <string>
#include <vector>
#include <utility>

#define MAX_CLIENTS 10
#define MSG_SET_MAX_PLAYER 9999
#define MSG_REJECTED 4004
#define MSG_SET_TRUE_ANSWER 42

enum MessageType {
    MSG_DRAW = 1,
    MSG_CLEAR = 2,
    MSG_PING  = 3,
    MSG_ANSWER = 4,
    MSG_CORRECT = 5,
    MSG_WRONG = 6,
    MSG_PLAYER_NUM = 7,
    MSG_DISCONNECT = 8,
    MSG_PLAYER_CNT = 9,
    MSG_SELECTED_PLAYER = 10,
    MSG_ERASE_ALL = 11,
    MSG_SCORE = 12
};

struct DrawPacket {
    int type;
    int x;
    int y;
    int color;
    int thick;
    int drawStatus;
};

struct AnswerPacket {
    int type;
    std::string nickname;
    std::string answer;
};

struct CorrectPacket {
    int type;
    std::string nickname;
};

struct WrongPacket {
    int type;
    std::string nickname;
    std::string message;
};

struct CommonPacket {
    int type;
    std::string nickname;
    std::string message;
};

struct PlayerNumPacket {
    int type;
    int player_num;
};

struct PlayerCntPacket {
    int type;
    int currentPlayer_cnt;
    int maxPlayer;
};

struct SelectedPlayerPacket {
    int type;
    std::string nickname;
};

struct EraseAllPacket {
    int type;
};

struct ScorePacket {
    int type;
    std::vector<std::pair<std::string, int>> score;
};

#endif
