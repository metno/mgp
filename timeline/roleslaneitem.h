#ifndef ROLESLANEITEM_H
#define ROLESLANEITEM_H

#include <QGraphicsRectItem>

class QRectF;
class QTime;
class QGraphicsTextItem;

class RolesLaneItem : public QGraphicsRectItem
{
public:
    RolesLaneItem(qint64);
    qint64 roleId() const { return roleId_; }
    void updateRect(const QRectF &);
    void updateProperties();

private:
    qint64 roleId_;
    QGraphicsTextItem *nameItem_;
    QGraphicsTextItem *timeItem_;
};

#endif // ROLESLANEITEM_H
