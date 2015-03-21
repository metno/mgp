#ifndef LANEVIEW_H
#define LANEVIEW_H

#include <QGraphicsView>

class LaneScene;

class LaneView : public QGraphicsView
{
    Q_OBJECT
public:
    LaneView(LaneScene *, QWidget * = 0);
protected:
    void resizeEvent(QResizeEvent *);
};

#endif // LANEVIEW_H
