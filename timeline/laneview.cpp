#include "laneview.h"
#include "lanescene.h"
#include "rolesview.h"
#include "rolesscene.h"
#include "wheelscaler.h"
#include "common.h"
#include <QResizeEvent>
#include <QScrollBar>
#include <QRectF>

LaneView::LaneView(LaneScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
    , panning_(false)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    connect(this, SIGNAL(scaled(qreal, qreal)), dynamic_cast<RolesView *>(scene->rolesScene_->views().first()), SLOT(updateScale(qreal, qreal)));
}

void LaneView::updateScale(qreal sx, qreal sy)
{
    setTransform(QTransform::fromScale(sx, sy));
    emit scaled(sx, sy);
}

void LaneView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
}

void LaneView::wheelEvent(QWheelEvent *event)
{
    const QPair<qreal, qreal> scaleFactors = WheelScaler::exec(this, event);
    if (scaleFactors.first > 0) {
        const QPoint vpos = mapFromGlobal(QCursor::pos());
        const QPointF spos = mapToScene(vpos);
        const QRectF srect = scene()->sceneRect();
        const qreal hsmfrac = (spos.x() - srect.x()) / srect.width(); // horizontal scene frac at mouse pos
        const qreal vsmfrac = (spos.y() - srect.y()) / srect.height(); // vertical scene frac at mouse pos

        const qreal sx = scaleFactors.first;
        const qreal sy = scaleFactors.second;
        updateScale(sx, sy);

        // update view to focus on the part of the scene that was under the mouse before scaling
        // ### this feels only partially intuitive, but maybe it is good enough?
        horizontalScrollBar()->setValue(horizontalScrollBar()->minimum() + hsmfrac * (horizontalScrollBar()->maximum() - horizontalScrollBar()->minimum()));
        verticalScrollBar()->setValue(verticalScrollBar()->minimum() + vsmfrac * (verticalScrollBar()->maximum() - verticalScrollBar()->minimum()));

    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void LaneView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        panning_ = true;
        panPrevX_ = event->x();
        panPrevY_ = event->y();
        setCursor(Qt::ClosedHandCursor);
    }
    QGraphicsView::mousePressEvent(event);
}

void LaneView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
    {
        panning_ = false;
        setCursor(Qt::ArrowCursor);
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void LaneView::mouseMoveEvent(QMouseEvent *event)
{
    if (panning_)
    {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->x() - panPrevX_));
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->y() - panPrevY_));
        panPrevX_ = event->x();
        panPrevY_ = event->y();
    }
    QGraphicsView::mouseMoveEvent(event);
}
