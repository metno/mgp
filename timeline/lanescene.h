#ifndef LANESCENE_H
#define LANESCENE_H

#include <QGraphicsScene>
#include <QDate>

class RolesScene;
class QGraphicsRectItem;
class QGraphicsLineItem;
class LaneItem;
class TaskItem;

class LaneScene : public QGraphicsScene
{
    Q_OBJECT
    friend class LaneView;

public:
    LaneScene(RolesScene *, const QDate &, int, QObject * = 0);
    QDate baseDate() const;
    int dateSpan() const;
    static int secsInHour() { return 3600; }
    static int secsInDay() { return 24 * secsInHour(); }
    void setDateRange(const QDate &, int);

public slots:
    void updateFromTaskMgr();
    void updateGeometry();

private:
    RolesScene *rolesScene_;
    QList<QGraphicsRectItem *> dateItems_;
    QList<QGraphicsLineItem *> timeItems_;
    QList<QGraphicsRectItem *> roleTimeItems_;

    QList<LaneItem *> laneItems() const;
    QList<qint64> laneItemRoleIds() const;
    void addLaneItem(qint64);

    QList<TaskItem *> taskItems(qint64 = -1) const;
    QList<qint64> taskItemRoleIds() const;
    void addTaskItems(qint64);

    void updateBaseItemGeometry();
    void updateTaskItemGeometry();
    void updateTaskItemGeometryInLane(LaneItem *, int, int, int);
    QDate baseDate_;
    int dateSpan_;

    void updateCurrTimeMarker();
    QGraphicsLineItem *currTimeMarker_;

    void updateRoleTimeItems();

    qreal timestampToVPos(long) const;

signals:
    void dateRangeChanged();
};

#endif // LANESCENE_H
