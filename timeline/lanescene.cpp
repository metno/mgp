#include "lanescene.h"
#include "laneheaderscene.h"
#include "laneitem.h"
#include "taskitem.h"
#include "taskmanager.h"
#include "rolepanel.h"
#include "taskpanel.h"
#include "taskeditor.h"
#include "common.h"
#include "misc.h"
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QAction>
#include <QMenu>
#include <QSharedPointer>
#include <QDateTime>
#include <QSettings>

extern QSharedPointer<QSettings> settings;

LaneScene::LaneScene(LaneHeaderScene *laneHeaderScene, const QDate &baseDate__, int dateSpan__, QObject *parent)
    : QGraphicsScene(0, 0, dateSpan__ * secsInDay(), laneHeaderScene->height(), parent)
    , laneHeaderScene_(laneHeaderScene)
    , taskItemLoZValue_(10)
    , taskItemHiZValue_(12)
    , baseDate_(baseDate__)
    , dateSpan_(dateSpan__)
    , currTimeMarker_(0)
    , hoverTaskItem_(0)
    , currTaskItem_(0)
    , pendingCurrTaskId_(-1)
    , hoverLaneIndex_(-1)
    , insertTop_(-1)
    , insertBottom_(-1)
    , nextNewTaskId_(0)
    , contextMenuActive_(false)
    , taskRemovalActive_(false)
    , adjustedFromSettings_(false)
    , draggingTask_(false)
    , fontSizeBaseFrac_(0.5)
{
    Q_ASSERT(taskItemLoZValue_ < (taskItemHiZValue_ - 1)); // ensure a wide enough range

    setDateRange(baseDate_, dateSpan_);

    addTaskAction_ = new QAction("Add new task", 0);
    connect(addTaskAction_, SIGNAL(triggered()), SLOT(addNewTask()));

    editTaskAction_ = new QAction("Edit task", 0);
    connect(editTaskAction_, SIGNAL(triggered()), SLOT(editCurrentTask()));

    removeTaskAction_ = new QAction("Remove task", 0);
    connect(removeTaskAction_, SIGNAL(triggered()), SLOT(removeCurrentTask()));

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

    connect(laneHeaderScene_, SIGNAL(lanesSwapped(int, int)), SLOT(handleLanesSwapped(int, int)));
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
        for (int j = 0; j < laneItems_.size(); ++j) {
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

    updateGeometryAndContents();

    emit dateRangeChanged();
}

void LaneScene::updateFontSize(qreal frac)
{
    fontSizeBaseFrac_ = frac;
    foreach (TaskItem *tItem, taskItems_)
        tItem->updateFontSize(frac);
    emit fontSizeUpdated(frac);

    // update settings
    if (settings)
        settings->setValue("laneSceneFontSizeBaseFrac", frac);
}

void LaneScene::updateBaseItemGeometry()
{
    const qreal lwidth = laneHeaderScene_->laneWidth();
    const qreal lhpad = laneHeaderScene_->laneHorizontalPadding();

    int i = 0;
    foreach (LaneItem *lItem, laneItems_) {
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
        for (int j = 0; j < laneItems_.size(); ++j) {
            LaneItem *lItem = laneItems_.at(j);

            const QTime btime = TaskManager::instance().findRole(lItem->roleId())->loTime();
            const long bsecs = btime.hour() * 3600 + btime.minute() * 60 + btime.second();

            const QTime etime = TaskManager::instance().findRole(lItem->roleId())->hiTime();
            long esecs = etime.hour() * 3600 + etime.minute() * 60 + etime.second();
            if (esecs <= bsecs)
                esecs += secsInDay();

            const qreal by = date_y + bsecs;
            const qreal ey = date_y + esecs;
            roleTimeItems_.at(i * laneItems_.size() + j)->setRect(QRect(j * lwidth + lhpad, by, lwidth - lhpad, ey - by + 1));
        }
    }
}

void LaneScene::updateTaskItems()
{
    const qreal lwidth = laneHeaderScene_->laneWidth();
    const qreal lhpad = laneHeaderScene_->laneHorizontalPadding();

    int i = 0;
    foreach (LaneItem *lItem, laneItems_) {
        updateTaskItemsInLane(lItem, i++, lwidth, lhpad);
    }
}

void LaneScene::updateTaskItemsInLane(LaneItem *lItem, int index, int lwidth, int lhpad)
{
    foreach (TaskItem *tItem, taskItems(lItem->roleId())) {
        QSharedPointer<Task> task = TaskManager::instance().findTask(tItem->taskId());

        // update geometry
        const long loTimestamp = task->loDateTime().toTime_t();
        const long hiTimestamp = task->hiDateTime().toTime_t();
        Q_ASSERT(loTimestamp < hiTimestamp);
        const qreal y1 = timestampToVPos(loTimestamp);
        const qreal y2 = timestampToVPos(hiTimestamp);
        tItem->updateRect(QRectF(index * lwidth + 2 * lhpad, y1, lwidth - 4 * lhpad, y2 - y1));

        // update other properties
        tItem->updateName(task->name());
        tItem->updateSummary(task->summary());
        tItem->updateDescription(task->description());
        tItem->updateColor(task->color());
        if (tItem == currTaskItem_)
            updateCurrTaskMarkerRect(task.data());
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
    const QList<qint64> tmRoleIds = TaskManager::instance().roleIds();

    // remove lane items for roles that no longer exist in the task manager
    foreach (LaneItem *lItem, laneItems_) {
        bool removed = false;
        if (!tmRoleIds.contains(lItem->roleId())) {
            removeItem(lItem);
            laneItems_.removeOne(lItem);
            removed = true;
        }
        if (removed) {
            hoverLaneIndex_ = -1;
            hoverTaskItem_ = currTaskItem_ = 0;
            draggingTask_ = 0;
            hoverRoleMarker_->setVisible(false);
            currTaskMarker_->setVisible(false);
        }
    }

    // add lane items for roles in the task manager that have no lane items
    const QList<qint64> liRoleIds = laneItemRoleIds();
    foreach (qint64 tmRoleId, tmRoleIds) {
        if (!liRoleIds.contains(tmRoleId)) {
            LaneItem *item = new LaneItem(tmRoleId);
            addItem(item);
            laneItems_.append(item);
        }
    }

    if (!adjustedFromSettings_ && laneHeaderScene_->hasAdjustedFromSettings()) {
        adjustFromSettings();
        adjustedFromSettings_ = true;
    }

    const QList<qint64> tmAllTaskIds = TaskManager::instance().taskIds();

    // remove task items for tasks that ...
    foreach (TaskItem *tItem, taskItems()) {
        if ((!tmAllTaskIds.contains(tItem->taskId())) // ... no longer exist in the task manager, or
                || (TaskManager::instance().findTask(tItem->taskId())->roleId() == -1)) { // is no longer assigned to a role
            removeItem(tItem);
            taskItems_.removeOne(tItem);
        }
    }

    // ensure that each lane contains exactly the tasks that are assigned to the corresponding role
    foreach (LaneItem *lItem, laneItems_) {
        const qint64 roleId = lItem->roleId();
        const QList<qint64> tmRoleTaskIds = TaskManager::instance().assignedTasks(roleId);

        // remove task items for tasks that are no longer assigned to this role
        foreach (TaskItem *tItem, taskItems(roleId)) {
            if (!tmRoleTaskIds.contains(tItem->taskId())) {
                removeItem(tItem);
                taskItems_.removeOne(tItem);
            }
        }

        // add task items for tasks in the task manager that are assigned to this role but have no task items
        QList<qint64> itemRoleTaskIds = taskIds(taskItems(roleId));
        foreach (qint64 taskId, tmRoleTaskIds) {
            if (!itemRoleTaskIds.contains(taskId)) {
                TaskItem *taskItem = new TaskItem(taskId, fontSizeBaseFrac_);
                addItem(taskItem);
                taskItems_.append(taskItem);
                if (taskId == pendingCurrTaskId_) {
                    pendingCurrTaskId_ = -1;
                    setCurrTask(taskItem);
                }
            }
        }
    }

    updateZValues();
    updateGeometryAndContents();
}

void LaneScene::updateGeometryAndContents()
{
    // note: we assume that laneHeaderScene_ is up to date at this point

    // update scene rect width
    {
        const QRectF srect = sceneRect();
        setSceneRect(srect.x(), srect.y(), laneItems_.size() * laneHeaderScene_->laneWidth() + laneHeaderScene_->laneHorizontalPadding(), srect.height());
    }

    updateRoleTimeItems();
    updateBaseItemGeometry();
    updateTaskItems();
    updateCurrTimeMarker(); // ### called here for now; eventually to be called automatically every 10 secs or so
}

QList<qint64> LaneScene::laneItemRoleIds() const
{
    QList<qint64> roleIds;
    foreach (LaneItem *item, laneItems_)
        roleIds.append(item->roleId());
    return roleIds;
}

QList<TaskItem *> LaneScene::taskItems(qint64 roleId) const
{
    QList<TaskItem *> tItems;
    foreach (TaskItem *tItem, taskItems_) {
        if ((roleId < 0) || (tItem->roleId() == roleId))
            tItems.append(tItem);
    }
    return tItems;
}

QList<TaskItem *> LaneScene::taskItems(const QPointF &pos) const
{
    QList<TaskItem *> tItems;
    foreach (TaskItem *tItem, taskItems_) {
        if (tItem->sceneBoundingRect().contains(pos))
            tItems.append(tItem);
    }
    return tItems;
}

QList<TaskItem *> LaneScene::taskItems(const QRectF &rect) const
{
    QList<TaskItem *> tItems;
    foreach (TaskItem *tItem, taskItems_) {
        if (tItem->sceneBoundingRect().intersects(rect))
            tItems.append(tItem);
    }
    return tItems;
}

QList<qint64> LaneScene::taskIds(const QList<TaskItem *> &tItems) const
{
    QList<qint64> tIds;
    foreach (TaskItem *tItem, tItems)
        tIds.append(tItem->taskId());
    return tIds;
}

void LaneScene::handleLastMouseMoveEvent()
{

    QList<TaskItem *> hitTaskItems = taskItems(currScenePos_);
    TaskItem *origHoverTaskItem = hoverTaskItem_;
    if (hitTaskItems.isEmpty()) {
        hoverTaskItem_ = 0;
    } else {
        // use the item with the highest z value, i.e. the one at the top
        qreal hiZVal;
        for (int i = 0; i < hitTaskItems.size(); ++i) {
            TaskItem *item = hitTaskItems.at(i);
            if ((i == 0) || (item->zValue() > hiZVal)) {
                hiZVal = item->zValue();
                hoverTaskItem_ = item;
            }
        }
    }

    if (origHoverTaskItem)
        origHoverTaskItem->highlight(false);

    QGraphicsView *view = views().first();
    if (hoverTaskItem_) {
        hoverTaskItem_->highlight(true);
        Task *task = TaskManager::instance().findTask(hoverTaskItem_->taskId()).data();
        TaskPanel::instance().setContents(task);
        RolePanel::instance().setContents(TaskManager::instance().findRole(task->roleId()).data());

        if (!draggingTask_) {
            const int loPos = view->mapToGlobal(view->mapFromScene(hoverTaskItem_->rect().topLeft())).y();
            const int hiPos = view->mapToGlobal(view->mapFromScene(hoverTaskItem_->rect().bottomLeft())).y();
            const int tolerance = 5;
            if (qAbs(currScreenPos_.y() - loPos) < tolerance) {
                dragMode_ = Lo;
                view->setCursor(Qt::SizeVerCursor);
            } else if (qAbs(currScreenPos_.y() - hiPos) < tolerance) {
                dragMode_ = Hi;
                view->setCursor(Qt::SizeVerCursor);
            } else {
                dragMode_ = Both;
                view->unsetCursor();
            }
        }

    } else {
        if (!draggingTask_)
            view->unsetCursor();

        if (currTaskItem_) {
            Task *task = TaskManager::instance().findTask(currTaskItem_->taskId()).data();
            TaskPanel::instance().setContents(task);
            RolePanel::instance().setContents(TaskManager::instance().findRole(task->roleId()).data());
        } else {
            TaskPanel::instance().clearContents();
            RolePanel::instance().clearContents();
        }
    }

    const int lwidth = laneHeaderScene_->laneWidth();
    const int lhpad = laneHeaderScene_->laneHorizontalPadding();
    const int scenex = currScenePos_.x();
    hoverLaneIndex_ = (scenex < 0) ? -1 : (scenex / lwidth);
    if ((hoverLaneIndex_ >= 0) && (hoverLaneIndex_ < laneItems_.size())) {

        // set insertion position for adding new task
        insertTop_ = currScenePos_.y();
        insertBottom_ = insertTop_ + 2 * secsInHour(); // ### for now

        hoverTimeMarker_->setLine(lhpad, insertTop_, sceneRect().width() - lhpad, insertTop_);
        hoverRoleMarker_->setRect(hoverLaneIndex_ * lwidth + lhpad, 0, lwidth - lhpad, height());

        hoverTimeMarker_->setVisible(true);
        hoverRoleMarker_->setVisible(true);
    } else {
        hoverTimeMarker_->setVisible(false);
        hoverRoleMarker_->setVisible(false);
    }

    // update dragged task
    if (draggingTask_) {
        const long deltaTimestamp = vPosToTimestamp(currScenePos_.y()) - vPosToTimestamp(baseScenePos_.y());
        Q_ASSERT(currTaskItem_);
        QHash<QString, QVariant> values;
        const long newLoTimestamp = origLoTimestamp_ + deltaTimestamp;
        const long newHiTimestamp = origHiTimestamp_ + deltaTimestamp;
        if (dragMode_ == Lo) {
            if (newLoTimestamp < origHiTimestamp_)
                values.insert("loDateTime", QDateTime::fromTime_t(newLoTimestamp));
        } else if (dragMode_ == Hi) {
            if (newHiTimestamp > origLoTimestamp_)
                values.insert("hiDateTime", QDateTime::fromTime_t(newHiTimestamp));
        } else {
            Q_ASSERT(dragMode_ == Both);
            Q_ASSERT(newLoTimestamp < newHiTimestamp);
            values.insert("loDateTime", QDateTime::fromTime_t(newLoTimestamp));
            values.insert("hiDateTime", QDateTime::fromTime_t(newHiTimestamp));
        }

        if (!values.isEmpty())
            TaskManager::instance().updateTask(currTaskItem_->taskId(), values);
    }
}

void LaneScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    currScenePos_ = event->scenePos();
    currScreenPos_ = event->screenPos();

    handleLastMouseMoveEvent();
}

void LaneScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        updateCurrTaskItem(false);
        if (currTaskItem_) {
            draggingTask_ = true;
            baseScenePos_ = currScenePos_;
            Task *task = TaskManager::instance().findTask(currTaskItem_->taskId()).data();
            origLoTimestamp_ = task->loDateTime().toTime_t();
            origHiTimestamp_ = task->hiDateTime().toTime_t();
        }

    } else if (event->button() == Qt::RightButton) {
        updateCurrTaskItem(true);

        // open context menu
        QMenu contextMenu;
        contextMenu.addAction(addTaskAction_);
        if (hoverTaskItem_) {
            contextMenu.addAction(editTaskAction_);
            contextMenu.addAction(removeTaskAction_);
        }
        contextMenuActive_ = true;
        contextMenu.exec(QCursor::pos());
        contextMenuActive_ = false;
    }
}

void LaneScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    draggingTask_ = false;
}

void LaneScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->button() == Qt::LeftButton) && currTaskItem_)
        editCurrentTask();
}

void LaneScene::keyPressEvent(QKeyEvent *event)
{
    if (((event->key() == Qt::Key_Delete) || (event->key() == Qt::Key_Backspace)) && currTaskItem_) {
        removeCurrentTask();
    } else if (event->key() == Qt::Key_Insert) {
        addNewTask();
    } else if (event->key() == Qt::Key_PageUp) {
        cycleIntersectedTaskItems(true);
        handleLastMouseMoveEvent();
        if (!taskItems_.isEmpty())
            setCurrTask(taskItems_.last());
    } else if (event->key() == Qt::Key_PageDown) {
        cycleIntersectedTaskItems(false);
        handleLastMouseMoveEvent();
        if (!taskItems_.isEmpty())
            setCurrTask(taskItems_.last());
    }
}

void LaneScene::focusInEvent(QFocusEvent *)
{
    if (currTaskItem_)
        setCurrTask(currTaskItem_);
}

void LaneScene::focusOutEvent(QFocusEvent *)
{
    if ((!contextMenuActive_) && (!taskRemovalActive_))
        clearCurrTask();
}

void LaneScene::updateCurrTaskMarkerRect(Task *currTask)
{
    const qreal lwidth = laneHeaderScene_->laneWidth();
    const qreal lhpad = laneHeaderScene_->laneHorizontalPadding();
    const qreal lvpad = laneHeaderScene_->laneVerticalPadding();
    QRectF rect;
    const int currLaneIndex = laneHeaderScene_->roleIdToLaneIndex(currTask->roleId());
    Q_ASSERT((currLaneIndex >= 0) && (currLaneIndex < laneItems_.size()));
    rect.setLeft(currLaneIndex * lwidth + 2 * lhpad);
    rect.setTop(timestampToVPos(currTask->loDateTime().toTime_t()) - 0 * lvpad);
    rect.setWidth(lwidth - 4 * lhpad);
    rect.setHeight((currTask->hiDateTime().toTime_t() - currTask->loDateTime().toTime_t()) + 1 + 0 * lvpad);
    currTaskMarker_->setRect(rect);
}

