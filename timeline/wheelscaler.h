#ifndef WHEELSCALER_H
#define WHEELSCALER_H

#include <QtGlobal>
#include <QPair>

class QGraphicsView;
class QWheelEvent;

class WheelScaler
{
public:
    static QPair<qreal, qreal> exec(QGraphicsView *, QWheelEvent *, qreal, qreal, qreal, qreal);
};

#endif // WHEELSCALER_H
