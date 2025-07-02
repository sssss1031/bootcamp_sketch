#ifndef DRAWINGDISPATCHER_H
#define DRAWINGDISPATCHER_H


#include <QObject>
class DrawingDispatcher : public QObject {
    Q_OBJECT
public:
    static DrawingDispatcher& instance() {
        static DrawingDispatcher d; return d;
    }
signals:
    void drawArrived(int drawStatus, double x, double y, int color, int thick);
};

#endif // DRAWINGDISPATCHER_H
