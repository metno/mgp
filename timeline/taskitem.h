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
    void highlight(bool);
private:
    qint64 taskId_;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
    QGraphicsRectItem *hoverItem_;
};

#endif // TASKITEM_H
