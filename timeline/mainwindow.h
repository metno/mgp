#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class LaneScene;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow(QWidget * = 0);
public slots:
    void update();
private:
    LaneScene *laneScene_;
    void keyPressEvent(QKeyEvent *);
};

#endif // MAINWINDOW_H
