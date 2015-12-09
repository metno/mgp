#include "lanescene.h"
#include "rolesscene.h"
#include "laneitem.h"
#include "taskitem.h"
#include "taskmanager.h"
#include "common.h"
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>

LaneScene::LaneScene(RolesScene *rolesScene, const QDate &baseDate__, int dateSpan__, QObject *parent)
    : QGraphicsScene(0, 0, dateSpan__ * secsInDay(), rolesScene->height(), parent)
    , rolesScene_(rolesScene)
    , baseDate_(baseDate__)
    , dateSpan_(dateSpan__)
    , currTimeMarker_(0)
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

void LaneScene::updateRoleTimeItems()
{
    // clear items for existing range
    foreach (QGraphicsRectItem *roleTimeItem, roleTimeItems_)
        delete roleTimeItem;
    roleTimeItems_.clear();

    // add items for new range
    for (int i = 0; i < dateSpan_; ++i) {
        foreach (LaneItem *laneItem, laneItems()) {
            QGraphicsRectItem *roleTimeItem = new QGraphicsRectItem;
            roleTimeItem->setBrush(QBrush(QColor("#ffff00")));
            roleTimeItem->setOpacity(0.4);
            roleTimeItem->setZValue(3);
            addItem(roleTimeItem);
            roleTimeItems_.append(roleTimeItem);
        }
    }
}

void LaneScene::setDateRange(const QDate &baseDate__, int dateSpan__)
{
    // clear date- and time items for existing range
    foreach (QGraphicsRectItem *dateItem, dateItems_)
        delete dateItem;
    dateItems_.clear();
    foreach (QGraphicsLineItem *timeItem, timeItems_)
        delete timeItem;
    timeItems_.clear();

    baseDate_ = baseDate__;
    dateSpan_ = dateSpan__;

    // update scene rect width
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), dateSpan_ * secsInDay(), srect.height());

    // add items for new range
    for (int i = 0; i < dateSpan_; ++i) {
        // add date item
        QGraphicsRectItem *dateItem = new QGraphicsRectItem;
        dateItem->setBrush(QBrush(QColor((i % 2) ? "#eeeeee" : "#cccccc")));
        dateItem->setOpacity(0.4);
        dateItem->setZValue(2);
        addItem(dateItem);
        dateItems_.append(dateItem);

        // add time items
        for (int j = 0; j < 23; ++j) {
            QGraphicsLineItem *timeItem = new QGraphicsLineItem;
            timeItem->setPen(QPen(QColor("#ccc")));
            timeItem->setZValue(4);
            addItem(timeItem);
            timeItems_.append(timeItem);
        }
    }

    updateGeometry();

    emit dateRangeChanged();
}

void LaneScene::updateItemGeometry()
{
    const qreal lvpad = rolesScene_->laneVerticalPadding();
    const qreal lheight = rolesScene_->laneHeight();

    for (int i = 0; i < dateSpan_; ++i) {
        // update date item
        const long date_x = sceneRect().x() + i * secsInDay();
        const long y = sceneRect().y();
        const long h = sceneRect().height();
        dateItems_.at(i)->setRect(QRectF(date_x, y, secsInDay(), h));

        // update time items
        for (int j = 0; j < 23; ++j) {
            const long time_x = date_x + (j + 1) * secsInHour();
            timeItems_.at(i * 23 + j)->setLine(time_x, y, time_x, y + h);
        }

        // update role time items

        for (int j = 0; j < laneItems().size(); ++j) {
            LaneItem *lItem = laneItems().at(j);

            const QTime btime = TaskManager::instance()->findRole(lItem->roleId())->beginTime();
            const long bsecs = btime.hour() * 3600 + btime.minute() * 60 + btime.second();

            const QTime etime = TaskManager::instance()->findRole(lItem->roleId())->endTime();
            long esecs = etime.hour() * 3600 + etime.minute() * 60 + etime.second();
            if (esecs <= bsecs)
                esecs += secsInDay();

            const qreal bx = date_x + bsecs;
            const qreal ex = date_x + esecs;
            roleTimeItems_.at(i * laneItems().size() + j)->setRect(QRect(bx, j * lheight + lvpad, ex - bx + 1, lheight - lvpad));
        }
    }
}

