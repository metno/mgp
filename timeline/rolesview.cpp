#include "rolesview.h"
#include "rolesscene.h"
#include "wheelscaler.h"
#include "common.h"
#include <QResizeEvent>

RolesView::RolesView(RolesScene *lhScene, QWidget *parent)
    : QGraphicsView(lhScene, parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::ScrollHandDrag);
}

void RolesView::updateScale(qreal, qreal sy)
{
    setTransform(QTransform::fromScale(1.0, sy)); // scale vertical dimension only
    qobject_cast<RolesScene *>(scene())->updateGeometry();
}

void RolesView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}