void LaneScene::setCurrTask(TaskItem *item)
{
    Q_ASSERT(item);

    currTaskItem_ = item;
    bringToTop(currTaskItem_);

    // update highlighting etc.
    QSharedPointer<Task> currTask = TaskManager::instance().findTask(currTaskItem_->taskId());
    Q_ASSERT(currTask);

    updateCurrTaskMarkerRect(currTask.data());
    currTaskMarker_->setVisible(true);

    TaskPanel::instance().setContents(currTask.data());

    const qint64 roleId = currTask->roleId();
    QSharedPointer<Role> currRole = TaskManager::instance().findRole(roleId);
    RolePanel::instance().setContents(currRole.data());
}

void LaneScene::clearCurrTask()
{
    // make no task item current and update highlighting
    currTaskItem_ = 0;
    currTaskMarker_->setVisible(false);
    draggingTask_ = 0;
    TaskPanel::instance().clearContents();
    RolePanel::instance().clearContents();
}

void LaneScene::updateCurrTaskItem(bool ignoreMiss)
{
    if (hoverTaskItem_) {
        if (hoverTaskItem_ != currTaskItem_)
            setCurrTask(hoverTaskItem_);
    } else if (!ignoreMiss) {
        clearCurrTask();
    }
}

void LaneScene::adjustFromSettings()
{
    // ensure laneItems_ gets ordered correctly

    QList<LaneItem *> sortedLaneItems;
    const QList<qint64> laneItemRoleIds_ = laneItemRoleIds();
    for (int i = 0; i < laneItems_.size(); ++i) {
        const qint64 roleId = laneHeaderScene_->laneIndexToRoleId(i);
        sortedLaneItems.append(laneItems_.at(laneItemRoleIds_.indexOf(roleId)));
    }

    Q_ASSERT(sortedLaneItems.size() == laneItems_.size());
    laneItems_ = sortedLaneItems;
}

