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

void LaneHeaderView::updateScale(qreal sx, qreal sy)
{
    setTransform(QTransform::fromScale(sx, sy));
}

void LaneHeaderView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}

void LaneHeaderView::wheelEvent(QWheelEvent *event)
{
    const QPair<qreal, qreal> scaleFactors = WheelScaler::exec(this, event);
    if (scaleFactors.first > 0)
        emit scaled(scaleFactors.first, scaleFactors.second);
    else
        QGraphicsView::wheelEvent(event);
}
