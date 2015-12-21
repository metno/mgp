#include "lanescene.h"
#include "rolesscene.h"
#include "laneitem.h"
#include "taskitem.h"
#include "taskmanager.h"
#include "common.h"
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <QAction>
#include <QMenu>
#include <QSharedPointer>
#include <QDateTime>

LaneScene::LaneScene(RolesScene *rolesScene, const QDate &baseDate__, int dateSpan__, QObject *parent)
    : QGraphicsScene(0, 0, dateSpan__ * secsInDay(), rolesScene->height(), parent)
    , rolesScene_(rolesScene)
    , baseDate_(baseDate__)
    , dateSpan_(dateSpan__)
    , currTimeMarker_(0)
    , hoverTaskItem_(0)
    , currTaskItem_(0)
    , currLaneIndex_(-1)
    , insertTop_(-1)
    , insertBottom_(-1)
{
    setDateRange(baseDate_, dateSpan_);

    addTaskAction_ = new QAction("Add new task", 0);
    connect(addTaskAction_, SIGNAL(triggered()), SLOT(addTask()));

    editTaskAction_ = new QAction("Edit task", 0);
    connect(editTaskAction_, SIGNAL(triggered()), SLOT(editTask()));

    removeTaskAction_ = new QAction("Remove task", 0);
    connect(removeTaskAction_, SIGNAL(triggered()), SLOT(removeTask()));

    hoverTimeMarker_ = new QGraphicsLineItem;
    hoverTimeMarker_->setPen(QPen(QColor(0, 160, 0)));
    hoverTimeMarker_->setZValue(20);
    hoverTimeMarker_->setVisible(true);
    hoverTimeMarker_->setAcceptsHoverEvents(true);
    addItem(hoverTimeMarker_);

    hoverRoleMarker_ = new QGraphicsRectItem;
    hoverRoleMarker_->setBrush(QBrush(QColor(0, 255, 0, 16)));
    {
        QPen pen(QBrush(QColor(0, 160, 0)), 2);
        pen.setCosmetic(true);
        hoverRoleMarker_->setPen(pen);
    }
    hoverRoleMarker_->setZValue(20);
    hoverRoleMarker_->setVisible(true);
    addItem(hoverRoleMarker_);

    currTaskMarker_ = new QGraphicsRectItem;
    currTaskMarker_->setBrush(QBrush(QColor(255, 0, 0, 16)));
    {
        QPen pen(QBrush(QColor(255, 0, 0)), 2);
        pen.setCosmetic(true);
        currTaskMarker_->setPen(pen);
    }
    currTaskMarker_->setZValue(15);
    currTaskMarker_->setVisible(false);
    addItem(currTaskMarker_);
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
        for (int j = 0; j < laneItems().count(); ++j) {
            QGraphicsRectItem *roleTimeItem = new QGraphicsRectItem;
            roleTimeItem->setBrush(QBrush(QColor("#ffff00")));
            roleTimeItem->setOpacity(0.4);
            roleTimeItem->setZValue(3);
            addItem(roleTimeItem);
            roleTimeItems_.append(roleTimeItem);
        }
    }
}

// Returns the vertical position for a timestamp.
qreal LaneScene::timestampToVPos(long timestamp) const
{
    const long loSceneTimestamp = QDateTime(baseDate_, QTime(0, 0)).toTime_t();
    const long hiSceneTimestamp = QDateTime(baseDate_.addDays(dateSpan_), QTime(0, 0)).toTime_t();
    Q_ASSERT(loSceneTimestamp < hiSceneTimestamp);
    return ((timestamp - loSceneTimestamp) / qreal(hiSceneTimestamp - loSceneTimestamp)) * qreal(sceneRect().height());
}

// Returns the timestamp for a vertical position.
long LaneScene::vPosToTimestamp(qreal y) const
{
    const long loSceneTimestamp = QDateTime(baseDate_, QTime(0, 0)).toTime_t();
    const long hiSceneTimestamp = QDateTime(baseDate_.addDays(dateSpan_), QTime(0, 0)).toTime_t();
    Q_ASSERT(loSceneTimestamp < hiSceneTimestamp);
    qreal frac = y / qreal(sceneRect().height());
    return loSceneTimestamp + frac * (hiSceneTimestamp - loSceneTimestamp);
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

    // update scene rect height
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), srect.width(), dateSpan_ * secsInDay());

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

