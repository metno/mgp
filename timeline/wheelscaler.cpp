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
        view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        if ((val * m11) < 1.0) {
            view->setTransform(QTransform());
            // Q_ASSERT(view->transform().m11() == 1.0);
            return 1.0;
        }
        view->setTransform(QTransform::fromScale(val * m11, val * m11));
        return val * m11;
    }

    return -1.0;
}
