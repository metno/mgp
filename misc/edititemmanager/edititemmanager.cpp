#include "edititemmanager.h"
#include "edititembase.h"

#include "rectangle.h" // ### needed only as long as this file refers to specific EditItemBase subclasses (like Rectangle)

static QString undoCommandText(int nadded, int nremoved, int nmodified)
{
    QString s;
    if (nadded > 0)
        s += QString("add %1 item%2").arg(nadded).arg((nadded != 1) ? "s" : "");
    if (nremoved > 0)
        s += QString("%1remove %2 item%3").arg(s.isEmpty() ? "" : ", ").arg(nremoved).arg((nremoved != 1) ? "s" : "");
    if (nmodified > 0)
        s += QString("%1modify %2 item%3").arg(s.isEmpty() ? "" : ", ").arg(nmodified).arg((nmodified != 1) ? "s" : "");
    return s;
}

class AddOrRemoveItemsCommand : public QUndoCommand
{
public:
    AddOrRemoveItemsCommand(EditItemManager *, const QSet<EditItemBase *> &, const QSet<EditItemBase *> &);
private:
    EditItemManager *eim_;
    QSet<EditItemBase *> addedItems_;
    QSet<EditItemBase *> removedItems_;
    virtual void undo();
    virtual void redo();
};

AddOrRemoveItemsCommand::AddOrRemoveItemsCommand(
    EditItemManager *eim, const QSet<EditItemBase *> &addedItems, const QSet<EditItemBase *> &removedItems)
    : QUndoCommand(undoCommandText(addedItems.size(), removedItems.size(), 0))
    , eim_(eim)
    , addedItems_(addedItems)
    , removedItems_(removedItems)
{}

void AddOrRemoveItemsCommand::undo()
{
    eim_->addItems(removedItems_);
    eim_->removeItems(addedItems_);
    eim_->repaint();
}

void AddOrRemoveItemsCommand::redo()
{
    eim_->addItems(addedItems_);
    eim_->removeItems(removedItems_);
    eim_->repaint();
}

EditItemManager::EditItemManager()
    : hoverItem_(0)
    , incompleteItem_(0)
    , repaintNeeded_(false)
    , skipRepaint_(false)
{
    connect(&undoStack_, SIGNAL(canUndoChanged(bool)), this, SIGNAL(canUndoChanged(bool)));
    connect(&undoStack_, SIGNAL(canRedoChanged(bool)), this, SIGNAL(canRedoChanged(bool)));
}

// Adds an item to the scene. \a incomplete indicates whether the item is in the process of being manually placed.
void EditItemManager::addItem(EditItemBase *item, bool incomplete)
{
    if (incomplete) {
        // set this item as the incomplete item
        if (incompleteItem_) {
            // issue warning?
        }
        incompleteItem_ = item;
        emit incompleteEditing(true);
    } else {
        // create undo command
        QSet<EditItemBase *> addedItems;
        addedItems.insert(item);
        QSet<EditItemBase *> removedItems;
        AddOrRemoveItemsCommand *arCmd = new AddOrRemoveItemsCommand(this, addedItems, removedItems);
        undoStack_.push(arCmd);
    }

    repaint();
}

void EditItemManager::addItem_(EditItemBase *item)
{
    items_.insert(item);
    connect(item, SIGNAL(repaintNeeded()), this, SLOT(repaint()));
    if (false) selItems_.insert(item); // for now, don't pre-select new items
}

void EditItemManager::addItems(const QSet<EditItemBase *> &items)
{
    foreach (EditItemBase *item, items)
        addItem_(item);
}

void EditItemManager::removeItem(EditItemBase *item)
{
    items_.remove(item);
    disconnect(item, SIGNAL(repaintNeeded()), this, SLOT(repaint()));
    selItems_.remove(item);
}

void EditItemManager::removeItems(const QSet<EditItemBase *> &items)
{
    foreach (EditItemBase *item, items)
        removeItem(item);
}

QUndoStack * EditItemManager::undoStack()
{
    return &undoStack_;
}

void EditItemManager::pasteFromClipboard()
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if (mimeData->formats().contains("test/rectItems")) {
        QByteArray data = mimeData->data("test/rectItems");
        QDataStream dstream(data);
        QVariantMap msg;
        dstream >> msg;
//            qDebug() << "msg:" << msg;
        const QVariantList rectItems = msg.value("rectItems").toList();
        foreach (QVariant rectItem, rectItems) {
            QVariantMap itemProps = rectItem.toMap();
            const QRect rect = itemProps.value("rect").toRect();
            const QColor color = itemProps.value("color").value<QColor>();
//                qDebug() << "rect:" << rect << ", color:" << color;
            addItem(new EditItem_Rectangle::Rectangle(rect, color));
            repaint();
        }
    } else {
        qDebug() << "MIME type \"test/rectItems\" not supported; supported types:" << mimeData->formats();
    }
}

