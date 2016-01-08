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
    void updateHScale(qreal);
    void updateVScale(qreal);

private:
    virtual void resizeEvent(QResizeEvent *);
    virtual void wheelEvent(QWheelEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    virtual void leaveEvent(QEvent *);

    bool panning_;
    int panPrevX_;
    int panPrevY_;
    qreal lastScaleX_;
    qreal lastScaleY_;

signals:
    void scaled(qreal, qreal);
    void viewLeft();
};

#endif // LANEVIEW_H
