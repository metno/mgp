#include "taskitem.h"
#include "taskmanager.h"
#include "common.h"
#include <QBrush>
#include <QColor>
#include <QCursor>

TaskItem::TaskItem(qint64 taskId__)
    : taskId_(taskId__)
{
    setBrush(QBrush(QColor(128 + qrand() % 32, 128 + qrand() % 32, 128 + qrand() % 32)));
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

void TaskItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "mousePressEvent for task" << taskId_;
}
