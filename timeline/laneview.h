#ifndef LANEVIEW_H
#define LANEVIEW_H

#include <QGraphicsView>

class LaneScene;

class LaneView : public QGraphicsView
{
    Q_OBJECT
public:
    LaneView(LaneScene *, QWidget * = 0);
private:
    void resizeEvent(QResizeEvent *);
};

#endif // LANEVIEW_H
