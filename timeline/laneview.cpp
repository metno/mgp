#include "laneview.h"
#include "lanescene.h"
#include "leftheaderview.h"
#include "leftheaderscene.h"
#include "wheelscaler.h"
#include "common.h"
#include <QResizeEvent>

LaneView::LaneView(LaneScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setDragMode(QGraphicsView::ScrollHandDrag);
    connect(this, SIGNAL(scaled(qreal, qreal)), dynamic_cast<LeftHeaderView *>(scene->leftHeaderScene_->views().first()), SLOT(updateScale(qreal, qreal)));
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
    if (scaleFactors.first > 0) {
        const qreal sx = scaleFactors.first;
        const qreal sy = scaleFactors.second;
        emit scaled(sx, sy);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        updateScale(sx, sy);
    } else {
        QGraphicsView::wheelEvent(event);
    }
}
