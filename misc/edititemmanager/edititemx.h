#ifndef EDITITEMX_H
#define EDITITEMX_H

#include <QtGui> // ### TODO: include relevant headers only
#include "edititembase.h"

class EditItemX : public EditItemBase
{
    Q_OBJECT
    friend class SetGeometryCommand;
public:
    EditItemX();
    virtual ~EditItemX();
private:
    virtual bool hit(const QPoint &, bool) const;
    virtual bool hit(const QRect &) const;
    virtual void mousePress(QMouseEvent *, bool *, QList<QUndoCommand *> *, QSet<EditItemBase *> *, bool *);
    virtual void mouseRelease(QMouseEvent *, bool *, QList<QUndoCommand *> *);
    virtual void mouseMove(QMouseEvent *, bool *);
    virtual void mouseHover(QMouseEvent *, bool *);
    virtual void keyPress(QKeyEvent *, bool *, QList<QUndoCommand *> *, QSet<EditItemBase *> *);
    virtual void keyRelease(QKeyEvent *, bool *);
    virtual void draw(DrawModes);
    int hitControlPoint(const QPoint &) const;
    void move(const QPoint &);
    void resize(const QPoint &);
    void updateControlPoints();
    void drawControlPoints();
    void drawHoverHighlighting();
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

    bool moving_;
    bool resizing_;
    QPoint baseMousePos_;
    QPoint baseBottomLeftPos_;
    QPoint baseBottomRightPos_;
    QPoint baseTopLeftPos_;
    QPoint baseTopRightPos_;
    int currCtrlPointIndex_;

    QAction *remove_;
    QAction *split_;
    QAction *merge_;
    QMenu *contextMenu_;

    QColor color_;
};

#endif // EDITITEMX_H
