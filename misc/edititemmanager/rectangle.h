#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <QtGui> // ### TODO: include relevant headers only
#include "edititembase.h"

class Rectangle : public EditItemBase
{
    Q_OBJECT
    friend class SetGeometryCommand;
public:
    enum PlacementMode { Instant = 0, Resize = 1 };
    Rectangle(PlacementMode = Instant);
    virtual ~Rectangle();
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
    void drawHoverHighlighting(bool isFocusItem);
    void remove(bool *, QSet<EditItemBase *> *);
    void split(bool *, QList<QUndoCommand *> *, QSet<EditItemBase *> *);
    void merge(bool *, QList<QUndoCommand *> *, QSet<EditItemBase *> *);
    QRect geometry() const { return rect_; }
    void setGeometry(const QRect &);
    QRect preMoveGeometry() const { return preMoveRect_; }
    QRect upperHalf() const;
    QRect lowerHalf() const;

    QRect rect_;
    QList<QRect> controlPoints_;
    QRect preMoveRect_;

    bool placementMode_;
    bool moving_;
    bool resizing_;
    QPoint baseMousePos_;
    QPoint baseBottomLeftPos_;
    QPoint baseBottomRightPos_;
    QPoint baseTopLeftPos_;
    QPoint baseTopRightPos_;
    int pressedCtrlPointIndex_;
    int hoveredCtrlPointIndex_;

    QPoint *placementPos1_;
    QPoint *placementPos2_;

    QAction *remove_;
    QAction *split_;
    QAction *merge_;
    QMenu *contextMenu_;

    QColor color_;
};

#endif // RECTANGLE_H
