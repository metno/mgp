#ifndef MULTILINE_H
#define MULTILINE_H

#include <QtGui> // ### TODO: include relevant headers only
#include "edititembase.h"

namespace EditItem_MultiLine {

class MultiLine : public EditItemBase
{
    Q_OBJECT
    friend class SetGeometryCommand;
public:
    MultiLine();
    virtual ~MultiLine();
private:
    virtual bool hit(const QPoint &, bool) const;
    virtual bool hit(const QRect &) const;
    virtual void mousePress(QMouseEvent *, bool *, QList<QUndoCommand *> *, QSet<EditItemBase *> *, bool *);
    virtual void incompleteMousePress(QMouseEvent *, bool *, bool *, bool *);
    virtual void mouseRelease(QMouseEvent *, bool *, QList<QUndoCommand *> *);
    virtual void incompleteMouseRelease(QMouseEvent *, bool *, bool *, bool *);
    virtual void mouseMove(QMouseEvent *, bool *);
    virtual void incompleteMouseMove(QMouseEvent *, bool *);
    virtual void mouseHover(QMouseEvent *, bool *);
    virtual void incompleteMouseHover(QMouseEvent *, bool *);
    virtual void keyPress(QKeyEvent *, bool *, QList<QUndoCommand *> *, QSet<EditItemBase *> *);
    virtual void incompleteKeyPress(QKeyEvent *, bool *, bool *, bool *);
    virtual void keyRelease(QKeyEvent *, bool *);
    virtual void incompleteKeyRelease(QKeyEvent *, bool *);
    virtual void draw(DrawModes, bool);
    int hitControlPoint(const QPoint &) const;
    void move(const QPoint &);
    void resize(const QPoint &);
    void updateControlPoints();
    void drawControlPoints();
    void drawHoverHighlighting(bool);
    void remove(bool *, QSet<EditItemBase *> *);
    void split(bool *, QList<QUndoCommand *> *, QSet<EditItemBase *> *);
    void merge(bool *, QList<QUndoCommand *> *, QSet<EditItemBase *> *);
    QList<QPoint> geometry() const { return points_; }
    void setGeometry(const QList<QPoint> &);
    QList<QPoint> baseGeometry() const { return basePoints_; }
    QList<QPoint> firstSegment(int) const; // the arg is a control point index
    QList<QPoint> secondSegment(int) const; // ditto
    qreal distance(const QPoint &) const;

    QList<QPoint> points_;
    QList<QRect> controlPoints_;
    QList<QPoint> basePoints_;

    bool moving_;
    bool resizing_;
    QPoint baseMousePos_;
    int pressedCtrlPointIndex_;
    int hoveredCtrlPointIndex_;

    QPoint *placementPos_;

    QAction *remove_;
    QAction *split_;
    QAction *merge_;
    QMenu *contextMenu_;

    QColor color_;
};

} // namespace EditItem_MultiLine

#endif // MULTILINE_H
