#include "lanescene.h"
#include "laneheaderscene.h"
#include "taskmanager.h"
#include "common.h"
#include <QGraphicsRectItem>

LaneScene::LaneScene(LaneHeaderScene *laneHeaderScene, qreal w, QObject *parent)
    : QGraphicsScene(0, 0, w, laneHeaderScene->height(), parent)
    , laneHeaderScene_(laneHeaderScene)
{
    // add background
    QGraphicsRectItem *background = new QGraphicsRectItem(sceneRect());
    background->setBrush(QBrush(QColor("#eeeeee")));
    addItem(background);
}

void LaneScene::refresh()
{
    qDebug() << "LaneScene::refresh() ...";
}
