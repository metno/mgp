#include "taskitem.h"
#include "taskmanager.h"
#include "common.h"
#include <QCursor>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsView>

TaskItem::TaskItem(qint64 taskId__)
    : taskId_(taskId__)
{
    colors_.append(QColor(Qt::red));
    colors_.append(QColor(Qt::green));
    colors_.append(QColor(Qt::blue));
    colors_.append(QColor(Qt::cyan));
    colors_.append(QColor(Qt::magenta));
    colors_.append(QColor(Qt::black));
    colors_.append(QColor(Qt::gray));

    setRandomColor();
    setZValue(10);
    setCursor(Qt::ArrowCursor);
    setAcceptHoverEvents(true);

    hoverItem_ = new QGraphicsRectItem(this);
    hoverItem_->setFlag(QGraphicsItem::ItemStacksBehindParent);
    hoverItem_->setBrush(QColor("#ff0"));
    hoverItem_->setPen(QPen(QColor("#880")));
    hoverItem_->setVisible(false);
}

qint64 TaskItem::taskId() const
{
    return taskId_;
}

qint64 TaskItem::roleId() const
{
    QSharedPointer<Task> task = TaskManager::instance()->findTask(taskId());
    return task ? task->roleId() : -1;
}

void TaskItem::setRandomColor()
{
    QColor newColor;
    do {
        newColor = colors_.at(qrand() % colors_.size());
    } while (newColor == brush().color());
    setBrush(QBrush(newColor));
}

void TaskItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    qDebug() << "mousePressEvent for task" << taskId_;
    setRandomColor();
}

void TaskItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    if (!scene()->views().isEmpty()) {
        const qreal w = 5;
        const qreal addh = w / scene()->views().first()->transform().m11();
        const qreal addv = w / scene()->views().first()->transform().m22();
        hoverItem_->setRect(rect().adjusted(-addh, -addv, addh, addv));
    }

    hoverItem_->setVisible(true);
}

void TaskItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    hoverItem_->setVisible(false);
}
