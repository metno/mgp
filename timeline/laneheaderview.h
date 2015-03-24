#ifndef LANEHEADERVIEW_H
#define LANEHEADERVIEW_H

#include <QGraphicsView>

class LaneHeaderScene;

class LaneHeaderView : public QGraphicsView
{
    Q_OBJECT

public:
    LaneHeaderView(LaneHeaderScene *, QWidget * = 0);

public slots:
    void updateScale(qreal);

private:
    virtual void resizeEvent(QResizeEvent *);
    virtual void wheelEvent(QWheelEvent *);

signals:
    void resized();
    void scaled(qreal);
};

#endif // LANEHEADERVIEW_H
