#ifndef SECONDWINDOW_H
#define SECONDWINDOW_H

#include <QMainWindow>

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

private slots:
    void onLineEditReturnPressed();

};

#endif // SECONDWINDOW_H
