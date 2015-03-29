#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class LeftHeaderScene;
class LaneScene;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget * = 0);

private:
    LeftHeaderScene *leftHeaderScene_;
    LaneScene *laneScene_;
    virtual void keyPressEvent(QKeyEvent *);

private slots:
    void refresh();
};

#endif // MAINWINDOW_H
