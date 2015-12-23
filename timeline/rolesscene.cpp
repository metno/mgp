#include "rolesscene.h"
#include "roleslaneitem.h"
#include "rolesview.h"
#include "taskmanager.h"
#include "rolepanel.h"
#include "roleeditor.h"
#include "common.h"
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QAction>
#include <QMenu>

RolesScene::RolesScene(qreal w, qreal h, QObject *parent)
    : QGraphicsScene(0, 0, w, h, parent)
    , hoverRolesLaneItem_(0)
{
    // add background item
    bgItem_ = new QGraphicsRectItem(sceneRect());
    bgItem_->setBrush(QBrush(QColor("#cccccc")));
    bgItem_->setZValue(-1);
    addItem(bgItem_);

    editRoleAction_ = new QAction("Edit role", 0);
    connect(editRoleAction_, SIGNAL(triggered()), SLOT(editHoveredRole()));

    removeRoleAction_ = new QAction("Remove role", 0);
    connect(removeRoleAction_, SIGNAL(triggered()), SLOT(removeHoveredRole()));
}

void RolesScene::updateFromTaskMgr()
{
    const QList<qint64> tmRoleIds = TaskManager::instance().roleIds();

    // remove header items for roles that no longer exist in the task manager
    foreach (RolesLaneItem *hItem, headerItems()) {
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

void RolesScene::updateGeometryAndContents()
{
    // update scene rect
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), headerItems().size() * laneWidth() + laneHorizontalPadding(), views().first()->height() - 10);

    // update header item rects and texts
    const qreal lhpad = laneHorizontalPadding();
    const qreal lvpad = laneVerticalPadding();
    const qreal lwidth = laneWidth();
    int i = 0;
    foreach (RolesLaneItem *item, headerItems()) {
        item->updateRect(QRectF(i * lwidth + lhpad, lvpad, lwidth - lhpad, height() - 2 * lvpad));
        item->updateName(TaskManager::instance().findRole(item->roleId())->name());
        i++;
    }

    // update background item
    bgItem_->setRect(sceneRect());
}

void RolesScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // update hovered roles lane item
    foreach (QGraphicsItem *item, items(event->scenePos())) {
        hoverRolesLaneItem_ = dynamic_cast<RolesLaneItem *>(item);
        if (hoverRolesLaneItem_)
            break;
    }

    if (hoverRolesLaneItem_)
        RolePanel::instance().setContents(TaskManager::instance().findRole(hoverRolesLaneItem_->roleId()).data());
    else
        RolePanel::instance().clearContents();
}

void RolesScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        if (!hoverRolesLaneItem_)
            return;

        // open context menu
        QMenu contextMenu;
        contextMenu.addAction(editRoleAction_);
        contextMenu.addAction(removeRoleAction_);
        contextMenu.exec(QCursor::pos());
    }
}

void RolesScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->button() == Qt::LeftButton) && hoverRolesLaneItem_)
        editHoveredRole();
}

QList<RolesLaneItem *> RolesScene::headerItems() const
{
    QList<RolesLaneItem *> hItems;
    foreach (QGraphicsItem *item, items()) {
        RolesLaneItem *hItem = dynamic_cast<RolesLaneItem *>(item);
        if (hItem)
            hItems.append(hItem);
    }
    return hItems;
}

QList<qint64> RolesScene::headerItemRoleIds() const
{
    QList<qint64> hiRoleIds;
    foreach (QGraphicsItem *item, items()) {
        RolesLaneItem *hItem = dynamic_cast<RolesLaneItem *>(item);
        if (hItem)
            hiRoleIds.append(hItem->roleId());
    }
    return hiRoleIds;
}

void RolesScene::addHeaderItem(qint64 roleId)
{
    addItem(new RolesLaneItem(roleId));
}

qint64 RolesScene::laneToRoleId(int laneIndex) const
{
    QList<qint64> roleIds = headerItemRoleIds();
    if (laneIndex >= 0 && laneIndex < roleIds.size())
        return roleIds.at(laneIndex);
    return -1;
}

void RolesScene::editHoveredRole()
{
    Q_ASSERT(hoverRolesLaneItem_);
    const qint64 roleId = hoverRolesLaneItem_->roleId();
    Role *role = TaskManager::instance().findRole(roleId).data();
    const QHash<QString, QString> values = RoleEditor::instance().edit(role);
    if (!values.isEmpty()) {
        TaskManager::instance().updateRole(roleId, values);
        RolePanel::instance().setContents(role);
    }
}

void RolesScene::removeHoveredRole()
{
    Q_ASSERT(hoverRolesLaneItem_);
    TaskManager::instance().removeRole(hoverRolesLaneItem_->roleId());
    hoverRolesLaneItem_ = 0;
    TaskManager::instance().emitUpdated();
    RolePanel::instance().clearContents();
}
