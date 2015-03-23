#ifndef LANEHEADERBGITEM_H
#define LANEHEADERBGITEM_H

#include <QGraphicsRectItem>

class LaneHeaderBGItem : public QGraphicsRectItem
{
public:
    LaneHeaderBGItem(qint64);
    qint64 roleId() const { return roleId_; }
private:
    qint64 roleId_;
};

#endif // LANEHEADERBGITEM_H
