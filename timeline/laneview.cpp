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

    connect(this, SIGNAL(scaled(qreal, qreal)), dynamic_cast<LaneHeaderView *>(scene->laneHeaderScene_->views().first()), SLOT(updateScale(qreal, qreal)));
    connect(dynamic_cast<LaneHeaderView *>(scene->laneHeaderScene_->views().first()), SIGNAL(scaled(qreal, qreal)), SLOT(updateScale(qreal, qreal)));
}

void LaneView::updateScale(qreal sx, qreal sy)
{
    setTransform(QTransform::fromScale(sx, sy));
}

void LaneView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
}

void LaneView::wheelEvent(QWheelEvent *event)
{
    const QPair<qreal, qreal> scaleFactors = WheelScaler::exec(this, event);
    if (scaleFactors.first > 0)
        emit scaled(scaleFactors.first, scaleFactors.second);
    else
        QGraphicsView::wheelEvent(event);
}
