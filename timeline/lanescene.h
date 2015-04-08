#ifndef LANESCENE_H
#define LANESCENE_H

#include <QGraphicsScene>
#include <QDate>

class LeftHeaderScene;
class QGraphicsRectItem;
class LaneBGItem;

class LaneScene : public QGraphicsScene
{
    Q_OBJECT
    friend class LaneView;

public:
    LaneScene(LeftHeaderScene *, const QDate &, int, QObject * = 0);
    QDate baseDate() const;
    int dateSpan() const;
    static qreal dateWidth();
    void setDateRange(const QDate &, int);

public slots:
    void refresh();

private:
    LeftHeaderScene *leftHeaderScene_;
    QList<QGraphicsRectItem *> dateItems_;
    QList<LaneBGItem *> laneItems() const;
    QList<qint64> laneItemRoleIds() const;
    void addLaneItem(qint64);
    void updateDateItems();
    QDate baseDate_;
    int dateSpan_;

signals:
    void dateRangeChanged();
};

#endif // LANESCENE_H
