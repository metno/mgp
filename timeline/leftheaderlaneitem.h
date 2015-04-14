#ifndef LEFTHEADERLANEITEM_H
#define LEFTHEADERLANEITEM_H

#include <QGraphicsRectItem>

class QRectF;
class QGraphicsTextItem;

class LeftHeaderLaneItem : public QGraphicsRectItem
{
public:
    LeftHeaderLaneItem(qint64);
    qint64 roleId() const { return roleId_; }
    void updateRect(const QRectF &);
private:
    qint64 roleId_;
    QGraphicsTextItem *nameItem_;
    QGraphicsTextItem *timeItem_;
};

#endif // LEFTHEADERLANEITEM_H
