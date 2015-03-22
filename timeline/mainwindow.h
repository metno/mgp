#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow(QWidget * = 0);
private:
    void keyPressEvent(QKeyEvent *);
};

#endif // MAINWINDOW_H