void LaneScene::updateBaseItemGeometry()
{
    const qreal lwidth = rolesScene_->laneWidth();
    const qreal lhpad = rolesScene_->laneHorizontalPadding();

    int i = 0;
    foreach (LaneItem *lItem, laneItems()) {
        // update lane item rect
        lItem->setRect(i * lwidth + lhpad, 0, lwidth - lhpad, height());
        i++;
    }

    for (int i = 0; i < dateSpan_; ++i) {
        // update date item
        const long x = sceneRect().x();
        const long date_y = sceneRect().y() + i * secsInDay();
        const long w = sceneRect().width();
        dateItems_.at(i)->setRect(QRectF(x, date_y, w, secsInDay()));

        // update time items
        for (int j = 0; j < 23; ++j) {
            const long time_y = date_y + (j + 1) * secsInHour();
            timeItems_.at(i * 23 + j)->setLine(x, time_y, x + w, time_y);
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

            const qreal by = date_y + bsecs;
            const qreal ey = date_y + esecs;
            roleTimeItems_.at(i * laneItems().size() + j)->setRect(QRect(j * lwidth + lhpad, by, lwidth - lhpad, ey - by + 1));
        }
    }
}

void LaneScene::updateTaskItemGeometry()
{
    const qreal lwidth = rolesScene_->laneWidth();
    const qreal lhpad = rolesScene_->laneHorizontalPadding();

    int i = 0;
    foreach (LaneItem *lItem, laneItems()) {
        updateTaskItemGeometryInLane(lItem, i++, lwidth, lhpad);
    }
}

