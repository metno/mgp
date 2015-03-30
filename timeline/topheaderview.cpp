#include "topheaderview.h"
#include "topheaderscene.h"
#include "laneview.h"
#include "lanescene.h"
#include "wheelscaler.h"
#include "common.h"
#include <QResizeEvent>

TopHeaderView::TopHeaderView(TopHeaderScene *thScene, QWidget *parent)
    : QGraphicsView(thScene, parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    connect(dynamic_cast<LaneView *>(thScene->laneScene_->views().first()), SIGNAL(scaled(qreal, qreal)), SLOT(updateScale(qreal, qreal)));
}

void TopHeaderView::updateScale(qreal sx, qreal)
{
    setTransform(QTransform::fromScale(sx, 1.0)); // scale horizontal dimension only
}

void TopHeaderView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}
