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

void LeftHeaderView::updateScale(qreal sx, qreal sy)
{
    setTransform(QTransform::fromScale(sx, sy));
}

void LeftHeaderView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}

void LeftHeaderView::wheelEvent(QWheelEvent *event)
{
    const QPair<qreal, qreal> scaleFactors = WheelScaler::exec(this, event);
    if (scaleFactors.first > 0)
        emit scaled(scaleFactors.first, scaleFactors.second);
    else
        QGraphicsView::wheelEvent(event);
}
