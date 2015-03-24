#ifndef LANESCENE_H
#define LANESCENE_H

#include <QGraphicsScene>

class LaneHeaderScene;
class QGraphicsRectItem;
class LaneBGItem;

class LaneScene : public QGraphicsScene
{
    Q_OBJECT
    friend class LaneView;

public:
    LaneScene(LaneHeaderScene *, qreal, QObject * = 0);

public slots:
    void refresh();

private:
    LaneHeaderScene *laneHeaderScene_;
    QGraphicsRectItem *bgItem_;
    QList<LaneBGItem *> laneItems() const;
    QList<qint64> laneItemRoleIds() const;
    void addLaneItem(qint64);
};

#endif // LANESCENE_H
