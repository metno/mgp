#include "lanescene.h"
#include "leftheaderscene.h"
#include "lanebgitem.h"
#include "taskmanager.h"
#include "common.h"
#include <QGraphicsRectItem>

LaneScene::LaneScene(LeftHeaderScene *leftHeaderScene, qreal w, QObject *parent)
    : QGraphicsScene(0, 0, w, leftHeaderScene->height(), parent)
    , leftHeaderScene_(leftHeaderScene)
{
    // add background
    bgItem_ = new QGraphicsRectItem(sceneRect());
    bgItem_->setBrush(QBrush(QColor("#eeeeee")));
    bgItem_->setZValue(-1);
    addItem(bgItem_);
}

void LaneScene::refresh()
{
    // note: we assume that leftHeaderScene_ is already refreshed at this point

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

    // update scene rect
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), srect.width(), laneItems().size() * leftHeaderScene_->laneHeight() + leftHeaderScene_->lanePadding());

    // update header item rects
    const qreal lpadding = leftHeaderScene_->lanePadding();
    const qreal lheight = leftHeaderScene_->laneHeight();
    int i = 0;
    foreach (LaneBGItem *item, laneItems()) {
        item->setPos(0, 0);
        item->setRect(lpadding, i * lheight + lpadding, width() - 2 * lpadding, lheight - lpadding);
        i++;
    }

    // update background item
    bgItem_->setRect(sceneRect());
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
