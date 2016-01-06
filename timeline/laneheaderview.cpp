#include "laneheaderview.h"
#include "laneheaderscene.h"
#include "wheelscaler.h"
#include "rolepanel.h"
#include "common.h"
#include "mainwindow.h"
#include <QResizeEvent>

LaneHeaderView::LaneHeaderView(LaneHeaderScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    connect(this, SIGNAL(viewLeft()), scene, SLOT(handleViewLeft()));
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

void LaneHeaderView::keyPressEvent(QKeyEvent *event)
{
    MainWindow::instance().handleKeyPressEvent(event); // to handle Control+Q for closing application etc.
    QGraphicsView::keyPressEvent(event); // propagate to scene
}

void LaneHeaderView::leaveEvent(QEvent *)
{
    RolePanel::instance().clearContents();
    emit viewLeft();
}
