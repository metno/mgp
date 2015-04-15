#ifndef TASKITEM_H
#define TASKITEM_H

#include <QGraphicsRectItem>
#include <QColor>
#include <QBrush>

class TaskItem : public QGraphicsRectItem
{
public:
    TaskItem(qint64);
    qint64 taskId() const;
    qint64 roleId() const;
private:
    qint64 taskId_;
    QList<QColor> colors_;
    void setRandomColor();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
    QGraphicsRectItem *hoverItem_;
};

#endif // TASKITEM_H
