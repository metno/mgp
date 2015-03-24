#include "laneheaderview.h"
#include "laneheaderscene.h"
#include "wheelscaler.h"
#include "common.h"
#include <QResizeEvent>

LaneHeaderView::LaneHeaderView(LaneHeaderScene *lhScene, QWidget *parent)
    : QGraphicsView(lhScene, parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void LaneHeaderView::updateScale(qreal val)
{
    scale(val, val);
    if (transform().m11() < 1.0)
        setTransform(QTransform());
}

void LaneHeaderView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}

void LaneHeaderView::wheelEvent(QWheelEvent *event)
{
    const qreal scaleVal = WheelScaler::exec(this, event);
    if (scaleVal > 0)
        emit scaled(scaleVal);
    else
        QGraphicsView::wheelEvent(event);
}