void LaneScene::updateCurrTimeMarker()
{
    if (!currTimeMarker_) {
        currTimeMarker_ = new QGraphicsLineItem;
        currTimeMarker_->setPen(QPen(Qt::red));
        currTimeMarker_->setZValue(3);
        addItem(currTimeMarker_);
    }

    const long loSceneTimestamp = QDateTime(baseDate_, QTime(0, 0)).toTime_t();
    const long hiSceneTimestamp = QDateTime(baseDate_.addDays(dateSpan_), QTime(0, 0)).toTime_t();
    Q_ASSERT(loSceneTimestamp < hiSceneTimestamp);
    const qreal sceneTimeFact = 1.0 / (hiSceneTimestamp - loSceneTimestamp);
    const QRectF srect = sceneRect();

    const long currTimestamp = QDateTime::currentDateTime().toTime_t();
    const qreal x = (currTimestamp - loSceneTimestamp) * sceneTimeFact * qreal(srect.width());
    currTimeMarker_->setLine(x, srect.y(), x, srect.height());

}

void LaneScene::updateFromTaskMgr()
{
    const QList<qint64> tmRoleIds = TaskManager::instance()->roleIds();

    // remove lane items for roles that no longer exist in the task manager
    foreach (LaneItem *lItem, laneItems()) {
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
    // note: we assume that rolesScene_ is up to date at this point

    // update scene rect height
    {
        const QRectF srect = sceneRect();
        setSceneRect(srect.x(), srect.y(), srect.width(), laneItems().size() * rolesScene_->laneHeight() + rolesScene_->laneVerticalPadding());
    }

    updateRoleTimeItems();

    updateItemGeometry();

    const qreal lvpad = rolesScene_->laneVerticalPadding();
    const qreal lheight = rolesScene_->laneHeight();
    const long loSceneTimestamp = QDateTime(baseDate_, QTime(0, 0)).toTime_t();
    const long hiSceneTimestamp = QDateTime(baseDate_.addDays(dateSpan_), QTime(0, 0)).toTime_t();
    Q_ASSERT(loSceneTimestamp < hiSceneTimestamp);
    const qreal sceneTimeFact = 1.0 / (hiSceneTimestamp - loSceneTimestamp);
    const QRectF srect = sceneRect();

    int i = 0;
    foreach (LaneItem *lItem, laneItems()) {
        // update lane item rect
        lItem->setRect(0, i * lheight + lvpad, width(), lheight - lvpad);

        // update task item rects
        foreach(TaskItem *tItem, taskItems(lItem->roleId())) {
            QSharedPointer<Task> task = TaskManager::instance()->findTask(tItem->taskId());
            const long loTimestamp = task->loDateTime().toTime_t();
            const long hiTimestamp = task->hiDateTime().toTime_t();
            Q_ASSERT(loTimestamp < hiTimestamp);
            const qreal x1 = (loTimestamp - loSceneTimestamp) * sceneTimeFact * qreal(srect.width());
            const qreal x2 = (hiTimestamp - loSceneTimestamp) * sceneTimeFact * qreal(srect.width());
            tItem->setRect(x1, i * lheight + 2 * lvpad, x2 - x1, lheight - 4 * lvpad);
        }

        i++;
    }

    updateCurrTimeMarker(); // ### called here for now; eventually to be called automatically every 10 secs or so
}

QList<LaneItem *> LaneScene::laneItems() const
{
    QList<LaneItem *> lItems;
    foreach (QGraphicsItem *item, items()) {
        LaneItem *lItem = dynamic_cast<LaneItem *>(item);
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
        LaneItem *lItem = dynamic_cast<LaneItem *>(item);
        if (lItem)
            liRoleIds.append(lItem->roleId());
    }
    return liRoleIds;
}

void LaneScene::addLaneItem(qint64 roleId)
{
    addItem(new LaneItem(roleId));
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
