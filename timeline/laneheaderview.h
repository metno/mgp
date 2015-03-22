#ifndef LANEHEADERVIEW_H
#define LANEHEADERVIEW_H

#include <QGraphicsView>

class LaneHeaderScene;

class LaneHeaderView : public QGraphicsView
{
    Q_OBJECT
public:
    LaneHeaderView(LaneHeaderScene *, QWidget * = 0);
private:
    void resizeEvent(QResizeEvent *);
signals:
    void resized();
};

#endif // LANEHEADERVIEW_H