void EditItemManager::mousePress(QMouseEvent *event)
{
    if (incompleteItem_) {
        incompleteMousePress(event);
        return;
    }

    const QList<EditItemBase *> hitItems = findHitItems(event->pos());
    EditItemBase *hitItem = // consider only this item to be hit
        hitItems.empty()
        ? 0
        : hitItems.first(); // for now; eventually use the one with higher z-value etc. ... 2 B DONE
    const bool hitSelItem = selItems_.contains(hitItem); // whether an already selected item was hit
    const bool shiftPressed = event->modifiers() & Qt::ShiftModifier;

    repaintNeeded_ = false;

    QSet<EditItemBase *> origSelItems(selItems_);

    // update selection and hit status
    if (!(hitSelItem || (hitItem && shiftPressed))) {
        selItems_.clear();
    } else if (shiftPressed && hitSelItem && (selItems_.size() > 1)) {
        selItems_.remove(hitItem);
        hitItem = 0;
    }

    QSet<EditItemBase *> addedItems;
    QSet<EditItemBase *> removedItems;
    QList<QUndoCommand *> undoCommands;

    if (hitItem) { // an item is still considered hit
        selItems_.insert(hitItem); // ensure the hit item is selected (it might already be)

        // send mouse press to the hit item
        bool multiItemOp = false;
        QSet<EditItemBase *> eventItems(selItems_); // operate on current selection

        bool rpn = false;
        hitItem->mousePress(event, &rpn, &undoCommands, &eventItems, &multiItemOp);
        if (rpn) repaintNeeded_ = true;
        addedItems = eventItems - selItems_;
        removedItems = selItems_ - eventItems;

        if (items_.contains(hitItem)) {
            // the hit item is still there
            if (multiItemOp) {
                // send the mouse press to other selected items
                // (note that these are not allowed to modify item sets, nor does it make sense for them to flag
                // the event as the beginning of a potential multi-item operation)
                foreach (EditItemBase *item, selItems_)
                    if (item != hitItem) {
                        rpn = false;
                        item->mousePress(event, &rpn, &undoCommands);
                        if (rpn)
                            repaintNeeded_ = true;
                    }
            }
        } else {
            // the hit item removed itself as a result of the mouse press and it makes no sense
            // to send the mouse press to other items
        }
    }

    const bool addedOrRemovedItems = (!addedItems.empty()) || (!removedItems.empty());
    const bool modifiedItems = !undoCommands.empty();
    if (addedOrRemovedItems || modifiedItems)  {
        // combine the aggregated effect of the operation into one undo command
        undoStack_.beginMacro(undoCommandText(addedItems.size(), removedItems.size(), undoCommands.size()));
        skipRepaint_ = true; // temporarily prevent redo() calls from repainting
        if (addedOrRemovedItems) {
            // push sub-command representing aggregated adding/removal of items
            AddOrRemoveItemsCommand *arCmd = new AddOrRemoveItemsCommand(this, addedItems, removedItems);
            undoStack_.push(arCmd);
        }
        // push sub-commands representing individual item modifications
        foreach (QUndoCommand *undoCmd, undoCommands)
            undoStack_.push(undoCmd);
        undoStack_.endMacro();
        skipRepaint_ = false;
        repaintNeeded_ = true;
    }

    // repaint if necessary
    if (repaintNeeded_ || (selItems_ != origSelItems))
        repaint();
}

// Handles a mouse press event for an item in the process of being completed.
void EditItemManager::incompleteMousePress(QMouseEvent *event)
{
    Q_ASSERT(incompleteItem_);
    bool rpn = false;
    bool complete = false;
    bool aborted = false;
    incompleteItem_->incompleteMousePress(event, &rpn, &complete, &aborted);
    if (complete) {
        addItem(incompleteItem_); // causes repaint
        incompleteItem_ = 0;
        emit incompleteEditing(false);
    } else {
        if (aborted) {
            delete incompleteItem_; // or leave it to someone else?
            incompleteItem_ = 0;
            emit incompleteEditing(false);
        }
        if (rpn)
            repaint();
    }
}

void EditItemManager::mouseRelease(QMouseEvent *event)
{
    if (incompleteItem_) {
        incompleteMouseRelease(event);
        return;
    }

    repaintNeeded_ = false;

    QList<QUndoCommand *> undoCommands;

    // send to selected items
    foreach (EditItemBase *item, selItems_)
        item->mouseRelease(event, &repaintNeeded_, &undoCommands);

    const bool modifiedItems = !undoCommands.empty();
    if (modifiedItems) {
        // combine the aggregated effect of the operation into one undo command
        undoStack_.beginMacro(undoCommandText(0, 0, undoCommands.size()));
        skipRepaint_ = true; // temporarily prevent redo() calls from repainting
        // push sub-commands representing individual item modifications
        foreach (QUndoCommand *undoCmd, undoCommands)
            undoStack_.push(undoCmd);
        undoStack_.endMacro();
        skipRepaint_ = false;
        repaintNeeded_ = true;
    }

    if (repaintNeeded_)
        repaint();
}

