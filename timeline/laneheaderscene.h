#ifndef LANEHEADERSCENE_H
#define LANEHEADERSCENE_H

#include <QGraphicsScene>

class QGraphicsRectItem;
class LaneHeaderBGItem;

class LaneHeaderScene : public QGraphicsScene
{
    Q_OBJECT
public:
    LaneHeaderScene(qreal, qreal, qreal, qreal, QObject * = 0);
    static qreal laneHeight() { return 100; }
    static qreal lanePadding() { return 5; }

public slots:
    void refresh();

private:
    QGraphicsRectItem *bgItem_;
    QList<LaneHeaderBGItem *> headerItems() const;
    QList<qint64> headerItemRoleIds() const;
    void addHeaderItem(qint64);
};

#endif // LANEHEADERSCENE_H
