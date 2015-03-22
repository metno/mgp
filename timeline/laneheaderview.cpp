#include "laneheaderview.h"
#include "laneheaderscene.h"
#include "common.h"
#include <QResizeEvent>

LaneHeaderView::LaneHeaderView(LaneHeaderScene *lhScene, QWidget *parent)
    : QGraphicsView(lhScene, parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void LaneHeaderView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}
