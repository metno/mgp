#ifndef LANEHEADERITEM_H
#define LANEHEADERITEM_H

#include <QGraphicsRectItem>

class QRectF;
class QTime;
class QGraphicsTextItem;

// ### NOTE: Any reference to the term 'role' in this file (+ the .cpp file)
// is based on the assumption that a lane always represents a role. In a later
// version, a lane shows the tasks that match a user-defined condition/filter.
// The role responsible for the tasks in the lane may be part of the condition
// (and this is expected to be the most used condition in practice), but the
// condition may be more general.

class LaneHeaderItem : public QGraphicsRectItem
{
public:
    LaneHeaderItem(qint64);
    qint64 roleId() const { return roleId_; }
    void highlight(bool);
    void updateRect(const QRectF &);
    void updateProperties();

private:
    qint64 roleId_;
    QGraphicsTextItem *nameItem_;
    QGraphicsTextItem *timeItem_;
    QGraphicsTextItem *filterItem_;
    QGraphicsRectItem *hoverItem_;
};

#endif // LANEHEADERITEM_H
