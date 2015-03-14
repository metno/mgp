#include "scene.h"
#include <QtGui>

Scene::Scene()
    : size_(QSize(4000, 1000))
    , nLanes_(6)
    , padding_(5)
{
    setSceneRect(QRect(QPoint(0, 0), size_));

    // add background
    QGraphicsRectItem *background = new QGraphicsRectItem(sceneRect());
    background->setBrush(QBrush(QColor("#eeeeee")));
    addItem(background);

    // add lanes
    const int totLaneHeight = sceneRect().height() / nLanes_;
    const int laneHeight = totLaneHeight - padding_;
    const int x = sceneRect().left() + padding_;
    const int laneWidth = sceneRect().width() - 2 * padding_;
    for (int i = 0; i < nLanes_; ++i) {
        const int y = i * totLaneHeight + padding_;
        QGraphicsRectItem *lane = new QGraphicsRectItem(x, y, laneWidth, laneHeight);
        lane->setBrush(QBrush(QColor(128 + qrand() % 128, 128 + qrand() % 128, 128 + qrand() % 128)));
        addItem(lane);
    }
}
