#include "leftheaderscene.h"
#include "leftheaderbgitem.h"
#include "leftheaderview.h"
#include "taskmanager.h"
#include "common.h"
#include <QGraphicsRectItem>
#include <QGraphicsView>

LeftHeaderScene::LeftHeaderScene(qreal x, qreal y, qreal w, qreal h, QObject *parent)
    : QGraphicsScene(x, y, w, h, parent)
{
    // add background item
    bgItem_ = new QGraphicsRectItem(sceneRect());
    bgItem_->setBrush(QBrush(QColor("#cccccc")));
    bgItem_->setZValue(-1);
    addItem(bgItem_);
}

void LeftHeaderScene::updateFromTaskMgr()
{
    const QList<qint64> tmRoleIds = TaskManager::instance()->roleIds();

    // remove header items for roles that no longer exist in the task manager
    foreach (LeftHeaderLaneItem *hItem, headerItems()) {
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

void LeftHeaderScene::updateGeometry()
{
    // update scene rect
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), views().first()->width() - 10, headerItems().size() * laneHeight() + laneVerticalPadding());

    // update header item rects
    const qreal lhpad = laneHorizontalPadding();
    const qreal lvpad = laneVerticalPadding();
    const qreal lheight = laneHeight();
    int i = 0;
    foreach (LeftHeaderLaneItem *item, headerItems()) {
        item->updateRect(QRectF(lhpad, i * lheight + lvpad, width() - 2 * lhpad, lheight - lvpad));
        i++;
    }

    // update background item
    bgItem_->setRect(sceneRect());
}

QList<LeftHeaderLaneItem *> LeftHeaderScene::headerItems() const
{
    QList<LeftHeaderLaneItem *> hItems;
    foreach (QGraphicsItem *item, items()) {
        LeftHeaderLaneItem *hItem = dynamic_cast<LeftHeaderLaneItem *>(item);
        if (hItem)
            hItems.append(hItem);
    }
    return hItems;
}

QList<qint64> LeftHeaderScene::headerItemRoleIds() const
{
    QList<qint64> hiRoleIds;
    foreach (QGraphicsItem *item, items()) {
        LeftHeaderLaneItem *hItem = dynamic_cast<LeftHeaderLaneItem *>(item);
        if (hItem)
            hiRoleIds.append(hItem->roleId());
    }
    return hiRoleIds;
}

void LeftHeaderScene::addHeaderItem(qint64 roleId)
{
    addItem(new LeftHeaderLaneItem(roleId));
}
