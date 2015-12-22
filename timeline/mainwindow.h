#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QDate>

class RolesScene;
class LaneScene;
class TimelineScene;
class TimelineController;
class RolesController;
class TasksController;
class QSplitter;
class QDateEdit;
class QSpinBox;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    static void init(const QDate &, int);
    static MainWindow &instance();
    void handleKeyPressEvent(QKeyEvent *);

private:
    static bool isInit_;
    static QDate baseDate_;
    static int dateSpan_;
    MainWindow();

    RolesScene *rolesScene_;
    LaneScene *laneScene_;
    TimelineScene *timelineScene_;
    QSplitter *topSplitter_;
    QSplitter *botSplitter_;

    TimelineController *timelineController_;
    RolesController *rolesController_;
    TasksController *tasksController_;

    virtual void keyPressEvent(QKeyEvent *);
    virtual void showEvent(QShowEvent *);

private slots:
    void updateFromTaskMgr();
    void updateGeometry();
    void updateDateRange(bool = false);
    void updateSplitters(int, int);
    void resetZooming();

    void add5Roles(); // ### for testing
    void test2(); // ### for testing
};

#endif // MAINWINDOW_H
