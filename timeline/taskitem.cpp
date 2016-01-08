#include "taskitem.h"
#include "taskmanager.h"
#include "common.h"
#include <QCursor>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QAction>
#include <QMenu>
#include <QCursor>
#include <QGraphicsTextItem>
#include <QFont>

TaskItem::TaskItem(qint64 taskId__)
    : taskId_(taskId__)
{
    setBrush(QColor("#bbb"));
    setCursor(Qt::ArrowCursor);
    setAcceptHoverEvents(true);

    hoverItem_ = new QGraphicsRectItem(this);
    hoverItem_->setFlag(QGraphicsItem::ItemStacksBehindParent);
    hoverItem_->setBrush(QColor("#ff0"));
    hoverItem_->setPen(QPen(QColor("#880")));
    hoverItem_->setVisible(false);

    QSharedPointer<Task> task = TaskManager::instance().findTask(taskId_);
    Q_ASSERT(task);

    nameItem_ = new QGraphicsTextItem(task->name(), this);
    nameItem_->setFont(QFont("helvetica", 14, QFont::Bold));
    nameItem_->setZValue(2);
    nameItem_->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    //nameItem_->setPos(rect().x() + 5, rect().y());

    summaryItem_ = new QGraphicsTextItem(task->summary(), this);
    summaryItem_->setFont(QFont("helvetica", 12, QFont::Normal));
    summaryItem_->setZValue(2);
    summaryItem_->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    //summaryItem_->setPos(rect().x() + 5, rect().y() + 5);

    updateTextPositions();
}

qint64 TaskItem::taskId() const
{
    return taskId_;
}

qint64 TaskItem::roleId() const
{
    QSharedPointer<Task> task = TaskManager::instance().findTask(taskId());
    return task ? task->roleId() : -1;
}

void TaskItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
//    qDebug() << "mousePressEvent for task" << taskId_;
}

void TaskItem::highlight(bool enabled)
{
    if (enabled) {
        Q_ASSERT(!scene()->views().isEmpty());
        const qreal w = 5;
        const qreal addh = w / scene()->views().first()->transform().m11();
        const qreal addv = w / scene()->views().first()->transform().m22();
        hoverItem_->setRect(rect().adjusted(-addh, -addv, addh, addv));
    }

    hoverItem_->setVisible(enabled);
}

void TaskItem::updateRect(const QRectF &r)
{
    setRect(r);
    summaryItem_->setTextWidth(scene()->views().first()->transform().m11() * r.width());
    updateTextPositions();
}

void TaskItem::updateName(const QString &name)
{
    nameItem_->setPlainText(name);
}

void TaskItem::updateSummary(const QString &summary)
{
    summaryItem_->setPlainText(summary);
}

void TaskItem::updateDescription(const QString &descr)
{
    Q_UNUSED(descr);
//    descrItem_->setPlainText(descr);
}

void TaskItem::updateTextPositions()
{
    int addh = 5; // for now
    int addv_summary = 25; // for now (depends on font size etc.)
    if (scene() && (!scene()->views().isEmpty())) {
        addh /= scene()->views().first()->transform().m11();
        addv_summary /= scene()->views().first()->transform().m22();
    }
    const int addv_name = addh;

    nameItem_->setPos(rect().x() + addh, rect().y() + addv_name);
    summaryItem_->setPos(rect().x() + addh, rect().y() + addv_summary);
}
