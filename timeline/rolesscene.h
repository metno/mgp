#ifndef ROLESSCENE_H
#define ROLESSCENE_H

#include <QGraphicsScene>

class QGraphicsRectItem;
class RolesLaneItem;

class RolesScene : public QGraphicsScene
{
    Q_OBJECT

public:
    RolesScene(qreal, qreal, qreal, qreal, QObject * = 0);
    static qreal laneHeight() { return 100; }
    static qreal laneHorizontalPadding() { return 5; }
    static qreal laneVerticalPadding() { return 5; }

public slots:
    void updateFromTaskMgr();
    void updateGeometry();

private:
    QGraphicsRectItem *bgItem_;
    QList<RolesLaneItem *> headerItems() const;
    QList<qint64> headerItemRoleIds() const;
    void addHeaderItem(qint64);
};

#endif // ROLESSCENE_H
