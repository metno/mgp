#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <QtGui> // ### TODO: include relevant headers only
#include "edititembase.h"

namespace EditItem_Rectangle {

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
    virtual void mouseRelease(QMouseEvent *, bool *, QList<QUndoCommand *> *);
    virtual void mouseMove(QMouseEvent *, bool *);
    virtual void mouseHover(QMouseEvent *, bool *);
    virtual void mouseDoubleClick(QMouseEvent *, bool *);
    virtual void keyPress(QKeyEvent *, bool *, QList<QUndoCommand *> *, QSet<EditItemBase *> *);
    virtual void keyRelease(QKeyEvent *, bool *);

    virtual void incompleteMousePress(QMouseEvent *, bool *, bool *, bool *);
    virtual void incompleteMouseRelease(QMouseEvent *, bool *, bool *, bool *);
    virtual void incompleteMouseMove(QMouseEvent *, bool *);
    virtual void incompleteMouseHover(QMouseEvent *, bool *);
    virtual void incompleteMouseDoubleClick(QMouseEvent *, bool *, bool *, bool *);
    virtual void incompleteKeyPress(QKeyEvent *, bool *, bool *, bool *);
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
    QRect geometry() const { return rect_; }
    void setGeometry(const QRect &);
    QRect baseGeometry() const { return baseRect_; }
    QRect upperHalf() const;
    QRect lowerHalf() const;

    QRect rect_;
    QList<QRect> controlPoints_;
    QRect baseRect_;

    bool placementMode_;
    bool moving_;
    bool resizing_;
    QPoint baseMousePos_;
    int pressedCtrlPointIndex_;
    int hoveredCtrlPointIndex_;

    QPoint *placementPos1_;

    QAction *remove_;
    QAction *split_;
    QAction *merge_;
    QMenu *contextMenu_;

    QColor color_;
};

} // namespace EditItem_Rectangle

#endif // RECTANGLE_H
