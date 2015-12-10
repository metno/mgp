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
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::ScrollHandDrag);
    connect(dynamic_cast<LaneView *>(thScene->laneScene_->views().first()), SIGNAL(scaled(qreal, qreal)), SLOT(updateScale(qreal, qreal)));
}

void TimelineView::updateScale(qreal, qreal sy)
{
    setTransform(QTransform::fromScale(1.0, sy)); // scale vertical dimension only
}

void TimelineView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}
