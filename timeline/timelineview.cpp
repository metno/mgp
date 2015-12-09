#include "timelineview.h"
#include "timelinescene.h"
#include "laneview.h"
#include "lanescene.h"
#include "wheelscaler.h"
#include "common.h"
#include <QResizeEvent>

TimelineView::TimelineView(TimelineScene *thScene, QWidget *parent)
    : QGraphicsView(thScene, parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setDragMode(QGraphicsView::ScrollHandDrag);
    connect(dynamic_cast<LaneView *>(thScene->laneScene_->views().first()), SIGNAL(scaled(qreal, qreal)), SLOT(updateScale(qreal, qreal)));
}

void TimelineView::updateScale(qreal sx, qreal)
{
    setTransform(QTransform::fromScale(sx, 1.0)); // scale horizontal dimension only
}

void TimelineView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}
