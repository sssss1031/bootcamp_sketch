#ifndef MYBOARDFRAME_H
#define MYBOARDFRAME_H

#include <QFrame>

class MyBoardFrame : public QFrame {
    Q_OBJECT
public:
    explicit MyBoardFrame(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

#endif // MYBOARDFRAME_H

