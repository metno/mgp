#ifndef TOPHEADERSCENE_H
#define TOPHEADERSCENE_H

#include <QGraphicsScene>

class LaneScene;
class QGraphicsRectItem;
class QGraphicsTextItem;

class TopHeaderScene : public QGraphicsScene
{
    Q_OBJECT
    friend class TopHeaderView;

public:
    TopHeaderScene(LaneScene *, qreal, QObject * = 0);

public slots:
    void refresh();

private:
    LaneScene *laneScene_;
    QList<QGraphicsRectItem *> dateRectItems_; // ### rename to dateRectItems_
    QList<QGraphicsTextItem *> dateTextItems_; // ### rename to dateTextItems_
    void updateDateItems();

private slots:
    void updateDateRange();
};

#endif // TOPHEADERSCENE_H
