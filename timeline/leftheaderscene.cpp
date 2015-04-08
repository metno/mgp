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
    foreach (LeftHeaderBGItem *hItem, headerItems()) {
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
    setSceneRect(srect.x(), srect.y(), views().first()->width() - 10, headerItems().size() * laneHeight() + lanePadding());

    // update header item rects
    const qreal lpadding = lanePadding();
    const qreal lheight = laneHeight();
    int i = 0;
    foreach (LeftHeaderBGItem *item, headerItems()) {
        item->setPos(0, 0);
        item->setRect(lpadding, i * lheight + lpadding, width() - 2 * lpadding, lheight - lpadding);
        i++;
    }

    // update background item
    bgItem_->setRect(sceneRect());
}

QList<LeftHeaderBGItem *> LeftHeaderScene::headerItems() const
{
    QList<LeftHeaderBGItem *> hItems;
    foreach (QGraphicsItem *item, items()) {
        LeftHeaderBGItem *hItem = dynamic_cast<LeftHeaderBGItem *>(item);
        if (hItem)
            hItems.append(hItem);
    }
    return hItems;
}

QList<qint64> LeftHeaderScene::headerItemRoleIds() const
{
    QList<qint64> hiRoleIds;
    foreach (QGraphicsItem *item, items()) {
        LeftHeaderBGItem *hItem = dynamic_cast<LeftHeaderBGItem *>(item);
        if (hItem)
            hiRoleIds.append(hItem->roleId());
    }
    return hiRoleIds;
}

void LeftHeaderScene::addHeaderItem(qint64 roleId)
{
    addItem(new LeftHeaderBGItem(roleId));
}
