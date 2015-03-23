#ifndef LANEBGITEM_H
#define LANEBGITEM_H

#include <QGraphicsRectItem>

class LaneBGItem : public QGraphicsRectItem
{
public:
    LaneBGItem(qint64);
    qint64 roleId() const { return roleId_; }
private:
    qint64 roleId_;
};

#endif // LANEBGITEM_H
