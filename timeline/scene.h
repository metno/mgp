#ifndef SCENE_H
#define SCENE_H

#include <QtGui>

class Scene : public QGraphicsScene
{
    Q_OBJECT
public:
    Scene();
private:
    QSize size_;
    int nLanes_;
    int padding_;
};

#endif // SCENE_H
