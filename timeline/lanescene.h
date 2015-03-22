#ifndef LANESCENE_H
#define LANESCENE_H

#include <QGraphicsScene>

class LaneHeaderScene;

class LaneScene : public QGraphicsScene
{
    Q_OBJECT
public:
    LaneScene(LaneHeaderScene *, qreal, QObject * = 0);

public slots:
    void refresh();

private:
    LaneHeaderScene *laneHeaderScene_;
};

#endif // LANESCENE_H
