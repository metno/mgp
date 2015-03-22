#include "lanescene.h"
#include "laneheaderscene.h"
#include "laneitem.h"
#include "taskmanager.h"
#include "common.h"
#include <QGraphicsRectItem>

LaneScene::LaneScene(LaneHeaderScene *laneHeaderScene, qreal w, QObject *parent)
    : QGraphicsScene(0, 0, w, laneHeaderScene->height(), parent)
    , laneHeaderScene_(laneHeaderScene)
{
    // add background
    bgItem_ = new QGraphicsRectItem(sceneRect());
    bgItem_->setBrush(QBrush(QColor("#eeeeee")));
    bgItem_->setZValue(-1);
    addItem(bgItem_);
}

void LaneScene::refresh()
{
    // note: we assume that laneHeaderScene_ is already refreshed at this point

    // adjust lane item count to match role count
    const int diff = TaskManager::instance()->roleIds().size() - laneItems().size();
    if (diff > 0) {
        for (int i = 0; i < diff; ++i)
            addOneLaneItem();
    } else if (diff < 0) {
        for (int i = 0; i < -diff; ++i)
            removeOneLaneItem();
    }

    // update scene rect
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), srect.width(), laneItems().size() * laneHeaderScene_->laneHeight() + laneHeaderScene_->lanePadding());

    // update header item attributes
    int i = 0;
    const qreal lpadding = laneHeaderScene_->lanePadding();
    const qreal lheight = laneHeaderScene_->laneHeight();
    foreach (LaneItem *item, laneItems()) {
        item->setRect(lpadding, i * lheight + lpadding, width() - 2 * lpadding, lheight - lpadding);
        item->setBrush(QBrush(QColor(128 + qrand() % 128, 128 + qrand() % 128, 128 + qrand() % 128)));
        i++;
    }

    // update background item
    bgItem_->setRect(sceneRect());
}

QList<LaneItem *> LaneScene::laneItems() const
{
    QList<LaneItem *> lItems;
    foreach (QGraphicsItem *item, items()) {
        LaneItem *lItem = dynamic_cast<LaneItem *>(item);
        if (lItem)
            lItems.append(lItem);
    }
    return lItems;
}

void LaneScene::addOneLaneItem()
{
    addItem(new LaneItem);
}

void LaneScene::removeOneLaneItem()
{
    removeItem(laneItems().first());
}
