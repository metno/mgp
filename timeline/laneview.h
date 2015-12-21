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
    void updateScale(qreal, qreal);

private:
    virtual void resizeEvent(QResizeEvent *);
    virtual void wheelEvent(QWheelEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

    bool panning_;
    int panPrevX_;
    int panPrevY_;

signals:
    void scaled(qreal, qreal);
};

#endif // LANEVIEW_H
