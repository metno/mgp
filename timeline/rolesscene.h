#ifndef ROLESSCENE_H
#define ROLESSCENE_H

#include <QGraphicsScene>

class QGraphicsRectItem;
class RolesLaneItem;

class RolesScene : public QGraphicsScene
{
    Q_OBJECT

public:
    RolesScene(qreal, qreal, QObject * = 0);
    static qreal laneWidth() { return 100; } // ### should this be wider?
    static qreal laneHorizontalPadding() { return 5; }
    static qreal laneVerticalPadding() { return 5; }
    qint64 laneToRoleId(int) const;

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
