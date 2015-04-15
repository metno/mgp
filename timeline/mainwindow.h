#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QDate>

class LeftHeaderScene;
class LaneScene;
class TopHeaderScene;
class QSplitter;
class QDateEdit;
class QSpinBox;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(const QDate &, int, QWidget * = 0);

private:
    LeftHeaderScene *leftHeaderScene_;
    LaneScene *laneScene_;
    TopHeaderScene *topHeaderScene_;
    QSplitter *topSplitter_;
    QSplitter *botSplitter_;
    QDateEdit *baseDateEdit_;
    QSpinBox *dateSpanSpinBox_;
    virtual void keyPressEvent(QKeyEvent *);
    virtual void showEvent(QShowEvent *);

private slots:
    void updateFromTaskMgr();
    void updateGeometry();
    void updateDateRange();
    void updateSplitters(int, int);
    void showToday();
    void resetZooming();

    void test1(); // ### for testing
};

#endif // MAINWINDOW_H
