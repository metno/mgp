#include "lanescene.h"
#include "laneheaderscene.h"
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

void LaneScene::update()
{
    laneHeaderScene_->update();

    remember to eventually remove scene.* and view.* (i.e. files not in timeline.pro)
}