void LaneScene::addNewTask()
{
    const qint64 roleId = laneHeaderScene_->laneIndexToRoleId(hoverLaneIndex_);
    const long loTimestamp = vPosToTimestamp(insertTop_);
    const long hiTimestamp = vPosToTimestamp(insertBottom_);

    const qint64 taskId = TaskManager::instance()
            .addTask(TaskProperties(
                         QString("new task %1").arg(nextNewTaskId_),
                         QString("summary of task %1").arg(nextNewTaskId_),
                         QString("description of task %1<br/>another line").arg(nextNewTaskId_),
                         QDateTime::fromTime_t(loTimestamp),
                         QDateTime::fromTime_t(hiTimestamp)));
    nextNewTaskId_++;
    TaskManager::instance().assignTaskToRole(taskId, roleId);
    pendingCurrTaskId_ = taskId;
    TaskManager::instance().emitUpdated();
}

void LaneScene::editCurrentTask()
{
    Q_ASSERT(currTaskItem_);
    const qint64 taskId = currTaskItem_->taskId();
    Task *task = TaskManager::instance().findTask(taskId).data();
    const QHash<QString, QVariant> values = TaskEditor::instance().edit(task);
    if (!values.isEmpty()) {
        TaskManager::instance().updateTask(taskId, values);
        TaskPanel::instance().setContents(task);
    }
}

