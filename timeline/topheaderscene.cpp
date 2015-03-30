#include "topheaderscene.h"
#include "topheaderview.h"
#include "lanescene.h"
#include "common.h"
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsView>

TopHeaderScene::TopHeaderScene(LaneScene *laneScene, qreal h, QObject *parent)
    : QGraphicsScene(0, 0, laneScene->width(), h, parent)
    , laneScene_(laneScene)
{
    // add background item
    bgItem_ = new QGraphicsRectItem(sceneRect());
    bgItem_->setBrush(QBrush(QColor("#aaaaaa")));
    bgItem_->setZValue(-1);
    addItem(bgItem_);

    // add line item (for testing)
    lineItem_ = new QGraphicsLineItem(QLineF(sceneRect().bottomLeft(), sceneRect().topRight()));
    lineItem_->setPen(QPen(QColor("#ff0000")));
    lineItem_->setZValue(-0.5);
    addItem(lineItem_);
}

void TopHeaderScene::refresh()
{
    // update scene rect
    const QRectF srect = sceneRect();
    setSceneRect(srect.x(), srect.y(), laneScene_->width(), views().first()->height() - 10);

    // update background item
    bgItem_->setRect(sceneRect());

    // update line item
    lineItem_->setLine(QLineF(sceneRect().bottomLeft(), sceneRect().topRight()));
}
