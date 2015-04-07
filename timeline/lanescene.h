#ifndef LANESCENE_H
#define LANESCENE_H

#include <QGraphicsScene>

class LeftHeaderScene;
class QGraphicsRectItem;
class LaneBGItem;
class QDate;

class LaneScene : public QGraphicsScene
{
    Q_OBJECT
    friend class LaneView;

public:
    LaneScene(LeftHeaderScene *, const QDate &baseDate, int dateSpan, QObject * = 0);

public slots:
    void refresh();

private:
    LeftHeaderScene *leftHeaderScene_;
    QList<QGraphicsRectItem *> dateItems_;
    QList<LaneBGItem *> laneItems() const;
    QList<qint64> laneItemRoleIds() const;
    void addLaneItem(qint64);
    static qreal dateWidth() { return 1000; }
    void updateDateItems();
    int dateSpan_;
};

#endif // LANESCENE_H
