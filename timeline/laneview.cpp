#include "laneview.h"
#include "lanescene.h"
#include "laneheaderview.h"
#include "laneheaderscene.h"
#include "wheelscaler.h"
#include "common.h"
#include <QResizeEvent>

LaneView::LaneView(LaneScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    connect(this, SIGNAL(scaled(qreal)), dynamic_cast<LaneHeaderView *>(scene->laneHeaderScene_->views().first()), SLOT(updateScale(qreal)));
    connect(dynamic_cast<LaneHeaderView *>(scene->laneHeaderScene_->views().first()), SIGNAL(scaled(qreal)), SLOT(updateScale(qreal)));
}

void LaneView::updateScale(qreal val)
{
    scale(val, val);
}

void LaneView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
}

void LaneView::wheelEvent(QWheelEvent *event)
{
    const qreal scaleVal = WheelScaler::exec(this, event);
    if (scaleVal > 0)
        emit scaled(scaleVal);
    else
        QGraphicsView::wheelEvent(event);
}
