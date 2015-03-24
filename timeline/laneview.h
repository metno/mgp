#ifndef LANEVIEW_H
#define LANEVIEW_H

#include <QGraphicsView>

class LaneScene;

class LaneView : public QGraphicsView
{
    Q_OBJECT

public:
    LaneView(LaneScene *, QWidget * = 0);

public slots:
    void updateScale(qreal);

private:
    virtual void resizeEvent(QResizeEvent *);
    virtual void wheelEvent(QWheelEvent *);

signals:
    void scaled(qreal);
};

#endif // LANEVIEW_H
