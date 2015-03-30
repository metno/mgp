#ifndef TOPHEADERSCENE_H
#define TOPHEADERSCENE_H

#include <QGraphicsScene>

class LaneScene;
class QGraphicsRectItem;
class QGraphicsLineItem;

class TopHeaderScene : public QGraphicsScene
{
    Q_OBJECT
    friend class TopHeaderView;

public:
    TopHeaderScene(LaneScene *, qreal, QObject * = 0);

public slots:
    void refresh();

private:
    LaneScene *laneScene_;
    QGraphicsRectItem *bgItem_;
    QGraphicsLineItem *lineItem_;
};

#endif // TOPHEADERSCENE_H
