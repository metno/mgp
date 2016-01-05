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
    , currLaneIndex_(0)
    , contextMenuActive_(false)
{
    // add background item
    bgItem_ = new QGraphicsRectItem(sceneRect());
    bgItem_->setBrush(QBrush(QColor("#cccccc")));
    bgItem_->setZValue(-1);
    addItem(bgItem_);

    editAction_ = new QAction("Edit lane", 0);
    connect(editAction_, SIGNAL(triggered()), SLOT(editHoveredLane()));

    removeAction_ = new QAction("Remove lane", 0);
    connect(removeAction_, SIGNAL(triggered()), SLOT(removeHoveredLane()));

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
    foreach (LaneHeaderItem *hItem, headerItems()) {
        if (!tmRoleIds.contains(hItem->roleId()))
            removeItem(hItem);
    }

    // add header items for unrepresented roles in the task manager
    const QList<qint64> hiRoleIds = headerItemRoleIds();
    foreach (qint64 tmRoleId, tmRoleIds) {
        if (!hiRoleIds.contains(tmRoleId))
            addHeaderItem(tmRoleId);
    }

    updateGeometryAndContents();
}

void LaneHeaderScene::updateGeometryAndContents()
{
    // update scene rect
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), headerItems().size() * laneWidth() + laneHorizontalPadding(), views().first()->height() - 10);

    // update header item rects and texts
    const qreal lhpad = laneHorizontalPadding();
    const qreal lvpad = laneVerticalPadding();
    const qreal lwidth = laneWidth();
    int i = 0;
    foreach (LaneHeaderItem *item, headerItems()) {
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
        const int scenex = event->scenePos().x();
        currLaneIndex_ = (scenex < 0) ? -1 : (scenex / laneWidth());
    } else {
        RolePanel::instance().clearContents();
        currLaneIndex_ = -1;
    }
}

void LaneHeaderScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        updateCurrLaneHeaderItem(false);

    } else if (event->button() == Qt::RightButton) {
        updateCurrLaneHeaderItem(true);

        if (!hoverItem_)
            return;

        // open context menu
        QMenu contextMenu;
        contextMenu.addAction(editAction_);
        contextMenu.addAction(removeAction_);
        contextMenuActive_ = true;
        contextMenu.exec(QCursor::pos());
        contextMenuActive_ = false;
    }
}

void LaneHeaderScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->button() == Qt::LeftButton) && hoverItem_)
        editHoveredLane();
}

void LaneHeaderScene::focusOutEvent(QFocusEvent *)
{
    currItem_ = 0;
    currMarker_->setVisible(false);
}

void LaneHeaderScene::setCurrLaneHeader(LaneHeaderItem *laneHeaderItem)
{
    // make another lane header item current
    currItem_ = laneHeaderItem;

    // update highlighting etc.
    const qreal lwidth = laneWidth();
    const qreal lhpad = laneHorizontalPadding();
    const qreal lvpad = laneVerticalPadding();

    QRectF rect;
    Q_ASSERT((currLaneIndex_ >= 0) && (currLaneIndex_ < headerItems().size()));
    rect.setLeft(currLaneIndex_ * lwidth + lhpad);
    rect.setTop(lvpad);
    rect.setWidth(lwidth - lhpad);
    rect.setHeight(height() - 2 * lvpad);
    currMarker_->setRect(rect);
    currMarker_->setVisible(true);
}

void LaneHeaderScene::clearCurrLaneHeader()
{
    // make no lane header item current and update highlighting
    currItem_ = 0;
    currMarker_->setVisible(false);
}

void LaneHeaderScene::updateCurrLaneHeaderItem(bool ignoreMiss)
{
    if (hoverItem_) {
        if (hoverItem_ != currItem_)
            setCurrLaneHeader(hoverItem_);
    } else if (!ignoreMiss) {
        clearCurrLaneHeader();
    }
}

QList<LaneHeaderItem *> LaneHeaderScene::headerItems() const
{
    QList<LaneHeaderItem *> hItems;
    foreach (QGraphicsItem *item, items()) {
        LaneHeaderItem *hItem = dynamic_cast<LaneHeaderItem *>(item);
        if (hItem)
            hItems.append(hItem);
    }

    // before returning, the order of the list needs to be modified
    // according to a 'final order' that may be changed interactively
    // (thus supporting client-side moving of lanes (on the server, the
    // lane-order is irrelevant, since each user should be allowed to
    // define his/her own order!)) ... TBD
    // NOTE: whenever this mapping changes (via mouse/key events in this
    // LaneHeaderScene), a signal must be emitted so that the LaneScene can
    // update itself.

    return hItems;
}

QList<qint64> LaneHeaderScene::headerItemRoleIds() const
{
    QList<qint64> hiRoleIds;
    foreach (QGraphicsItem *item, items()) {
        LaneHeaderItem *hItem = dynamic_cast<LaneHeaderItem *>(item);
        if (hItem)
            hiRoleIds.append(hItem->roleId());
    }
    return hiRoleIds;
}

void LaneHeaderScene::addHeaderItem(qint64 roleId)
{
    addItem(new LaneHeaderItem(roleId));
}

qint64 LaneHeaderScene::laneToRoleId(int laneIndex) const
{
    QList<qint64> roleIds = headerItemRoleIds();
    if (laneIndex >= 0 && laneIndex < roleIds.size())
        return roleIds.at(laneIndex);
    return -1;
}

void LaneHeaderScene::editHoveredLane()
{
    Q_ASSERT(hoverItem_);
    const qint64 roleId = hoverItem_->roleId();
    Role *role = TaskManager::instance().findRole(roleId).data();
    const QHash<QString, QVariant> values = RoleEditor::instance().edit(role);
    if (!values.isEmpty()) {
        TaskManager::instance().updateRole(roleId, values);
        RolePanel::instance().setContents(role);
    }
}

void LaneHeaderScene::removeHoveredLane()
{
    if (!confirm("Really remove lane?"))
        return;
    Q_ASSERT(hoverItem_);
    TaskManager::instance().removeRole(hoverItem_->roleId());
    hoverItem_ = 0;
    TaskManager::instance().emitUpdated();
    RolePanel::instance().clearContents();
}

void LaneHeaderScene::handleViewLeft()
{
    if (contextMenuActive_)
        return;
    if (hoverItem_)
        hoverItem_->highlight(false);
    hoverItem_ = 0;
}
