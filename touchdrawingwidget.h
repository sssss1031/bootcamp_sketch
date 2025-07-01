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
    void setEraseMode(bool enabled);
    void erase();

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
    int penColor = 1;
    int penWidth = 6;
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
};

#endif // TOUCHDRAWINGWIDGET_H

