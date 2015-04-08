#ifndef LEFTHEADERSCENE_H
#define LEFTHEADERSCENE_H

#include <QGraphicsScene>

class QGraphicsRectItem;
class LeftHeaderBGItem;

class LeftHeaderScene : public QGraphicsScene
{
    Q_OBJECT

public:
    LeftHeaderScene(qreal, qreal, qreal, qreal, QObject * = 0);
    static qreal laneHeight() { return 100; }
    static qreal lanePadding() { return 5; }

public slots:
    void updateFromTaskMgr();
    void updateGeometry();

private:
    QGraphicsRectItem *bgItem_;
    QList<LeftHeaderBGItem *> headerItems() const;
    QList<qint64> headerItemRoleIds() const;
    void addHeaderItem(qint64);
};

#endif // LEFTHEADERSCENE_H
