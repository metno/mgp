#ifndef TASKITEM_H
#define TASKITEM_H

#include <QGraphicsRectItem>
#include <QColor>
#include <QBrush>

class QGraphicsTextItem;

class TaskItem : public QGraphicsRectItem
{
public:
    TaskItem(qint64);
    qint64 taskId() const;
    qint64 roleId() const;
    void highlight(bool);
    void updateRect(const QRectF &);

private:
    qint64 taskId_;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
    QGraphicsRectItem *hoverItem_;
    QGraphicsTextItem *nameItem_;
};

#endif // TASKITEM_H
