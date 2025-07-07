#ifndef CHATMESSAGEDISPATCHER_H
#define CHATMESSAGEDISPATCHER_H

#include <QObject>
#include <QString>

/*채팅 메시지를 SecondWindow/ThirdWindow에 각각 직접 QMetaObject::invokeMethod로 보내는 부분을
ChatMessageDispatcher 싱글톤을 통해 한 번만 emit 하도록 변경*/


class ChatMessageDispatcher : public QObject {
    Q_OBJECT
public:
    static ChatMessageDispatcher& instance() {
        static ChatMessageDispatcher dispatcher;
        return dispatcher;
    }
signals:
    void chatMessageArrived(const QString& msg); // 메시지 도착 시 시그널
private:
    ChatMessageDispatcher() {} // 생성자 private(외부에서 생성 못하게)
};

#endif // CHATMESSAGEDISPATCHER_H

