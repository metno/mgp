#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QDate>

class LaneHeaderScene;
class LaneScene;
class TimelineScene;
class TimelineController;
class LanesController;
class TasksController;
class QSplitter;
class QDateEdit;
class QSpinBox;
class QSplitter;
class QString;
class QSlider;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    static void init(const QDate &, const QDate &, const QDate &, int, int, int);
    static MainWindow &instance();
    void handleKeyPressEvent(QKeyEvent *);

private:
    static bool isInit_;
    static QDate baseDate_;
    static QDate minBaseDate_;
    static QDate maxBaseDate_;
    static int dateSpan_;
    static int minDateSpan_;
    static int maxDateSpan_;
    MainWindow();

    LaneHeaderScene *laneHeaderScene_;
    LaneScene *laneScene_;
    TimelineScene *timelineScene_;
    QSplitter *hsplitter1_;
    QSplitter *hsplitter2_;
    QSplitter *hsplitter3_;
    QSplitter *vsplitter1_;

    QSlider *hscaleSlider_;
    QSlider *vscaleSlider_;

    TimelineController *timelineController_;
    LanesController *lanesController_;
    TasksController *tasksController_;

    virtual void keyPressEvent(QKeyEvent *);
    virtual void showEvent(QShowEvent *);
    virtual void resizeEvent(QResizeEvent *);

    QList<int> loadSplitterSizesFromSettings(const QString &, const QList<int> &);
    void updateSplitterSettings(const QSplitter *, const QString &);
    void updateAllSplitterSettings();

private slots:
    void updateFromTaskMgr();
    void updateGeometry();
    void updateDateRange(bool = false);
    void updateHSplitter1or2(int, int);
    void updateHSplitter3(int, int);
    void updateVSplitter1(int, int);
    void addNewLane();
    void handleHScaleSliderUpdate(int);
    void handleVScaleSliderUpdate(int);
    void handleViewScaled(qreal, qreal);
};

#endif // MAINWINDOW_H
