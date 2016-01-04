#include "laneheaderview.h"
#include "laneheaderscene.h"
#include "wheelscaler.h"
#include "rolepanel.h"
#include "common.h"
#include <QResizeEvent>

LaneHeaderView::LaneHeaderView(LaneHeaderScene *rScene, QWidget *parent)
    : QGraphicsView(rScene, parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void LaneHeaderView::updateScale(qreal sx, qreal)
{
    setTransform(QTransform::fromScale(sx, 1.0)); // scale horizontal dimension only
    qobject_cast<LaneHeaderScene *>(scene())->updateGeometryAndContents();
}

void LaneHeaderView::resizeEvent(QResizeEvent *event)
{
    emit resized();
    QGraphicsView::resizeEvent(event);
}

void LaneHeaderView::leaveEvent(QEvent *)
{
    RolePanel::instance().clearContents();
}
