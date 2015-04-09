#include "lanescene.h"
#include "leftheaderscene.h"
#include "lanebgitem.h"
#include "taskitem.h"
#include "taskmanager.h"
#include "common.h"
#include <QGraphicsRectItem>

LaneScene::LaneScene(LeftHeaderScene *leftHeaderScene, const QDate &baseDate__, int dateSpan__, QObject *parent)
    : QGraphicsScene(0, 0, dateSpan__ * dateWidth(), leftHeaderScene->height(), parent)
    , leftHeaderScene_(leftHeaderScene)
    , baseDate_(baseDate__)
    , dateSpan_(dateSpan__)
{
    setDateRange(baseDate_, dateSpan_);
}

QDate LaneScene::baseDate() const
{
    return baseDate_;
}

int LaneScene::dateSpan() const
{
    return dateSpan_;
}

qreal LaneScene::dateWidth()
{
    return 1000;
}

void LaneScene::setDateRange(const QDate &baseDate__, int dateSpan__)
{
    // clear items for existing range
    foreach (QGraphicsRectItem *dateItem, dateItems_)
        delete dateItem;
    dateItems_.clear();

    baseDate_ = baseDate__;
    dateSpan_ = dateSpan__;

    // update scene rect width
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), dateSpan_ * dateWidth(), srect.height());

    // add items for new range
    for (int i = 0; i < dateSpan_; ++i) {
        QGraphicsRectItem *dateItem = new QGraphicsRectItem;
        dateItem->setBrush(QBrush(QColor((i % 2) ? "#eeeeee" : "#cccccc")));
        dateItem->setOpacity(0.4);
        dateItem->setZValue(2);
        addItem(dateItem);
        dateItems_.append(dateItem);
    }

    updateGeometry();

    emit dateRangeChanged();
}

void LaneScene::updateDateItemGeometries()
{
    for (int i = 0; i < dateSpan_; ++i) {
        const qreal x = sceneRect().x() + i * dateWidth();
        const qreal y = sceneRect().y();
        const qreal w = dateWidth();
        const qreal h = sceneRect().height();
        dateItems_.at(i)->setRect(QRectF(x, y, w, h));
    }
}

void LaneScene::updateFromTaskMgr()
{
    const QList<qint64> tmRoleIds = TaskManager::instance()->roleIds();

    // remove lane items for roles that no longer exist in the task manager
    foreach (LaneBGItem *lItem, laneItems()) {
        if (!tmRoleIds.contains(lItem->roleId()))
            removeItem(lItem);
    }

    // add lane items for unrepresented roles in the task manager
    const QList<qint64> liRoleIds = laneItemRoleIds();
    foreach (qint64 tmRoleId, tmRoleIds) {
        if (!liRoleIds.contains(tmRoleId))
            addLaneItem(tmRoleId);
    }

    const QList<qint64> tmTaskIds = TaskManager::instance()->taskIds();

    // remove task items for tasks that no longer exist in the task manager
    foreach (TaskItem *tItem, taskItems()) {
        if (!tmTaskIds.contains(tItem->taskId()))
            removeItem(tItem);
    }

    // add task items for unrepresented roles in the task manager
    const QList<qint64> tiRoleIds = taskItemRoleIds();
    foreach (qint64 tmRoleId, tmRoleIds) {
        if (!tiRoleIds.contains(tmRoleId))
            addTaskItems(tmRoleId);
    }

    updateGeometry();
}

void LaneScene::updateGeometry()
{
    // note: we assume that leftHeaderScene_ is up to date at this point

    // update scene rect height
    {
        const QRectF srect = sceneRect();
        setSceneRect(srect.x(), srect.y(), srect.width(), laneItems().size() * leftHeaderScene_->laneHeight() + leftHeaderScene_->lanePadding());
    }

    updateDateItemGeometries();

    const qreal lpadding = leftHeaderScene_->lanePadding();
    const qreal lheight = leftHeaderScene_->laneHeight();
    const long loSceneTime = QDateTime(baseDate_, QTime(0, 0)).toUTC().toUTC().toTime_t();
    const long hiSceneTime = QDateTime(baseDate_.addDays(dateSpan_), QTime(23, 59, 59)).toUTC().toTime_t();
    Q_ASSERT(loSceneTime < hiSceneTime);
    qDebug() << loSceneTime << hiSceneTime;
    const qreal sceneTimeFact = 1.0 / (hiSceneTime - loSceneTime);
    const QRectF srect = sceneRect();
    int i = 0;
    foreach (LaneBGItem *lItem, laneItems()) {
        // update lane item rect
        lItem->setPos(0, 0);
        lItem->setRect(lpadding, i * lheight + lpadding, width() - 2 * lpadding, lheight - lpadding);

        // update task item rects
        foreach(TaskItem *tItem, taskItems(lItem->roleId())) {
            QSharedPointer<Task> task = TaskManager::instance()->findTask(tItem->taskId());
            const long loTime = task->loDateTime().toUTC().toTime_t();
            const long hiTime = task->hiDateTime().toUTC().toTime_t();
            Q_ASSERT(loTime < hiTime);

            const qreal x1 = srect.x() + (loTime - loSceneTime) * sceneTimeFact * qreal(srect.width());
            const qreal x2 = srect.x() + (hiTime - loSceneTime) * sceneTimeFact * qreal(srect.width());

            //tItem->setPos(x1, 0);
            tItem->setRect(x1, i * lheight + 2 * lpadding, (x2 - x1), lheight - 4 * lpadding);

        }

        i++;
    }
}

QList<LaneBGItem *> LaneScene::laneItems() const
{
    QList<LaneBGItem *> lItems;
    foreach (QGraphicsItem *item, items()) {
        LaneBGItem *lItem = dynamic_cast<LaneBGItem *>(item);
        if (lItem)
            lItems.append(lItem);
    }
    return lItems;
}

// ### consider turning this into a template function (see taskItemRoleIds())
QList<qint64> LaneScene::laneItemRoleIds() const
{
    QList<qint64> liRoleIds;
    foreach (QGraphicsItem *item, items()) {
        LaneBGItem *lItem = dynamic_cast<LaneBGItem *>(item);
        if (lItem)
            liRoleIds.append(lItem->roleId());
    }
    return liRoleIds;
}

void LaneScene::addLaneItem(qint64 roleId)
{
    addItem(new LaneBGItem(roleId));
}

QList<TaskItem *> LaneScene::taskItems(qint64 roleId) const
{
    QList<TaskItem *> tItems;
    foreach (QGraphicsItem *item, items()) {
        TaskItem *tItem = dynamic_cast<TaskItem *>(item);
        if (tItem && ((roleId < 0) || (tItem->roleId() == roleId)))
            tItems.append(tItem);
    }
    return tItems;
}

// ### consider turning this into a template function (see laneItemRoleIds())
QList<qint64> LaneScene::taskItemRoleIds() const
{
    QList<qint64> tiRoleIds;
    foreach (QGraphicsItem *item, items()) {
        TaskItem *tItem = dynamic_cast<TaskItem *>(item);
        if (tItem)
            tiRoleIds.append(tItem->roleId());
    }
    return tiRoleIds;
}

void LaneScene::addTaskItems(qint64 roleId)
{
    foreach (qint64 taskId, TaskManager::instance()->assignedTasks(roleId))
        addItem(new TaskItem(taskId));
}
