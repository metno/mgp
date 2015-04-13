#ifndef LANESCENE_H
#define LANESCENE_H

#include <QGraphicsScene>
#include <QDate>

class LeftHeaderScene;
class QGraphicsRectItem;
class QGraphicsLineItem;
class LaneBGItem;
class TaskItem;

class LaneScene : public QGraphicsScene
{
    Q_OBJECT
    friend class LaneView;

public:
    LaneScene(LeftHeaderScene *, const QDate &, int, QObject * = 0);
    QDate baseDate() const;
    int dateSpan() const;
    static qreal dateWidth();
    void setDateRange(const QDate &, int);

public slots:
    void updateFromTaskMgr();
    void updateGeometry();

private:
    LeftHeaderScene *leftHeaderScene_;
    QList<QGraphicsRectItem *> dateItems_;

    QList<LaneBGItem *> laneItems() const;
    QList<qint64> laneItemRoleIds() const;
    void addLaneItem(qint64);

    QList<TaskItem *> taskItems(qint64 = -1) const;
    QList<qint64> taskItemRoleIds() const;
    void addTaskItems(qint64);

    void updateDateItemGeometry();
    QDate baseDate_;
    int dateSpan_;

    void updateCurrTimeMarker();
    QGraphicsLineItem *currTimeMarker_;

signals:
    void dateRangeChanged();
};

#endif // LANESCENE_H