// Handles a mouse release event for an item in the process of being completed.
void EditItemManager::incompleteMouseRelease(QMouseEvent *event)
{
    Q_ASSERT(incompleteItem_);
    bool rpn = false;
    bool complete = false;
    bool aborted = false;
    incompleteItem_->incompleteMouseRelease(event, &rpn, &complete, &aborted);
    if (complete) {
        addItem(incompleteItem_); // causes repaint
        incompleteItem_ = 0;
        emit incompleteEditing(false);
    } else {
        if (aborted) {
            delete incompleteItem_; // or leave it to someone else?
            incompleteItem_ = 0;
            emit incompleteEditing(false);
        }
        if (rpn)
            repaint();
    }
}

void EditItemManager::mouseMove(QMouseEvent *event)
{
    if (incompleteItem_) {
        incompleteMouseMove(event);
        return;
    }

    // Check if the event is part of a multi-select operation using a rubberband-rectangle.
    // In that case, the event should only be used to update the selection (and tell items to redraw themselves as
    // approproate), and NOT be passed on through the mouseMove() functions of the selected items ... 2 B DONE!

    repaintNeeded_ = false;

    const EditItemBase *origHoverItem = hoverItem_;
    hoverItem_ = 0;
    const bool hover = !event->buttons();
    if (hover) {
        const QList<EditItemBase *> hitItems = findHitItems(event->pos());
        if (!hitItems.empty()) {
            // consider only the topmost item that was hit ... 2 B DONE
            // for now, consider only the first that was found
            hoverItem_ = hitItems.first();
            
            // send mouse hover event to the hover item
            bool rpn = false;
            hoverItem_->mouseHover(event, &rpn);
            if (rpn) repaintNeeded_ = true;
        } else {
            // ignore hovering outside any item
        }
    } else {
        // send move event to all selected items
        foreach (EditItemBase *item, selItems_) {
            bool rpn = false;
            item->mouseMove(event, &rpn);
            if (rpn) repaintNeeded_ = true;
        }
    }

    if (repaintNeeded_ || (hoverItem_ != origHoverItem))
        repaint();
}

// Handles a mouse move event for an item in the process of being completed.
void EditItemManager::incompleteMouseMove(QMouseEvent *event)
{
    Q_ASSERT(incompleteItem_);

    const EditItemBase *origHoverItem = hoverItem_;
    hoverItem_ = 0;
    const bool hover = !event->buttons();
    if (hover) {
        bool rpn = false;
        incompleteItem_->incompleteMouseHover(event, &rpn);
        if (rpn) repaintNeeded_ = true;
        if (incompleteItem_->hit(event->pos(), false))
            hoverItem_ = incompleteItem_;
    } else {
        bool rpn = false;
        incompleteItem_->incompleteMouseMove(event, &rpn);
        if (rpn) repaintNeeded_ = true;
    }

    if (repaintNeeded_ || (hoverItem_ != origHoverItem))
        repaint();
}

void EditItemManager::mouseDoubleClick(QMouseEvent *event)
{
    if (incompleteItem_) {
        incompleteMouseDoubleClick(event);
        return;
    }

    // do nothing for now
}

void EditItemManager::incompleteMouseDoubleClick(QMouseEvent *event)
{
    Q_ASSERT(incompleteItem_);
    bool rpn = false;
    bool complete = false;
    bool aborted = false;
    incompleteItem_->incompleteMouseDoubleClick(event, &rpn, &complete, &aborted);
    if (complete) {
        addItem(incompleteItem_); // causes repaint
        incompleteItem_ = 0;
        emit incompleteEditing(false);
    } else {
        if (aborted) {
            delete incompleteItem_; // or leave it to someone else?
            incompleteItem_ = 0;
            emit incompleteEditing(false);
        }
        if (rpn)
            repaint();
    }
}

static EditItemBase *idToItem(const QSet<EditItemBase *> &items, int id)
{
    foreach (EditItemBase *item, items)
        if (id == item->id())
            return item;
    return 0;
}

