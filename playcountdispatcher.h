#ifndef PLAYERCOUNTDISPATCHER_H
#define PLAYERCOUNTDISPATCHER_H

#pragma once
#include <QObject>

class PlayerCountDispatcher : public QObject {
    Q_OBJECT
public:
    static PlayerCountDispatcher& instance() {
        static PlayerCountDispatcher s;
        return s;
    }
signals:
    void playerCountUpdated(int current, int max);
private:
    PlayerCountDispatcher() = default;
    ~PlayerCountDispatcher() = default;
    Q_DISABLE_COPY(PlayerCountDispatcher)
};

#endif // PLAYERCOUNTDISPATCHER_H
