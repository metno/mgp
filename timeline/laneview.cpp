#include "laneview.h"
#include "lanescene.h"

LaneView::LaneView(LaneScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
{
//    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void LaneView::resizeEvent(QResizeEvent *)
{
    //fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
}
