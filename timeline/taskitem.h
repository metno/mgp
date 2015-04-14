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
    int colIndex_;
    QBrush origBrush_;
    void setRandomColor();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
};

#endif // TASKITEM_H