void LaneScene::updateTaskItemGeometryInLane(LaneItem *lItem, int index, int lwidth, int lhpad)
{
    // update task item rects
    foreach (TaskItem *tItem, taskItems(lItem->roleId())) {
        QSharedPointer<Task> task = TaskManager::instance()->findTask(tItem->taskId());
        const long loTimestamp = task->loDateTime().toTime_t();
        const long hiTimestamp = task->hiDateTime().toTime_t();
        Q_ASSERT(loTimestamp < hiTimestamp);
        const qreal y1 = timestampToVPos(loTimestamp);
        const qreal y2 = timestampToVPos(hiTimestamp);
        tItem->setRect(index * lwidth + 2 * lhpad, y1, lwidth - 4 * lhpad, y2 - y1);
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

    const long currTimestamp = QDateTime::currentDateTime().toTime_t();
    const qreal y = timestampToVPos(currTimestamp);
    currTimeMarker_->setLine(sceneRect().x(), y, sceneRect().width(), y);
}

void LaneScene::updateFromTaskMgr()
{
    const QList<qint64> tmRoleIds = TaskManager::instance()->roleIds();

    // remove lane items for roles that no longer exist in the task manager
    foreach (LaneItem *lItem, laneItems()) {
        if (!tmRoleIds.contains(lItem->roleId()))
            removeItem(lItem);
    }

    // add lane items for roles in the task manager that have no lane items
    const QList<qint64> liRoleIds = laneItemRoleIds();
    foreach (qint64 tmRoleId, tmRoleIds) {
        if (!liRoleIds.contains(tmRoleId))
            addLaneItem(tmRoleId);
    }

    const QList<qint64> tmAllTaskIds = TaskManager::instance()->taskIds();

    // remove task items for tasks that no longer exist in the task manager
    foreach (TaskItem *tItem, taskItems()) {
        if (!tmAllTaskIds.contains(tItem->taskId()))
            removeItem(tItem);
    }

    // ensure that each lane contains exactly the tasks that are assigned to the corresponding role
    foreach (LaneItem *lItem, laneItems()) {
        const qint64 roleId = lItem->roleId();
        const QList<qint64> tmRoleTaskIds = TaskManager::instance()->assignedTasks(roleId);

        // remove task items for tasks that are no longer assigned to this role
        foreach (TaskItem *tItem, taskItems(roleId)) {
            if (!tmRoleTaskIds.contains(tItem->taskId()))
                removeItem(tItem);
        }

        // add task items for tasks in the task manager that are assigned to this role but have no task items
        QList<qint64> itemRoleTaskIds = taskIds(taskItems(roleId));
        foreach (qint64 taskId, tmRoleTaskIds) {
            if (!itemRoleTaskIds.contains(taskId))
                addItem(new TaskItem(taskId));
        }
    }

    updateGeometry();
}

void LaneScene::updateGeometry()
{
    // note: we assume that rolesScene_ is up to date at this point

    // update scene rect width
    {
        const QRectF srect = sceneRect();
        setSceneRect(srect.x(), srect.y(), laneItems().size() * rolesScene_->laneWidth() + rolesScene_->laneHorizontalPadding(), srect.height());
    }

    updateRoleTimeItems();
    updateBaseItemGeometry();
    updateTaskItemGeometry();
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

QList<TaskItem *> LaneScene::taskItems(const QPointF &pos) const
{
    QList<TaskItem *> tItems;
    foreach (QGraphicsItem *item, items(pos)) {
        TaskItem *tItem = dynamic_cast<TaskItem *>(item);
        if (tItem)
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

QList<qint64> LaneScene::taskIds(const QList<TaskItem *> &tItems) const
{
    QList<qint64> tIds;
    foreach (TaskItem *tItem, tItems)
        tIds.append(tItem->taskId());
    return tIds;
}

void LaneScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // for existing tasks, update which task is the current one
    const QList<TaskItem *> hitTaskItems = taskItems(event->scenePos());
    TaskItem *origHoverTaskItem = hoverTaskItem_;
    hoverTaskItem_ = hitTaskItems.isEmpty() ? 0 : hitTaskItems.first();
    if (origHoverTaskItem)
        origHoverTaskItem->highlight(false);
    if (hoverTaskItem_)
        hoverTaskItem_->highlight(true);

    // update hover highlighting etc.
    const int lwidth = rolesScene_->laneWidth();
    const int lhpad = rolesScene_->laneHorizontalPadding();
    const int scenex = event->scenePos().x();
    currLaneIndex_ = (scenex < 0) ? -1 : (scenex / lwidth);
    if ((currLaneIndex_ >= 0) && (currLaneIndex_ < laneItems().size())) {

        // insertion position for adding new task
        insertTop_ = event->scenePos().y();
        insertBottom_ = insertTop_ + 2 * secsInHour(); // ### for now

        hoverTimeMarker_->setLine(lhpad, insertTop_, sceneRect().width() - lhpad, insertTop_);
        hoverRoleMarker_->setRect(currLaneIndex_ * lwidth + lhpad, 0, lwidth - lhpad, height());

        hoverTimeMarker_->setVisible(true);
        hoverRoleMarker_->setVisible(true);
    } else {
        hoverTimeMarker_->setVisible(false);
        hoverRoleMarker_->setVisible(false);
    }
}

void LaneScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (hoverTaskItem_) {
            if (hoverTaskItem_ != currTaskItem_) {
                // make another task item current
                currTaskItem_ = hoverTaskItem_;
                currTaskItem_->setSelected(true);

                // update highlighting
                QSharedPointer<Task> currTask = TaskManager::instance()->findTask(currTaskItem_->taskId());
                Q_ASSERT(currTask);
                const qreal lwidth = rolesScene_->laneWidth();
                const qreal lhpad = rolesScene_->laneHorizontalPadding();
                const qreal lvpad = rolesScene_->laneVerticalPadding();
                QRectF rect;
                rect.setLeft(currLaneIndex_ * lwidth + 2 * lhpad);
                rect.setTop(timestampToVPos(currTask->loDateTime().toTime_t()) - 0 * lvpad);
                rect.setWidth(lwidth - 4 * lhpad);
                rect.setHeight((currTask->hiDateTime().toTime_t() - currTask->loDateTime().toTime_t()) + 1 + 0 * lvpad);
                currTaskMarker_->setRect(rect);
                currTaskMarker_->setVisible(true);
            }
        } else {
            // make no task item current and update highlighting
            currTaskItem_ = 0;
            currTaskMarker_->setVisible(false);
        }
    } else if (event->button() == Qt::RightButton) {
        // open context menu
        QMenu contextMenu;
        contextMenu.addAction(addTaskAction_);
        if (hoverTaskItem_) {
            contextMenu.addAction(editTaskAction_);
            contextMenu.addAction(removeTaskAction_);
        }
        contextMenu.exec(menuPos_ = QCursor::pos());
    }
}

void LaneScene::addTask()
{
    const qint64 roleId = rolesScene_->laneToRoleId(currLaneIndex_);
    const long loTimestamp = vPosToTimestamp(insertTop_);
    const long hiTimestamp = vPosToTimestamp(insertBottom_);

    const qint64 taskId = TaskManager::instance()
            ->addTask(QSharedPointer<Task>(new Task("new task",
                                                    QDateTime::fromTime_t(loTimestamp),
                                                    QDateTime::fromTime_t(hiTimestamp))));
    TaskManager::instance()->assignTaskToRole(taskId, roleId);
    TaskManager::instance()->emitUpdated();
}

void LaneScene::editTask()
{
    Q_ASSERT(hoverTaskItem_);
    qDebug() << "editTask() ..." << hoverTaskItem_;
}

void LaneScene::removeTask()
{
    Q_ASSERT(hoverTaskItem_);
    TaskManager::instance()->removeTask(hoverTaskItem_->taskId());
    hoverTaskItem_ = 0;
    TaskManager::instance()->emitUpdated();
}
