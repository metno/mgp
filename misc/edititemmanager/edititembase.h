#ifndef EDITITEMBASE_H
#define EDITITEMBASE_H

#include <QtGui> // ### TODO: include relevant headers only

// This is the abstract base class for editable items.
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

    // Returns true iff the item is hit at \a pos.
    // The item is considered selected iff \a selected is true (a selected item may typically be hit at
    // control points as well).
    virtual bool hit(const QPoint &pos, bool selected) const = 0;

    // Returns true iff the item is considered to be hit by \a rect.
    // Whether this means that the item's shape is partially or fully inside \a rect is
    // up to the item itself.
    virtual bool hit(const QRect &bbox) const = 0;

    // Handles a mouse press event for an item in its normal state.
    //
    // \a event is the event.
    //
    // \a repaintNeeded is set to true iff the scene needs to be repainted (typically of the item modified it's state
    // in a way that is reflected visually).
    //
    // Undo-commands representing the effect of this event may be inserted into \a undoCommands.
    // NOTE: commands must not be removed from this container (it may contain commands from other items as well).
    //
    // \a items is, if non-null, a set of items that may potentially be operated on by the event (always including this item).
    // Items may be inserted into or removed from this container to reflect how items were inserted or removed as a result of the operation.
    // NOTE: While new items may be created (with the new operator), existing items must never be deleted (using the delete operator)
    // while in this function. This will be done from the outside.
    //
    // \a multiItemOp is, if non-null, set to true iff the event starts an operation that may involve other items (such as a move operaion).
    virtual void mousePress(
        QMouseEvent *event, bool *repaintNeeded, QList<QUndoCommand *> *undoCommands,
        QSet<EditItemBase *> *items = 0, bool *multiItemOp = 0) = 0;

    // Handles a mouse press event for an item in the process of being completed (i.e. during manual placement of a new item).
    //
    // \a complete is set to true iff the item is in a complete state upon returning from the function.
    //
    // \a aborted is set to true iff completing the item should be cancelled. This causes the item to be deleted.
    //
    // See mousePress() for the documentation of other arguments.
    virtual void incompleteMousePress(QMouseEvent *event, bool *repaintNeeded, bool *complete, bool *aborted) = 0;

    // Handles other events for an item in its normal state.
    // See mousePress() for the documentation of the arguments.
    virtual void mouseRelease(QMouseEvent *event, bool *repaintNeeded, QList<QUndoCommand *> *undoCommands) = 0;
    virtual void mouseMove(QMouseEvent *event, bool *repaintNeeded) = 0;
    virtual void mouseHover(QMouseEvent *event, bool *repaintNeeded) = 0;
    virtual void mouseDoubleClick(QMouseEvent *event, bool *repaintNeeded) = 0;
    virtual void keyPress(QKeyEvent *event, bool *repaintNeeded, QList<QUndoCommand *> *undoCommands, QSet<EditItemBase *> *items = 0) = 0;
    virtual void keyRelease(QKeyEvent *event, bool *repaintNeeded) = 0;

    // Handles other events for an item in the process of being completed (i.e. during manual placement of a new item).
    // See incompleteMousePress() for the documentation of the arguments.
    virtual void incompleteMouseRelease(QMouseEvent *event, bool *repaintNeeded, bool *complete, bool *aborted) = 0;
    virtual void incompleteMouseMove(QMouseEvent *event, bool *repaintNeeded) = 0;
    virtual void incompleteMouseHover(QMouseEvent *event, bool *repaintNeeded) = 0;
    virtual void incompleteMouseDoubleClick(QMouseEvent *event, bool *repaintNeeded, bool *complete, bool *aborted) = 0;
    virtual void incompleteKeyPress(QKeyEvent *event, bool *repaintNeeded, bool *complete, bool *aborted) = 0;
    virtual void incompleteKeyRelease(QKeyEvent *event, bool *repaintNeeded) = 0;

    // Draws the item.
    // \a modes indicates whether the item is selected, hovered, both, or neither.
    // \a incomplete is true iff the item is in the process of being completed (i.e. during manual placement of a new item).
    virtual void draw(DrawModes modes, bool incomplete) = 0;

    // Returns the item's globally unique ID.
    int id() const;

    // Emits the repaintNeeded() signal.
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
