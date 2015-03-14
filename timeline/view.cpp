#include "view.h"
#include <QtGui>

View::View(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
{
//    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void View::resizeEvent(QResizeEvent *)
{
    //fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
}
