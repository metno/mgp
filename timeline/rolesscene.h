#ifndef ROLESSCENE_H
#define ROLESSCENE_H

#include <QGraphicsScene>

class QGraphicsRectItem;
class RolesLaneItem;
class QAction;

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

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);

    QList<RolesLaneItem *> headerItems() const;
    QList<qint64> headerItemRoleIds() const;
    void addHeaderItem(qint64);

    QAction *editRoleAction_;
    QAction *removeRoleAction_;
    RolesLaneItem *hoverRolesLaneItem_; // roles lane item (if any) currently hovered

private slots:
    void editHoveredRole();
    void removeHoveredRole();
};

#endif // ROLESSCENE_H