void LaneScene::removeCurrentTask()
{
    taskRemovalActive_ = true;

    if (confirm("Really remove task?")) {
        Q_ASSERT(currTaskItem_);
        TaskManager::instance().removeTask(currTaskItem_->taskId());
        currTaskItem_ = 0;
        currTaskMarker_->setVisible(false);
        draggingTask_ = 0;
        TaskManager::instance().emitUpdated();
    }

    taskRemovalActive_ = false;
}

void LaneScene::handleViewScaleUpdate()
{
    foreach (TaskItem *tItem, taskItems()) {
        tItem->updateTextPositions();
        tItem->updateShadowPositions();
    }
}

void LaneScene::handleViewLeft()
{
    if (contextMenuActive_)
        return;
    if (hoverTaskItem_)
        hoverTaskItem_->highlight(false);
    hoverTaskItem_ = 0;
}

void LaneScene::handleLanesSwapped(int pos1, int pos2)
{
    Q_ASSERT((pos1 >= 0) && (pos1 < laneItems_.size()));
    Q_ASSERT((pos2 >= 0) && (pos2 < laneItems_.size()));
    Q_ASSERT(pos1 != pos2);
    laneItems_.swap(pos1, pos2);
    updateGeometryAndContents();
    if (currTaskItem_)
        setCurrTask(currTaskItem_); // update highlighting of current task
}

void LaneScene::updateZValues()
{
    const qreal delta = (taskItems_.size() > 1)
            ? (taskItemHiZValue_ - taskItemLoZValue_) / (taskItems_.size() - 1)
            : 0;
    for (int i = 0; i < taskItems_.size(); ++i)
        taskItems_.at(i)->setZValue(taskItemLoZValue_ + i * delta);
}

void LaneScene::cycleIntersectedTaskItems(bool up)
{
    if (!hoverTaskItem_)
        return;

    // find task items that intersect the hovered item
    QList<TaskItem *> isctTaskItems = taskItems(hoverTaskItem_->sceneBoundingRect());

    if (isctTaskItems.size() < 2)
        return;

    const int first = taskItems_.indexOf(isctTaskItems.first());
    const int last = taskItems_.indexOf(isctTaskItems.last());

    if (up)
        taskItems_.insert(first, taskItems_.takeAt(last));
    else
        taskItems_.insert(last, taskItems_.takeAt(first));

    updateZValues();
}

void LaneScene::bringToTop(TaskItem *taskItem)
{
    taskItems_.append(taskItems_.takeAt(taskItems_.indexOf(taskItem)));
    updateZValues();
}
