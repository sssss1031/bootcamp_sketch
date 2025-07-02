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
    explicit SecondWindow(QWidget *parent = 0);
    ~SecondWindow();


signals:
    void backToMain();

private:
    Ui::SecondWindow *ui;
    TouchDrawingWidget *drawingWidget;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onLineEditReturnPressed();
    void backToMainRequested();

};

#endif // SECONDWINDOW_H
