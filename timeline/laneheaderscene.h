#ifndef LANEHEADERSCENE_H
#define LANEHEADERSCENE_H

#include <QGraphicsScene>

class QGraphicsRectItem;

class LaneHeaderScene : public QGraphicsScene
{
    Q_OBJECT
public:
    LaneHeaderScene(qreal, qreal, qreal, qreal, QObject * = 0);
    void update();
private:
    QGraphicsRectItem *bgItem_;
};

#endif // LANEHEADERSCENE_H
