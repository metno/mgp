#ifndef TOPHEADERSCENE_H
#define TOPHEADERSCENE_H

#include <QGraphicsScene>

class LaneScene;
class QGraphicsRectItem;
class QGraphicsTextItem;

class TopHeaderScene : public QGraphicsScene
{
    Q_OBJECT
    friend class TopHeaderView;

public:
    TopHeaderScene(LaneScene *, qreal, QObject * = 0);

public slots:
    void updateFromTaskMgr();
    void updateGeometry();

private:
    LaneScene *laneScene_;
    QList<QGraphicsRectItem *> dateRectItems_;
    QList<QGraphicsTextItem *> dateTextItems_;
    void updateDateItemGeometries();

private slots:
    void updateDateRange();
};

#endif // TOPHEADERSCENE_H
