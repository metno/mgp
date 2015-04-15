#include "laneview.h"
#include "lanescene.h"
#include "leftheaderview.h"
#include "leftheaderscene.h"
#include "wheelscaler.h"
#include "common.h"
#include <QResizeEvent>
#include <QScrollBar>
#include <QRectF>

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
