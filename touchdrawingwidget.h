#ifndef TOUCHDRAWINGWIDGET_H
#define TOUCHDRAWINGWIDGET_H
#include <QWidget>
#include <QPointF>
#include <QPainterPath>
#include <QImage>
#include <QKeyEvent>

class TouchDrawingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TouchDrawingWidget(QWidget *parent = nullptr);
    //void onDrawPacket(int drawStatus, double x, double y, int color, int thick);
    void setEraseMode(bool enabled);
    void erase();
    void reset();
    enum PacketType {
        DRAW_BEGIN,   // 그리기 시작
        DRAW_POINT,   // 그리기 좌표
        DRAW_END      // 그리기 끝
    };

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private:
    QImage canvas;
    QPainterPath path;
    QPointF lastPoint;
    bool drawing=false;
    bool eraseMode=false;
    QPointF lastDrawPoint4client;
    bool hasLastDrawPoint = false;
    int penColor = 1;
    int penWidth = 11;
    enum Color{
        BLACK = 1,
        YELLOW,
        RED,
        BLUE,
        WHITE,
    };
    enum Width{
        SHALLOW=10,
        MIDDLE,
        THICK,
    };

public slots:
    void giveFocus();
    void onDrawPacket(int drawStatus, double x, double y, int color, int thick);
    void colorClicked();
    void widthClicked();

signals:
    void penChanged(int c, int w);

};

#endif // TOUCHDRAWINGWIDGET_H

