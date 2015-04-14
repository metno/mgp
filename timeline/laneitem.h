#ifndef LANEITEM_H
#define LANEITEM_H

#include <QGraphicsRectItem>

class LaneItem : public QGraphicsRectItem
{
public:
    LaneItem(qint64);
    qint64 roleId() const { return roleId_; }
private:
    qint64 roleId_;
};

#endif // LANEITEM_H
