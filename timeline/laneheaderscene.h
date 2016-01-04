#ifndef LANEHEADERSCENE_H
#define LANEHEADERSCENE_H

#include <QGraphicsScene>

class QGraphicsRectItem;
class LaneHeaderItem;
class QAction;

// ### NOTE: Any reference to the term 'role' in this file (+ the .cpp file)
// is based on the assumption that a lane always represents a role. In a later
// version, a lane shows the tasks that match a user-defined condition/filter.
// The role responsible for the tasks in the lane may be part of the condition
// (and this is expected to be the most used condition in practice), but the
// condition may be more general.
//
// For example, LaneHeaderScene::editHoveredLaneHeader() assumes that a lane
// header always represents a role. Once a lane is generalized to represent
// other things (such as tasks not assigned to any role yet, tasks of a specific
// category regardless of role, etc.), the editor must be generalized
// accordingly ... TBD

class LaneHeaderScene : public QGraphicsScene
{
    Q_OBJECT

public:
    LaneHeaderScene(qreal, qreal, QObject * = 0);
    static qreal laneWidth() { return 100; } // ### should this be wider?
    static qreal laneHorizontalPadding() { return 5; }
    static qreal laneVerticalPadding() { return 5; }
    qint64 laneToRoleId(int) const;

public slots:
    void updateFromTaskMgr();
    void updateGeometryAndContents();

private:
    QGraphicsRectItem *bgItem_;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);

    QList<LaneHeaderItem *> headerItems() const;
    QList<qint64> headerItemRoleIds() const;
    void addHeaderItem(qint64);

    QAction *editLaneHeaderAction_;
    QAction *removeLaneHeaderAction_;
    LaneHeaderItem *hoverLaneHeaderItem_; // lane header item (if any) currently hovered

private slots:
    void editHoveredLaneHeader();
    void removeHoveredLaneHeader();
};

#endif // LANEHEADERSCENE_H
