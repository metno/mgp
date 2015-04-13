#ifndef TASKITEM_H
#define TASKITEM_H

#include <QGraphicsRectItem>
#include <QColor>

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
    void setRandomColor();
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
};

#endif // TASKITEM_H
