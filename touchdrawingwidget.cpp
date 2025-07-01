#include "touchdrawingwidget.h"
#include <QTouchEvent>
#include <QPainter>
#include <QDebug>

TouchDrawingWidget::TouchDrawingWidget(QWidget *parent)
    : QWidget(parent), canvas(size(), QImage::Format_ARGB32_Premultiplied), eraseMode(false)
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    setAttribute(Qt::WA_StaticContents);
    setAttribute(Qt::WA_OpaquePaintEvent);
    canvas.fill(Qt::white);
    setFocusPolicy(Qt::StrongFocus);
}

void TouchDrawingWidget::resizeEvent(QResizeEvent *event)
{
    if (canvas.size() != size()) {
        QImage newImage(size(), QImage::Format_ARGB32_Premultiplied);
        newImage.fill(Qt::white);
        QPainter painter(&newImage);
        painter.drawImage(0, 0, canvas);
        canvas = newImage;
    }
    QWidget::resizeEvent(event);
}

bool TouchDrawingWidget::event(QEvent *event)
{
    if (event->type() == QEvent::TouchBegin ||
            event->type() == QEvent::TouchUpdate ||
            event->type() == QEvent::TouchEnd) {

        auto *touchEvent = static_cast<QTouchEvent *>(event);
                const QList<QTouchEvent::TouchPoint> points = touchEvent->touchPoints();

                if (!points.isEmpty()) {
                    const QTouchEvent::TouchPoint &pt = points.first();

                    switch (event->type()) {
                    case QEvent::TouchBegin:
                        drawing = true;
                        lastPoint = pt.pos();
                        path = QPainterPath();
                        path.moveTo(lastPoint);
                        break;

                    case QEvent::TouchUpdate:
                                    if (drawing) {
                                        QPointF currentPoint = pt.pos();
                                        path.quadTo(lastPoint, (lastPoint + currentPoint) / 2);  // 곡선 보간
                                        lastPoint = currentPoint;
                                        update();
                                    }
                                    break;
                    case QEvent::TouchEnd:
                                    if (drawing) {
                                        QPainter painter(&canvas);
                                        painter.setRenderHint(QPainter::Antialiasing);

                                        QColor color;
                                        switch(penColor) {
                                            case BLACK: color = Qt::black; break;
                                            case YELLOW: color = Qt::yellow; break;
                                            case RED: color = Qt::red; break;
                                            case BLUE: color = Qt::blue; break;
                                            case WHITE: color = Qt::white; break;
                                        }

                                        int width;
                                        switch(penWidth) {
                                            case SHALLOW: width = 3; break;
                                            case MIDDLE: width = 6; break;
                                            case THICK: width = 9; break;
                                        }

                                        painter.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                                        painter.drawPath(path);
                                        drawing = false;
                                        path = QPainterPath();
                                        update();
                                    }
                                    break;
                    default:
                        break;
                    }
            }
            return true;
        }

        return QWidget::event(event);
}

void TouchDrawingWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawImage(event->rect(), canvas, event->rect());

    if (drawing){
        painter.setRenderHint(QPainter::Antialiasing);

        QColor color;
        switch(penColor) {
            case BLACK: color = Qt::black; break;
            case YELLOW: color = Qt::yellow; break;
            case RED: color = Qt::red; break;
            case BLUE: color = Qt::blue; break;
            case WHITE: color = Qt::white; break;
        }

        int width;
        switch(penWidth) {
            case SHALLOW: width = 3; break;
            case MIDDLE: width = 6; break;
            case THICK: width = 9; break;
        }

        painter.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPath(path);
    }
}

void TouchDrawingWidget::setEraseMode(bool enabled)
{
    eraseMode = enabled;
    if(eraseMode)
        qDebug() << "erase enabled!";
    else
        qDebug() << "pen enabled!";
}

void TouchDrawingWidget::erase()
{
    canvas.fill(Qt::white);
    update();
}

void TouchDrawingWidget::keyPressEvent(QKeyEvent *event)
{
   // QString text;
    switch(event->key())
        { // black,yellow,red,blue,white
        case Qt::Key_1: penColor = BLACK; break;
        case Qt::Key_2: penColor = YELLOW; break;
        case Qt::Key_3: penColor = RED; break;
        case Qt::Key_4: penColor = BLUE; break;
        case Qt::Key_5: penColor = WHITE; break;

        case Qt::Key_Q: penWidth = SHALLOW; break;
        case Qt::Key_W: penWidth = MIDDLE; break;
        case Qt::Key_E: penWidth = THICK; break;

        case Qt::Key_Space: eraseMode = true; break;

        default: penColor = 1; break;
    }

    switch(penColor)
        {
        case BLACK: qDebug() << "black"; break;
        case YELLOW: qDebug() << "yellow"; break;
        case RED: qDebug() << "red"; break;
        case BLUE: qDebug() << "blue"; break;
        case WHITE: qDebug() << "white"; break;
    }

    switch(penWidth)
        {
        case SHALLOW: qDebug() << "shallow"; break;
        case MIDDLE: qDebug() << "middle"; break;
        case THICK: qDebug() << "thick"; break;
    }

    if (eraseMode) {
        erase();
        qDebug() << "erase!";
        eraseMode = false;
    }
        // ui->label->setText(text);
}

void TouchDrawingWidget::giveFocus()
{
    setFocus();
}

void TouchDrawingWidget::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    qDebug() << "got focus!";
}
