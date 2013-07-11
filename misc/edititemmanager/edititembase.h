#ifndef EDITITEMBASE_H
#define EDITITEMBASE_H

#include <QtGui> // ### TODO: include relevant headers only

class EditItemBase : public QObject
{
    Q_OBJECT
public:
    virtual ~EditItemBase() {}

    enum DrawMode {
        Normal = 0x0,   // the item is neither selected nor hovered
        Selected = 0x1, // the item is selected
        Hovered = 0x2   // the item is hovered
    };
    Q_DECLARE_FLAGS(DrawModes, DrawMode)

    virtual bool hit(const QPoint &, bool) const = 0;
    virtual bool hit(const QRect &) const = 0;

    // Handles a mouse press event.
    //
    // \a repaintNeeded is set to true iff the scene needs to be repainted.
    //
    // Undo-commands representing the effect of this event may be inserted into \a undoCommands.
    //
    // \a items is, if non-null, a set of items that may potentially be operated on (always including this item).
    // Before returning, the function must modify the set to reflect how items were removed or inserted.
    // NOTE: The event handler may create new items, but not delete existing ones (i.e. by (in)directly invoking the delete operator).
    // Deletion must be handled from the outside.
    //
    // \a multiItemOp is, if non-null, set to true iff the event starts an operation that may involve other items
    // (such as a move operaion).
    virtual void mousePress(
        QMouseEvent *event, bool *repaintNeeded, QList<QUndoCommand *> *undoCommands,
        QSet<EditItemBase *> *items = 0, bool *multiItemOp = 0) = 0;

    // Handles a mouse release event (see documentation for mousePress()).
    virtual void mouseRelease(QMouseEvent *, bool *, QList<QUndoCommand *> *) = 0;
    virtual void mouseMove(QMouseEvent *, bool *) = 0;
    virtual void mouseHover(QMouseEvent *, bool *) = 0;

    // Handles a key press event (see documentation for mousePress()).
    virtual void keyPress(QKeyEvent *, bool *, QList<QUndoCommand *> *, QSet<EditItemBase *> * = 0) = 0;
    virtual void keyRelease(QKeyEvent *, bool *) = 0;

    virtual void draw(DrawModes) = 0;

    int id() const;

    void repaint();

protected:
    EditItemBase();

private:
    int id_;
    static int nextId_;
    int nextId();

signals:
    void repaintNeeded();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(EditItemBase::DrawModes)

#endif // EDITITEMBASE_H
