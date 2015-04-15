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
    updateDateRange();
    connect(laneScene_, SIGNAL(dateRangeChanged()), SLOT(updateDateRange()));
}

void TopHeaderScene::updateItemGeometry()
{
    for (int i = 0; i < laneScene_->dateSpan(); ++i) {
        // update date rect- and text items
        const qreal x = sceneRect().x() + i * laneScene_->dateWidth();
        const qreal y = sceneRect().y();
        const qreal w = laneScene_->dateWidth();
        const qreal h = sceneRect().height();
        dateRectItems_.at(i)->setRect(QRectF(x, y, w, h));
        dateTextItems_.at(i)->setPos(dateRectItems_.at(i)->rect().x(), dateRectItems_.at(i)->rect().y());

        // update time text items
        if (!views().isEmpty()) {
            for (int j = 0; j < 24; ++j) {
                QGraphicsTextItem *item = timeTextItems_.at(i * 24 + j);
                const qreal xt = x + j * (laneScene_->dateWidth() / 24.0) - 0 * views().first()->transform().m11();
                item->setPos(xt, dateRectItems_.at(i)->rect().y() + dateRectItems_.at(i)->rect().height() - item->boundingRect().height());
            }
        }
    }
}

void TopHeaderScene::updateFromTaskMgr()
{
    updateGeometry();
}

void TopHeaderScene::updateGeometry()
{
    // update scene rect
    if (!views().isEmpty()) {
        const QRectF srect = sceneRect();
        setSceneRect(srect.x(), srect.y(), laneScene_->width(), views().first()->height() - 10);
    }
    updateItemGeometry();
}

void TopHeaderScene::updateDateRange()
{
    // clear items for existing range
    foreach (QGraphicsRectItem *dateRectItem, dateRectItems_)
        delete dateRectItem;
    dateRectItems_.clear();
    foreach (QGraphicsTextItem *dateTextItem, dateTextItems_)
        delete dateTextItem;
    dateTextItems_.clear();
    foreach (QGraphicsTextItem *timeTextItem, timeTextItems_)
        delete timeTextItem;
    timeTextItems_.clear();

    // add items for new range
    for (int i = 0; i < laneScene_->dateSpan(); ++i) {
        // add date rect item
        QGraphicsRectItem *dateRectItem = new QGraphicsRectItem;
        dateRectItem->setBrush(QBrush(QColor((i % 2) ? "#eeeeee" : "#cccccc")));
        dateRectItem->setOpacity(0.4);
        dateRectItem->setZValue(1);
        addItem(dateRectItem);
        dateRectItems_.append(dateRectItem);

        // add date text item
        QDate date = laneScene_->baseDate().addDays(i);
        QGraphicsTextItem *dateTextItem = new QGraphicsTextItem(date.toString("yyyy-MM-dd"));
        const bool today = (date == QDate::currentDate());
        dateTextItem->setFont(QFont("helvetica", 16, today ? QFont::Bold : QFont::Normal));
        //dateTextItem->setDefaultTextColor(today ? Qt::blue : Qt::black);
        dateTextItem->setZValue(2);
        dateTextItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        addItem(dateTextItem);
        dateTextItems_.append(dateTextItem);

        // add time text items
        for (int j = 0; j < 24; ++j) {
            QString timeText;
            timeText.sprintf("%02d", j);
            QGraphicsTextItem *timeTextItem = new QGraphicsTextItem(timeText);
            timeTextItem->setFont(QFont("helvetica", 12, today ? QFont::Bold : QFont::Normal));
            timeTextItem->setDefaultTextColor(QColor("#555"));
            timeTextItem->setZValue(2);
            timeTextItem->setFlag(QGraphicsItem::ItemIgnoresTransformations);
            addItem(timeTextItem);
            timeTextItems_.append(timeTextItem);
        }
    }

    updateGeometry();
}
