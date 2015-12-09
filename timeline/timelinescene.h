#ifndef TIMELINESCENE_H
#define TIMELINESCENE_H

#include <QGraphicsScene>

class LaneScene;
class QGraphicsRectItem;
class QGraphicsTextItem;

class TimelineScene : public QGraphicsScene
{
    Q_OBJECT
    friend class TimelineView;

public:
    TimelineScene(LaneScene *, qreal, QObject * = 0);

public slots:
    void updateFromTaskMgr();
    void updateGeometry();

private:
    LaneScene *laneScene_;
    QList<QGraphicsRectItem *> dateRectItems_;
    QList<QGraphicsTextItem *> dateTextItems_;
    QList<QGraphicsTextItem *> timeTextItems_;
    void updateItemGeometry();

private slots:
    void updateDateRange();
};

#endif // TIMELINESCENE_H
