#ifndef LANESCENE_H
#define LANESCENE_H

#include <QGraphicsScene>

class LaneHeaderScene;
class QGraphicsRectItem;
class LaneItem;

class LaneScene : public QGraphicsScene
{
    Q_OBJECT
public:
    LaneScene(LaneHeaderScene *, qreal, QObject * = 0);

public slots:
    void refresh();

private:
    LaneHeaderScene *laneHeaderScene_;
    QGraphicsRectItem *bgItem_;
    QList<LaneItem *> laneItems() const;
    void addOneLaneItem();
    void removeOneLaneItem();
};

#endif // LANESCENE_H
