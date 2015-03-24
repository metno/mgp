#ifndef WHEELSCALER_H
#define WHEELSCALER_H

#include <QtGlobal>

class QGraphicsView;
class QWheelEvent;

class WheelScaler
{
public:
    static qreal exec(QGraphicsView *, QWheelEvent *);
};

#endif // WHEELSCALER_H
