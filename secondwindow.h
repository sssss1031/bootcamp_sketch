#ifndef SECONDWINDOW_H
#define SECONDWINDOW_H

#include <QMainWindow>
#include "touchdrawingwidget.h"

namespace Ui {
class SecondWindow;
}

class SecondWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SecondWindow(int maxPlayer = 2, QWidget *parent = nullptr);
    ~SecondWindow();


signals:
    void backToMain();

private:
    int m_maxPlayer;
    Ui::SecondWindow *ui;
    TouchDrawingWidget *drawingWidget;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onLineEditReturnPressed();
    void backToMainRequested();
    void appendChatMessage(const QString& message);
    void onPenChanged(int color, int width);
};

#endif // SECONDWINDOW_H
