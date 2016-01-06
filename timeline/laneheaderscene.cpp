#include "laneheaderscene.h"
#include "laneheaderitem.h"
#include "laneheaderview.h"
#include "taskmanager.h"
#include "rolepanel.h"
#include "roleeditor.h"
#include "common.h"
#include "misc.h"
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QFocusEvent>
#include <QAction>
#include <QMenu>

LaneHeaderScene::LaneHeaderScene(qreal w, qreal h, QObject *parent)
    : QGraphicsScene(0, 0, w, h, parent)
    , hoverItem_(0)
    , currItem_(0)
    , contextMenuActive_(false)
{
    // add background item
    bgItem_ = new QGraphicsRectItem(sceneRect());
    bgItem_->setBrush(QBrush(QColor("#cccccc")));
    bgItem_->setZValue(-1);
    addItem(bgItem_);

    editAction_ = new QAction("Edit", 0);
    connect(editAction_, SIGNAL(triggered()), SLOT(editCurrentLane()));

    removeAction_ = new QAction("Remove", 0);
    connect(removeAction_, SIGNAL(triggered()), SLOT(removeCurrentLane()));

    moveLeftAction_ = new QAction("Move left", 0);
    connect(moveLeftAction_, SIGNAL(triggered()), SLOT(moveCurrentLaneLeft()));

    moveRightAction_ = new QAction("Move right", 0);
    connect(moveRightAction_, SIGNAL(triggered()), SLOT(moveCurrentLaneRight()));

    currMarker_ = new QGraphicsRectItem;
    currMarker_->setBrush(QBrush(QColor(255, 0, 0, 16)));
    {
        QPen pen(QBrush(QColor(255, 0, 0)), 2);
        pen.setCosmetic(true);
        currMarker_->setPen(pen);
    }
    currMarker_->setZValue(15);
    currMarker_->setVisible(false);
    addItem(currMarker_);
}

void LaneHeaderScene::updateFromTaskMgr()
{
    const QList<qint64> tmRoleIds = TaskManager::instance().roleIds();

    // remove header items for roles that no longer exist in the task manager
    foreach (LaneHeaderItem *hItem, headerItems_) {
        if (!tmRoleIds.contains(hItem->roleId())) {
            removeItem(hItem);
            headerItems_.removeOne(hItem);
        }
    }

    // add header items for unrepresented roles in the task manager
    const QList<qint64> hiRoleIds = headerItemRoleIds();
    foreach (qint64 tmRoleId, tmRoleIds) {
        if (!hiRoleIds.contains(tmRoleId)) {
            LaneHeaderItem *item = new LaneHeaderItem(tmRoleId);
            addItem(item);
            headerItems_.append(item);
        }
    }

    updateGeometryAndContents();
}

void LaneHeaderScene::updateGeometryAndContents()
{
    // update scene rect
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), headerItems_.size() * laneWidth() + laneHorizontalPadding(), views().first()->height() - 10);

    // update header item rects and texts
    const qreal lhpad = laneHorizontalPadding();
    const qreal lvpad = laneVerticalPadding();
    const qreal lwidth = laneWidth();
    int i = 0;
    foreach (LaneHeaderItem *item, headerItems_) {
        item->updateRect(QRectF(i * lwidth + lhpad, lvpad, lwidth - lhpad, height() - 2 * lvpad));
        item->updateProperties();
        i++;
    }

    // update background item
    bgItem_->setRect(sceneRect());
}

void LaneHeaderScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // reset any existing highlighting
    if (hoverItem_)
        hoverItem_->highlight(false);

    // decide which lane header is hovered, if any
    foreach (QGraphicsItem *item, items(event->scenePos())) {
        hoverItem_ = dynamic_cast<LaneHeaderItem *>(item);
        if (hoverItem_)
            break;
    }

    // update
    if (hoverItem_) {
        hoverItem_->highlight(true);
        RolePanel::instance().setContents(TaskManager::instance().findRole(hoverItem_->roleId()).data());
    } else {
        RolePanel::instance().clearContents();
    }
}

void LaneHeaderScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        updateCurrLaneHeaderItem(false);

    } else if (event->button() == Qt::RightButton) {
        updateCurrLaneHeaderItem(true);

        if (!currItem_)
            return;

        // open context menu
        QMenu contextMenu;
        contextMenu.addAction(editAction_);
        contextMenu.addAction(removeAction_);

        contextMenu.addAction(moveLeftAction_);
        moveLeftAction_->setEnabled(headerItems_.indexOf(currItem_) > 0);
        contextMenu.addAction(moveRightAction_);
        moveRightAction_->setEnabled(headerItems_.indexOf(currItem_) < (headerItems_.size() - 1));

        contextMenuActive_ = true;
        contextMenu.exec(QCursor::pos());
        contextMenuActive_ = false;
    }
}

void LaneHeaderScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->button() == Qt::LeftButton) && hoverItem_)
        editCurrentLane();
}

void LaneHeaderScene::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left)
        moveCurrentLaneLeft();
    else if (event->key() == Qt::Key_Right)
        moveCurrentLaneRight();
    else if ((event->key() == Qt::Key_Delete) || (event->key() == Qt::Key_Backspace))
        removeCurrentLane();
}

void LaneHeaderScene::focusInEvent(QFocusEvent *)
{
    if (currItem_)
        currMarker_->setVisible(true);
}

void LaneHeaderScene::focusOutEvent(QFocusEvent *)
{
    if (!contextMenuActive_)
        currMarker_->setVisible(false);
}

void LaneHeaderScene::setCurrItem(LaneHeaderItem *item)
{
    currItem_ = item;

    // update highlighting etc.
    const qreal lwidth = laneWidth();
    const qreal lhpad = laneHorizontalPadding();
    const qreal lvpad = laneVerticalPadding();

    QRectF rect;
    const int currLaneIndex = headerItems_.indexOf(currItem_);
    Q_ASSERT((currLaneIndex >= 0) && (currLaneIndex < headerItems_.size()));
    rect.setLeft(currLaneIndex * lwidth + lhpad);
    rect.setTop(lvpad);
    rect.setWidth(lwidth - lhpad);
    rect.setHeight(height() - 2 * lvpad);
    currMarker_->setRect(rect);
    currMarker_->setVisible(true);
}

void LaneHeaderScene::clearCurrItem()
{
    // make no lane header item current and update highlighting
    currItem_ = 0;
    currMarker_->setVisible(false);
}

void LaneHeaderScene::updateCurrLaneHeaderItem(bool ignoreMiss)
{
    if (hoverItem_) {
        if (hoverItem_ != currItem_)
            setCurrItem(hoverItem_);
    } else if (!ignoreMiss) {
        clearCurrItem();
    }
}

QList<qint64> LaneHeaderScene::headerItemRoleIds() const
{
    QList<qint64> hiRoleIds;
    foreach (LaneHeaderItem *item, headerItems_)
        hiRoleIds.append(item->roleId());
    return hiRoleIds;
}

qint64 LaneHeaderScene::laneIndexToRoleId(int i) const
{
    QList<qint64> roleIds = headerItemRoleIds();
    if (i >= 0 && i < roleIds.size())
        return roleIds.at(i);
    return -1;
}

int LaneHeaderScene::roleIdToLaneIndex(qint64 id) const
{
    return headerItemRoleIds().indexOf(id);
}

void LaneHeaderScene::editCurrentLane()
{
    Q_ASSERT(currItem_);
    const qint64 roleId = currItem_->roleId();
    Role *role = TaskManager::instance().findRole(roleId).data();
    const QHash<QString, QVariant> values = RoleEditor::instance().edit(role);
    if (!values.isEmpty()) {
        TaskManager::instance().updateRole(roleId, values);
        RolePanel::instance().setContents(role);
    }
}

void LaneHeaderScene::clearHoverItem()
{
    if (hoverItem_)
        hoverItem_->highlight(false);
    hoverItem_ = 0;
}

void LaneHeaderScene::removeCurrentLane()
{
    if (!confirm("Really remove lane?"))
        return;
    Q_ASSERT(currItem_);
    TaskManager::instance().removeRole(currItem_->roleId());
    clearCurrItem();
    clearHoverItem();
    TaskManager::instance().emitUpdated();
    RolePanel::instance().clearContents();
}

void LaneHeaderScene::moveCurrentLaneLeft()
{
    Q_ASSERT(headerItems_.contains(currItem_));
    const int pos = headerItems_.indexOf(currItem_);
    if (pos > 0) {
        headerItems_.swap(pos, pos - 1);
        updateGeometryAndContents();
        clearHoverItem();
        setCurrItem(currItem_); // update highlighting of current item
        emit lanesSwapped(pos, pos - 1);
    }
}

void LaneHeaderScene::moveCurrentLaneRight()
{
    Q_ASSERT(headerItems_.contains(currItem_));
    const int pos = headerItems_.indexOf(currItem_);
    if (pos < (headerItems_.size() - 1)) {
        headerItems_.swap(pos, pos + 1);
        updateGeometryAndContents();
        clearHoverItem();
        setCurrItem(currItem_); // update highlighting of current item
        emit lanesSwapped(pos, pos + 1);
    }
}

void LaneHeaderScene::handleViewLeft()
{
    if (contextMenuActive_)
        return;
    clearHoverItem();
}
