#include "taskitem.h"
#include "taskmanager.h"
#include "common.h"
#include <QBrush>
#include <QCursor>

TaskItem::TaskItem(qint64 taskId__)
    : taskId_(taskId__)
    , colIndex_(-1)
{
    colors_.append(QColor(Qt::red));
    colors_.append(QColor(Qt::green));
    colors_.append(QColor(Qt::blue));
    colors_.append(QColor(Qt::cyan));
    colors_.append(QColor(Qt::magenta));
    colors_.append(QColor(Qt::yellow));
    colors_.append(QColor(Qt::black));
    colors_.append(QColor(Qt::white));

    setRandomColor();
    setZValue(10);
    setCursor(Qt::ArrowCursor);
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
    int newColIndex;
    do {
        newColIndex = qrand() % colors_.size();
    } while (newColIndex == colIndex_);
    colIndex_ = newColIndex;
    setBrush(QBrush(colors_.at(colIndex_)));
}

void TaskItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    qDebug() << "mousePressEvent for task" << taskId_;
    setRandomColor();
}
