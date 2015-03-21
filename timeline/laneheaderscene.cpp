#include "laneheaderscene.h"
#include "taskmanager.h"
#include <QGraphicsRectItem>
#include <QGraphicsView>

#include <QDebug>

LaneHeaderScene::LaneHeaderScene(qreal x, qreal y, qreal w, qreal h, QObject *parent)
    : QGraphicsScene(x, y, w, h, parent)
{
    // add background
    bgItem_ = new QGraphicsRectItem(sceneRect());
    bgItem_->setBrush(QBrush(QColor("#cccccc")));
    addItem(bgItem_);
}

void LaneHeaderScene::update()
{
    // update scene rect to same width as view
    Q_ASSERT(views().size() == 1);
    QGraphicsView *view = views().first();
    const QRectF srect = sceneRect();
    qDebug() << "LaneHeaderScene::update(), scene rect:" << srect << ", view->width():" << view->width();
    setSceneRect(srect.x(), srect.y(), view->width(), srect.height());

    // update background
    bgItem_->setRect(sceneRect());
}
