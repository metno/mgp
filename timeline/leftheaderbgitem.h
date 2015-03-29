#ifndef LEFTHEADERBGITEM_H
#define LEFTHEADERBGITEM_H

#include <QGraphicsRectItem>

class LeftHeaderBGItem : public QGraphicsRectItem
{
public:
    LeftHeaderBGItem(qint64);
    qint64 roleId() const { return roleId_; }
private:
    qint64 roleId_;
};

#endif // LEFTHEADERBGITEM_H
