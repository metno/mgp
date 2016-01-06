#ifndef LANESCENE_H
#define LANESCENE_H

#include <QGraphicsScene>
#include <QDate>

class LaneHeaderScene;
class QGraphicsRectItem;
class QGraphicsLineItem;
class LaneItem;
class TaskItem;
class QGraphicsSceneMouseEvent;
class QAction;

class LaneScene : public QGraphicsScene
{
    Q_OBJECT
    friend class LaneView;

public:
    LaneScene(LaneHeaderScene *, const QDate &, int, QObject * = 0);
    QDate baseDate() const;
    int dateSpan() const;
    static int secsInHour() { return 3600; }
    static int secsInDay() { return 24 * secsInHour(); }
    void setDateRange(const QDate &, int);

public slots:
    void updateFromTaskMgr();
    void updateGeometryAndContents();

private:
    LaneHeaderScene *laneHeaderScene_;
    QList<QGraphicsRectItem *> dateItems_;
    QList<QGraphicsLineItem *> timeItems_;
    QList<QGraphicsRectItem *> roleTimeItems_;

    QList<LaneItem *> laneItems_;
    QList<qint64> laneItemRoleIds() const;

    QList<TaskItem *> taskItems(qint64 = -1) const;
    QList<TaskItem *> taskItems(const QPointF &) const;
    QList<qint64> taskItemRoleIds() const;
    QList<qint64> taskIds(const QList<TaskItem *> &) const;

    void updateBaseItemGeometry();
    void updateTaskItems();
    void updateTaskItemsInLane(LaneItem *, int, int, int);
    QDate baseDate_;
    int dateSpan_;

    void updateCurrTimeMarker();
    QGraphicsLineItem *currTimeMarker_;

    void updateRoleTimeItems();

    qreal timestampToVPos(long) const;
    long vPosToTimestamp(qreal) const;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);

    void setCurrTask(TaskItem *);
    void clearCurrTask();
    void updateCurrTaskItem(bool);

    QAction *addTaskAction_;
    QAction *editTaskAction_;
    QAction *removeTaskAction_;

    TaskItem *hoverTaskItem_; // task item (if any) currently hovered
    TaskItem *currTaskItem_; // task item (if any) currently selected
    qint64 pendingCurrTaskId_; // task ID that was just added and whose item should be set as current
    QGraphicsLineItem *hoverTimeMarker_;
    QGraphicsRectItem *hoverRoleMarker_;
    QGraphicsRectItem *currTaskMarker_;

    int hoverLaneIndex_;
    int insertTop_;
    int insertBottom_;
    int nextNewTaskId_;
    bool contextMenuActive_;

private slots:
    void addNewTask();
    void editCurrentTask();
    void removeCurrentTask();
    void handleViewScaleUpdate();
    void handleViewLeft();
    void handleLanesSwapped(int, int);

signals:
    void dateRangeChanged();
};

#endif // LANESCENE_H
