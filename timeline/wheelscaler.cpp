#include "wheelscaler.h"
#include <QGraphicsView>
#include <QWheelEvent>

qreal WheelScaler::exec(QGraphicsView *view, QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        const qreal scaleFactor = 1.15;
        const qreal val = (event->delta() > 0) ? scaleFactor : (1.0 / scaleFactor);
        view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        view->scale(val, val);
        return val;
    }

    return -1.0;
}
