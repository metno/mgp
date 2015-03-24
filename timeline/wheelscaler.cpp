#include "wheelscaler.h"
#include "common.h"
#include <QGraphicsView>
#include <QWheelEvent>

qreal WheelScaler::exec(QGraphicsView *view, QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        const qreal scaleFactor = 1.1;
        qreal val = (event->delta() > 0) ? scaleFactor : (1.0 / scaleFactor);
        const qreal m11 = view->transform().m11();
        if ((val * m11) < 1.0)
            val = 1.0 / m11;
        view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        view->scale(val, val);
        return val;
    }

    return -1.0;
}
