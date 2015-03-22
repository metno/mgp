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
    bgItem_->setZValue(-1);
    addItem(bgItem_);
}

qreal LaneHeaderScene::laneHeight()
{
    return 100;
}

qreal LaneHeaderScene::lanePadding()
{
    return 5;
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

    // update scene rect
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), views().first()->width() - 10, headerItems().size() * laneHeight() + lanePadding());

    // update header item attributes
    int i = 0;
    const qreal lpadding = lanePadding();
    const qreal lheight = laneHeight();
    foreach (LaneHeaderItem *item, headerItems()) {
        item->setRect(lpadding, i * lheight + lpadding, width() - 2 * lpadding, lheight - lpadding);
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
