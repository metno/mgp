#ifndef LANEHEADERSCENE_H
#define LANEHEADERSCENE_H

#include <QGraphicsScene>

class QGraphicsRectItem;
class LaneHeaderItem;

class LaneHeaderScene : public QGraphicsScene
{
    Q_OBJECT
public:
    LaneHeaderScene(qreal, qreal, qreal, qreal, QObject * = 0);
    static qreal laneHeight();
    static qreal lanePadding();

public slots:
    void refresh();

private:
    QGraphicsRectItem *bgItem_;
    QList<LaneHeaderItem *> headerItems() const;
    void addOneHeaderItem();
    void removeOneHeaderItem();
};

#endif // LANEHEADERSCENE_H
