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
    setDragMode(QGraphicsView::ScrollHandDrag);
}

void LeftHeaderView::updateScale(qreal, qreal sy)
{
    setTransform(QTransform::fromScale(1.0, sy)); // scale vertical dimension only
    qobject_cast<LeftHeaderScene *>(scene())->updateGeometry();
}

void LeftHeaderView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}
