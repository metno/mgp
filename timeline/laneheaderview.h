#ifndef LANEHEADERVIEW_H
#define LANEHEADERVIEW_H

#include <QGraphicsView>

class LaneHeaderScene;

// ### NOTE: Any reference to the term 'role' in this file (+ the .cpp file)
// is based on the assumption that a lane always represents a role. In a later
// version, a lane shows the tasks that match a user-defined condition/filter.
// The role responsible for the tasks in the lane may be part of the condition
// (and this is expected to be the most used condition in practice), but the
// condition may be more general.

class LaneHeaderView : public QGraphicsView
{
    Q_OBJECT

public:
    LaneHeaderView(LaneHeaderScene *, QWidget * = 0);

public slots:
    void updateScale(qreal, qreal);

private:
    virtual void resizeEvent(QResizeEvent *);
    virtual void leaveEvent(QEvent *);

signals:
    void resized();
};

#endif // LANEHEADERVIEW_H
