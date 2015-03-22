#include "laneheaderscene.h"
#include "laneheaderitem.h"
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
    addItem(bgItem_);
}

void LaneHeaderScene::refresh()
{
    // adjust header item count to match role count
    const int diff = TaskManager::instance()->roleIds().size() - headerItems().size();
    if (diff > 0) {
        for (int i = 0; i < diff; ++i)
            addOneHeaderItem();
    } else if (diff < 0) {
        for (int i = 0; i < -diff; ++i)
            removeOneHeaderItem();
    }

    const int itemCellHeight = 100;

    // update scene rect
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), views().first()->width() - 10, items().size() * itemCellHeight);

    // update header item attributes
    const qreal pad = 5;
    int i = 0;
    foreach (LaneHeaderItem *item, headerItems()) {
        item->setRect(pad, i * itemCellHeight + pad, width() - 2 * pad, itemCellHeight - pad);
        item->setBrush(QBrush(QColor(128 + qrand() % 128, 128 + qrand() % 128, 128 + qrand() % 128)));
        i++;
    }

    // update background item
    bgItem_->setRect(sceneRect());
}

QList<LaneHeaderItem *> LaneHeaderScene::headerItems() const
{
    QList<LaneHeaderItem *> hItems;
    foreach (QGraphicsItem *item, items()) {
        LaneHeaderItem *hItem = dynamic_cast<LaneHeaderItem *>(item);
        if (hItem)
            hItems.append(hItem);
    }
    return hItems;
}

void LaneHeaderScene::addOneHeaderItem()
{
    addItem(new LaneHeaderItem);
}

void LaneHeaderScene::removeOneHeaderItem()
{
    removeItem(headerItems().first());
}
