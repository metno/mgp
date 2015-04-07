#include "wheelscaler.h"
#include "common.h"
#include <QGraphicsView>
#include <QWheelEvent>

QPair<qreal, qreal> WheelScaler::exec(QGraphicsView *view, QWheelEvent *event)
{
    const bool scaleHorizontal = event->modifiers() & Qt::ShiftModifier;
    const bool scaleVertical = event->modifiers() & Qt::ControlModifier;

    if (scaleHorizontal || scaleVertical) {
        const qreal sfactBase = 1.01;
        const qreal sfact = (event->delta() > 0) ? sfactBase : (1.0 / sfactBase);
        qreal m11 = view->transform().m11(); // horizontal scaling factor
        qreal m22 = view->transform().m22(); // vertical scaling factor

        if (scaleHorizontal)
            m11 = qMin(qMax(sfact * m11, 0.1), 4.0);
        if (scaleVertical)
            m22 = qMin(qMax(sfact * m22, 0.1), 4.0);

        return qMakePair(m11, m22);
    }

    return qMakePair(-1.0, -1.0);
}
