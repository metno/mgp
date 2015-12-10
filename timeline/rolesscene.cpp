#include "rolesscene.h"
#include "roleslaneitem.h"
#include "rolesview.h"
#include "taskmanager.h"
#include "common.h"
#include <QGraphicsRectItem>
#include <QGraphicsView>

RolesScene::RolesScene(qreal w, qreal h, QObject *parent)
    : QGraphicsScene(0, 0, w, h, parent)
{
    // add background item
    bgItem_ = new QGraphicsRectItem(sceneRect());
    bgItem_->setBrush(QBrush(QColor("#cccccc")));
    bgItem_->setZValue(-1);
    addItem(bgItem_);
}

void RolesScene::updateFromTaskMgr()
{
    const QList<qint64> tmRoleIds = TaskManager::instance()->roleIds();

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

    updateGeometry();
}

void RolesScene::updateGeometry()
{
    // update scene rect
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), headerItems().size() * laneWidth() + laneHorizontalPadding(), views().first()->height() - 10);

    // update header item rects
    const qreal lhpad = laneHorizontalPadding();
    const qreal lvpad = laneVerticalPadding();
    const qreal lwidth = laneWidth();
    int i = 0;
    foreach (RolesLaneItem *item, headerItems()) {
        item->updateRect(QRectF(i * lwidth + lhpad, lvpad, lwidth - lhpad, height() - 2 * lvpad));
        i++;
    }

    // update background item
    bgItem_->setRect(sceneRect());
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
