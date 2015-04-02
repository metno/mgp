#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QDate>

class LeftHeaderScene;
class LaneScene;
class TopHeaderScene;
class QSplitter;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(const QDate &, QWidget * = 0);

private:
    QDate baseDate_;
    LeftHeaderScene *leftHeaderScene_;
    LaneScene *laneScene_;
    TopHeaderScene *topHeaderScene_;
    QSplitter *topSplitter_;
    QSplitter *botSplitter_;
    virtual void keyPressEvent(QKeyEvent *);

private slots:
    void refresh();
    void splitterMoved(int, int);
};

#endif // MAINWINDOW_H
