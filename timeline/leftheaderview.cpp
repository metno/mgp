#include "leftheaderview.h"
#include "leftheaderscene.h"
#include "wheelscaler.h"
#include "common.h"
#include <QResizeEvent>

LeftHeaderView::LeftHeaderView(LeftHeaderScene *lhScene, QWidget *parent)
    : QGraphicsView(lhScene, parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void LeftHeaderView::updateScale(qreal, qreal sy)
{
    setTransform(QTransform::fromScale(1.0, sy)); // scale vertical dimension only
}

void LeftHeaderView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}

void LeftHeaderView::wheelEvent(QWheelEvent *event)
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
