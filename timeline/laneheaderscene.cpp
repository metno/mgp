#include "laneheaderscene.h"
#include "laneheaderitem.h"
#include "laneheaderview.h"
#include "taskmanager.h"
#include "rolepanel.h"
#include "roleeditor.h"
#include "common.h"
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QAction>
#include <QMenu>

LaneHeaderScene::LaneHeaderScene(qreal w, qreal h, QObject *parent)
    : QGraphicsScene(0, 0, w, h, parent)
    , hoverLaneHeaderItem_(0)
{
    // add background item
    bgItem_ = new QGraphicsRectItem(sceneRect());
    bgItem_->setBrush(QBrush(QColor("#cccccc")));
    bgItem_->setZValue(-1);
    addItem(bgItem_);

    editLaneHeaderAction_ = new QAction("Edit lane header", 0);
    connect(editLaneHeaderAction_, SIGNAL(triggered()), SLOT(editHoveredLaneHeader()));

    removeLaneHeaderAction_ = new QAction("Remove lane header", 0);
    connect(removeLaneHeaderAction_, SIGNAL(triggered()), SLOT(removeHoveredLaneHeader()));
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
    // update hovered lane header item
    foreach (QGraphicsItem *item, items(event->scenePos())) {
        hoverLaneHeaderItem_ = dynamic_cast<LaneHeaderItem *>(item);
        if (hoverLaneHeaderItem_)
            break;
    }

    if (hoverLaneHeaderItem_)
        RolePanel::instance().setContents(TaskManager::instance().findRole(hoverLaneHeaderItem_->roleId()).data());
    else
        RolePanel::instance().clearContents();
}

void LaneHeaderScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        if (!hoverLaneHeaderItem_)
            return;

        // open context menu
        QMenu contextMenu;
        contextMenu.addAction(editLaneHeaderAction_);
        contextMenu.addAction(removeLaneHeaderAction_);
        contextMenu.exec(QCursor::pos());
    }
}

void LaneHeaderScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->button() == Qt::LeftButton) && hoverLaneHeaderItem_)
        editHoveredLaneHeader();
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

void LaneHeaderScene::editHoveredLaneHeader()
{
    Q_ASSERT(hoverLaneHeaderItem_);
    const qint64 roleId = hoverLaneHeaderItem_->roleId();
    Role *role = TaskManager::instance().findRole(roleId).data();
    const QHash<QString, QVariant> values = RoleEditor::instance().edit(role);
    if (!values.isEmpty()) {
        TaskManager::instance().updateRole(roleId, values);
        RolePanel::instance().setContents(role);
    }
}

void LaneHeaderScene::removeHoveredLaneHeader()
{
    Q_ASSERT(hoverLaneHeaderItem_);
    TaskManager::instance().removeRole(hoverLaneHeaderItem_->roleId());
    hoverLaneHeaderItem_ = 0;
    TaskManager::instance().emitUpdated();
    RolePanel::instance().clearContents();
}
