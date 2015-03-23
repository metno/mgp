#include "laneheaderscene.h"
#include "laneheaderbgitem.h"
#include "laneheaderview.h"
#include "taskmanager.h"
#include "common.h"
#include <QGraphicsRectItem>
#include <QGraphicsView>

LaneHeaderScene::LaneHeaderScene(qreal x, qreal y, qreal w, qreal h, QObject *parent)
    : QGraphicsScene(x, y, w, h, parent)
{
    // add background item
    bgItem_ = new QGraphicsRectItem(sceneRect());
    bgItem_->setBrush(QBrush(QColor("#cccccc")));
    bgItem_->setZValue(-1);
    addItem(bgItem_);
}

void LaneHeaderScene::refresh()
{
    const QList<qint64> tmRoleIds = TaskManager::instance()->roleIds();

    // remove header items for roles that no longer exist in the task manager
    foreach (LaneHeaderBGItem *hItem, headerItems()) {
        if (!tmRoleIds.contains(hItem->roleId()))
            removeItem(hItem);
    }

    // add header items for unrepresented roles in the task manager
    const QList<qint64> hiRoleIds = headerItemRoleIds();
    foreach (qint64 tmRoleId, tmRoleIds) {
        if (!hiRoleIds.contains(tmRoleId))
            addHeaderItem(tmRoleId);
    }

    // update scene rect
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), views().first()->width() - 10, headerItems().size() * laneHeight() + lanePadding());

    // update header item rects
    const qreal lpadding = lanePadding();
    const qreal lheight = laneHeight();
    int i = 0;
    foreach (LaneHeaderBGItem *item, headerItems()) {
        item->setPos(0, 0);
        item->setRect(lpadding, i * lheight + lpadding, width() - 2 * lpadding, lheight - lpadding);
        i++;
    }

    // update background item
    bgItem_->setRect(sceneRect());
}

QList<LaneHeaderBGItem *> LaneHeaderScene::headerItems() const
{
    QList<LaneHeaderBGItem *> hItems;
    foreach (QGraphicsItem *item, items()) {
        LaneHeaderBGItem *hItem = dynamic_cast<LaneHeaderBGItem *>(item);
        if (hItem)
            hItems.append(hItem);
    }
    return hItems;
}

QList<qint64> LaneHeaderScene::headerItemRoleIds() const
{
    QList<qint64> hiRoleIds;
    foreach (QGraphicsItem *item, items()) {
        LaneHeaderBGItem *hItem = dynamic_cast<LaneHeaderBGItem *>(item);
        if (hItem)
            hiRoleIds.append(hItem->roleId());
    }
    return hiRoleIds;
}

void LaneHeaderScene::addHeaderItem(qint64 roleId)
{
    addItem(new LaneHeaderBGItem(roleId));
}
