#include "topheaderscene.h"
#include "topheaderview.h"
#include "lanescene.h"
#include "common.h"
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
//#include <QGraphicsLineItem>
#include <QGraphicsView>

TopHeaderScene::TopHeaderScene(LaneScene *laneScene, qreal h, QObject *parent)
    : QGraphicsScene(0, 0, laneScene->width(), h, parent)
    , laneScene_(laneScene)
{
    // add date items
    for (int i = 0; i < laneScene_->dateSpan(); ++i) {
        QGraphicsRectItem *dateRectItem = new QGraphicsRectItem;
        dateRectItem->setBrush(QBrush(QColor((i % 2) ? "#eeeeee" : "#cccccc")));
        dateRectItem->setOpacity(0.4);
        dateRectItem->setZValue(1);
        addItem(dateRectItem);
        dateRectItems_.append(dateRectItem);

        QGraphicsTextItem *dateTextItem = new QGraphicsTextItem(laneScene_->baseDate().addDays(i).toString("yyyy-MM-dd"));
        dateTextItem->setFont(QFont("helvetica", 18));
        dateTextItem->setZValue(2);
        dateTextItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        addItem(dateTextItem);
        dateTextItems_.append(dateTextItem);
    }
    updateDateItems();
}

void TopHeaderScene::updateDateItems()
{
    for (int i = 0; i < laneScene_->dateSpan(); ++i) {
        const qreal x = sceneRect().x() + i * laneScene_->dateWidth();
        const qreal y = sceneRect().y();
        const qreal w = laneScene_->dateWidth();
        const qreal h = sceneRect().height();
        dateRectItems_.at(i)->setRect(QRectF(x, y, w, h));
        dateTextItems_.at(i)->setPos(dateRectItems_.at(i)->rect().x(), dateRectItems_.at(i)->rect().y());
    }
}

void TopHeaderScene::refresh()
{
    // update scene rect
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), laneScene_->width(), views().first()->height() - 10);

    updateDateItems();
}
