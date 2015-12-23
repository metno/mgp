#include "rolesview.h"
#include "rolesscene.h"
#include "wheelscaler.h"
#include "common.h"
#include <QResizeEvent>

RolesView::RolesView(RolesScene *rScene, QWidget *parent)
    : QGraphicsView(rScene, parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void RolesView::updateScale(qreal sx, qreal)
{
    setTransform(QTransform::fromScale(sx, 1.0)); // scale horizontal dimension only
    qobject_cast<RolesScene *>(scene())->updateGeometryAndContents();
}

void RolesView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}
