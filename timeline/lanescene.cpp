#include "lanescene.h"
#include "leftheaderscene.h"
#include "lanebgitem.h"
#include "taskmanager.h"
#include "common.h"
#include <QGraphicsRectItem>
#include <QDate>

LaneScene::LaneScene(LeftHeaderScene *leftHeaderScene, const QDate &baseDate, int dateSpan, QObject *parent)
    : QGraphicsScene(0, 0, dateSpan * dateWidth(), leftHeaderScene->height(), parent)
    , leftHeaderScene_(leftHeaderScene)
    , dateSpan_(dateSpan)
{
    // add date items
    for (int i = 0; i < dateSpan_; ++i) {
        QGraphicsRectItem *dateItem = new QGraphicsRectItem;
        dateItem->setBrush(QBrush(QColor((i % 2) ? "#eeeeee" : "#cccccc")));
        dateItem->setOpacity(0.4);
        dateItem->setZValue(2);
        addItem(dateItem);
        dateItems_.append(dateItem);
    }
    updateDateItems();
}

void LaneScene::updateDateItems()
{
    for (int i = 0; i < dateSpan_; ++i) {
        const qreal x = sceneRect().x() + i * dateWidth();
        const qreal y = sceneRect().y();
        const qreal w = dateWidth();
        const qreal h = sceneRect().height();
        dateItems_.at(i)->setRect(QRectF(x, y, w, h));
    }
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

    // update lane item rects
    const qreal lpadding = leftHeaderScene_->lanePadding();
    const qreal lheight = leftHeaderScene_->laneHeight();
    int i = 0;
    foreach (LaneBGItem *item, laneItems()) {
        item->setPos(0, 0);
        item->setRect(lpadding, i * lheight + lpadding, width() - 2 * lpadding, lheight - lpadding);
        i++;
    }

    updateDateItems();
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
