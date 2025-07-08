#include "myboardframe.h"
#include <QPainter>
#include <QPixmap>
#include <QTransform>

MyBoardFrame::MyBoardFrame(QWidget* parent) : QFrame(parent) {}

void MyBoardFrame::paintEvent(QPaintEvent* event) {
    QFrame::paintEvent(event);
    QPainter painter(this);
    QPixmap bg(":/new/prefix1/whiteboard.png");
    if (!bg.isNull()) {
        // 1. 90도 회전
        QTransform transform;
        transform.rotate(90);
        QPixmap rotated = bg.transformed(transform, Qt::SmoothTransformation);

        // 2. 프레임 크기에 맞게 비율 유지 + 더 크게 (여백 없이, 잘림 허용)
        QSize frameSize = this->size();
        QPixmap expanded = rotated.scaled(frameSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        // 3. 중앙 crop
        int x = (expanded.width() - frameSize.width()) / 2;
        int y = (expanded.height() - frameSize.height()) / 2;
        painter.drawPixmap(0, 0, expanded, x, y, frameSize.width(), frameSize.height());
    }
}
