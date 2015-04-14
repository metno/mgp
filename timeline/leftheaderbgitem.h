#ifndef LEFTHEADERBGITEM_H
#define LEFTHEADERBGITEM_H

#include <QGraphicsRectItem>

class QRectF;
class QGraphicsTextItem;

class LeftHeaderBGItem : public QGraphicsRectItem
{
public:
    LeftHeaderBGItem(qint64);
    qint64 roleId() const { return roleId_; }
    void updateRect(const QRectF &);
private:
    qint64 roleId_;
    QGraphicsTextItem *nameItem_;
};

#endif // LEFTHEADERBGITEM_H
