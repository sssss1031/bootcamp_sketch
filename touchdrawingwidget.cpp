#include "touchdrawingwidget.h"
#include "client.h"
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
                        send_coordinate(lastPoint.x(), lastPoint.y(), penColor, penWidth, 0);
                        break;

                    case QEvent::TouchUpdate:
                                    if (drawing) {
                                        QPointF currentPoint = pt.pos();
                                        path.quadTo(lastPoint, (lastPoint + currentPoint) / 2);  // 곡선 보간
                                        lastPoint = currentPoint;
                                        update();
                                        send_coordinate(currentPoint.x(), currentPoint.y(), penColor, penWidth, 1);
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
                                                case GREEN: color = Qt::green; break;
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
                                            send_coordinate(lastPoint.x(), lastPoint.y(), penColor, penWidth, 2);
                                        }
                                        break;
                        default:
                            break;
                        }
                }
            return true;

    }

        return QWidget::event(event);;
}


void TouchDrawingWidget::onDrawPacket(int drawStatus, double x, double y, int color, int thick)
{
    switch(drawStatus) {
    case DRAW_BEGIN:
        drawing = true;
        lastPoint = QPointF(x, y);
        path = QPainterPath();
        path.moveTo(lastPoint);

        this->penColor = color;
        this->penWidth = thick;
        break;
    case DRAW_POINT:
        if (drawing) {
            QPointF currentPoint(x, y);
            path.quadTo(lastPoint, (lastPoint + currentPoint) / 2);
            lastPoint = currentPoint;
            update();
           }
         break;
    case DRAW_END:
            if (drawing) {
                QPainter painter(&canvas);
                painter.setRenderHint(QPainter::Antialiasing);

                QColor qcolor;
                switch(penColor) {
                    case BLACK: qcolor = Qt::black; break;
                    case YELLOW: qcolor = Qt::yellow; break;
                    case RED: qcolor = Qt::red; break;
                    case GREEN: qcolor = Qt::green; break;
                    case BLUE: qcolor = Qt::blue; break;
                    case WHITE: qcolor = Qt::white; break;
                    case ERASER: color = Qt::white; break;
                }
                int width = 3;
                switch(penWidth) {
                      case SHALLOW: width = 3; break;
                      case MIDDLE: width = 6; break;
                      case THICK: width = 9; break;
                 }

                 painter.setPen(QPen(qcolor, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
                 painter.drawPath(path);

                 drawing = false;
                 path = QPainterPath();
                 update();
            }
            break;
      }
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
            case GREEN: color = Qt::green; break;
            case BLUE: color = Qt::blue; break;
            case WHITE: color = Qt::white; break;
            case ERASER: color = Qt::white; break;
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

void TouchDrawingWidget::erase()
{
    canvas.fill(Qt::white);
    qDebug() << "erasecalled\n";
    update();
}

void TouchDrawingWidget::reset()
{
    canvas.fill(Qt::white);
    update();
    penColor=1;
    penWidth=11;
    emit penChanged(penColor, penWidth);
}

void TouchDrawingWidget::keyPressEvent(QKeyEvent *event)
{
   // QString text;
    switch(event->key())
        { // black,yellow,red,blue,white
        case Qt::Key_1: penColor = BLACK; break;
        case Qt::Key_2: penColor = YELLOW; break;
        case Qt::Key_3: penColor = RED; break;
        case Qt::Key_4: penColor = GREEN; break;
        case Qt::Key_5: penColor = BLUE; break;
        case Qt::Key_6: penColor = WHITE; break;

        case Qt::Key_Q: penWidth = SHALLOW; break;
        case Qt::Key_W: penWidth = MIDDLE; break;
        case Qt::Key_E: penWidth = THICK; break;

        case Qt::Key_Space: eraseMode = true; break;
    }
        
    emit penChanged(penColor, penWidth);
    
    switch(penColor)
        {
        case BLACK: qDebug() << "black"; break;
        case YELLOW: qDebug() << "yellow"; break;
        case RED: qDebug() << "red"; break;
        case GREEN: qDebug() << "green"; break;
        case BLUE: qDebug() << "blue"; break;
        case WHITE: qDebug() << "white"; break;
        case ERASER: qDebug() << "eraser"; break;
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

void TouchDrawingWidget::colorClicked()
{
    if (penColor >= 6) { penColor = 1; }
    else { penColor += 1; }
    emit penChanged(penColor, penWidth);
}

void TouchDrawingWidget::setEraser()
{
    penColor = 7;
    emit penChanged(penColor, penWidth);
}

void TouchDrawingWidget::widthClicked()
{
    if (penWidth==12) { penWidth = 10; }
    else { penWidth += 1; }
    emit penChanged(penColor, penWidth);
}

void TouchDrawingWidget::widthUp()
{
    if (penWidth==12) return;
    else { penWidth += 1; }
    emit penChanged(penColor, penWidth);
}

void TouchDrawingWidget::widthDown()
{
    if (penWidth==10) return;
    else { penWidth -= 1; }
    emit penChanged(penColor, penWidth);
}

void TouchDrawingWidget::giveFocus()
{
    setFocus();
}

void TouchDrawingWidget::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    //qDebug() << "got focus!";
}