void EditItemManager::keyPress(QKeyEvent *event)
{
    if (incompleteItem_) {
        incompleteKeyPress(event);
        return;
    }

    if (selItems_.isEmpty()) {
        if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_V))
            pasteFromClipboard();
        return;
    }

    repaintNeeded_ = false;

    QSet<int> origSelIds; // IDs of the originally selected items
    foreach (EditItemBase *item, selItems_)
        origSelIds.insert(item->id());

    QSet<EditItemBase *> addedItems;
    QSet<EditItemBase *> removedItems;
    QList<QUndoCommand *> undoCommands;

    // process each of the originally selected items
    foreach (int origSelId, origSelIds) {

        // at this point, the item may or may not exist (it may have been removed in an earlier iteration)

        EditItemBase *origSelItem = idToItem(items_, origSelId);
        if (origSelItem) {
            // it still exists, so pass the event
            QSet<EditItemBase *> eventItems(selItems_); // operate on current selection
            bool rpn = false;
            origSelItem->keyPress(event, &rpn, &undoCommands, &eventItems);
            if (rpn) repaintNeeded_ = true;
            addedItems.unite(eventItems - selItems_);
            removedItems.unite(selItems_ - eventItems);
            selItems_.subtract(removedItems);
        }
    }

    const bool addedOrRemovedItems = (!addedItems.empty()) || (!removedItems.empty());
    const bool modifiedItems = !undoCommands.empty();
    if (addedOrRemovedItems || modifiedItems)  {
        // combine the aggregated effect of the operation into one undo command
        undoStack_.beginMacro(undoCommandText(addedItems.size(), removedItems.size(), undoCommands.size()));
        skipRepaint_ = true; // temporarily prevent redo() calls from repainting
        if (addedOrRemovedItems) {
            // push sub-command representing aggregated adding/removal of items
            AddOrRemoveItemsCommand *arCmd = new AddOrRemoveItemsCommand(this, addedItems, removedItems);
            undoStack_.push(arCmd);
        }
        // push sub-commands representing individual item modifications
        foreach (QUndoCommand *undoCmd, undoCommands)
            undoStack_.push(undoCmd);
        if (!undoCommands.empty())
            repaintNeeded_ = true; // assume that any item modification requires a repaint
        undoStack_.endMacro();
        skipRepaint_ = false;
        repaintNeeded_ = true;
    }

    if (repaintNeeded_)
        repaint();
}

// Handles a key press event for an item in the process of being completed.
void EditItemManager::incompleteKeyPress(QKeyEvent *event)
{
    Q_ASSERT(incompleteItem_);
    bool rpn = false;
    bool complete = false;
    bool aborted = false;
    incompleteItem_->incompleteKeyPress(event, &rpn, &complete, &aborted);
    if (complete) {
        addItem(incompleteItem_); // causes repaint
        incompleteItem_ = 0;
        emit incompleteEditing(false);
    } else {
        if (aborted) {
            delete incompleteItem_; // or leave it to someone else?
            incompleteItem_ = 0;
            emit incompleteEditing(false);
        }
        if (rpn)
            repaint();
    }
}

void EditItemManager::keyRelease(QKeyEvent *event)
{
    if (incompleteItem_) {
        incompleteKeyRelease(event);
        return;
    }

    bool rpNeeded = false; // whether at least one item needs to be repainted after processing the event

    // send to selected items
    foreach (EditItemBase *item, selItems_) {
        bool rpn = false;
        item->keyRelease(event, &rpn);
        if (rpn)
            rpNeeded = true;
    }

    if (rpNeeded)
        emit repaintNeeded();
}

// Handles a key release event for an item in the process of being completed.
void EditItemManager::incompleteKeyRelease(QKeyEvent *event)
{
    Q_ASSERT(incompleteItem_);
    bool rpn = false;
    incompleteItem_->incompleteKeyRelease(event, &rpn);
    if (rpn)
        repaint();
}

void EditItemManager::draw()
{
    Q_ASSERT(!items_.contains(incompleteItem_));
    foreach (EditItemBase *item, items_) {
        EditItemBase::DrawModes modes = EditItemBase::Normal; // ### always set, so redundant?
        if (selItems_.contains(item))
            modes |= EditItemBase::Selected;
        if (item == hoverItem_)
            modes |= EditItemBase::Hovered;
        item->draw(modes, false);
    }
    if (incompleteItem_) // note that only complete items may be selected
        incompleteItem_->draw((incompleteItem_ == hoverItem_) ? EditItemBase::Hovered : EditItemBase::Normal, true);
    emit paintDone();
}

void EditItemManager::undo()
{
    undoStack_.undo();
}

void EditItemManager::redo()
{
    undoStack_.redo();
}

void EditItemManager::repaint()
{
    if (!skipRepaint_)
        emit repaintNeeded();
}

QList<EditItemBase *> EditItemManager::findHitItems(const QPoint &pos) const
{
    QList<EditItemBase *> hitItems;
    foreach (EditItemBase *item, items_)
        if (item->hit(pos, selItems_.contains(item)))
            hitItems.append(item);
    return hitItems;
}
