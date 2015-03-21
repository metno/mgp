#include "laneheaderview.h"
#include "laneheaderscene.h"

LaneHeaderView::LaneHeaderView(LaneHeaderScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void LaneHeaderView::resizeEvent(QResizeEvent *)
{
    //fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
}
