#include <QtOpenGL>
#include "edititemx.h"

class SetGeometryCommand : public QUndoCommand
{
public:
    SetGeometryCommand(EditItemX *, const QRect &, const QRect &);
private:
    EditItemX *item_;
    QRect oldGeometry_;
    QRect newGeometry_;
    virtual void undo();
    virtual void redo();
};

SetGeometryCommand::SetGeometryCommand(
    EditItemX *item, const QRect &oldGeometry, const QRect &newGeometry)
    : item_(item)
    , oldGeometry_(oldGeometry)
    , newGeometry_(newGeometry)
{}

void SetGeometryCommand::undo()
{
    item_->setGeometry(oldGeometry_);
    item_->repaint();
}

void SetGeometryCommand::redo()
{
    item_->setGeometry(newGeometry_);
    item_->repaint();
}

EditItemX::EditItemX()
    : rect_(QRect(20, 20, 100, 100))
    , moving_(false)
    , resizing_(false)
    , currCtrlPointIndex_(-1)
    , remove_(new QAction("Remove", 0))
    , split_(new QAction("Split", 0))
    , merge_(new QAction("Merge", 0))
    , contextMenu_(new QMenu)
{
    updateControlPoints();
    color_.setRed(64 + 128 * (float(qrand()) / RAND_MAX));
    color_.setGreen(64 + 128 * (float(qrand()) / RAND_MAX));
    color_.setBlue(64 + 128 * (float(qrand()) / RAND_MAX));
}

EditItemX::~EditItemX()
{
    delete remove_;
    delete contextMenu_;
}

bool EditItemX::hit(const QPoint &pos, bool selected) const
{
    return rect_.contains(pos) || (selected && (hitControlPoint(pos) >= 0));
}

bool EditItemX::hit(const QRect &bbox) const
{
    // qDebug() << "EditItemX::hit(QRect) ... (not implemented)";
    Q_UNUSED(bbox);
    return false; // for now
}

void EditItemX::mousePress(
    QMouseEvent *event, bool *repaintNeeded, QList<QUndoCommand *> *undoCommands, QSet<EditItemBase *> *items, bool *multiItemOp)
{
    Q_ASSERT(undoCommands);

    if (event->button() == Qt::LeftButton) {
        currCtrlPointIndex_ = hitControlPoint(event->pos());
        resizing_ = (currCtrlPointIndex_ >= 0);
        moving_ = !resizing_;
        if (moving_)
            preMoveRect_ = rect_;
        baseMousePos_ = event->pos();
        baseBottomLeftPos_ = rect_.bottomLeft();
        baseBottomRightPos_ = rect_.bottomRight();
        baseTopLeftPos_ = rect_.topLeft();
        baseTopRightPos_ = rect_.topRight();

        if (multiItemOp)
            *multiItemOp = moving_; // i.e. a move operation would apply to all selected items

    } else if (event->button() == Qt::RightButton) {
        if (items) {
            // open a context menu and perform the selected action
            contextMenu_->clear();
            contextMenu_->addAction(remove_);
            contextMenu_->addAction(split_);
            if (items->size() > 1)
                contextMenu_->addAction(merge_);
            QAction *action = contextMenu_->exec(event->globalPos(), remove_);
            if (action == remove_) {
                remove(repaintNeeded, items);
            } else if (action == split_) {
                split(repaintNeeded, undoCommands, items); // undoCommands passed here since this operation may modify the internal state of this item
            } else if (action == merge_) {
                merge(repaintNeeded, undoCommands, items); // ditto
            }
        }
    }
}

void EditItemX::mouseRelease(QMouseEvent *event, bool *repaintNeeded, QList<QUndoCommand *> *undoCommands)
{
    Q_UNUSED(event); 
    Q_ASSERT(repaintNeeded);
    Q_ASSERT(undoCommands);
    if (moving_ && (geometry() != preMoveGeometry()))
        undoCommands->append(new SetGeometryCommand(this, preMoveGeometry(), geometry()));

    moving_ = resizing_ = false;
    *repaintNeeded = false;
}

void EditItemX::mouseMove(QMouseEvent *event, bool *repaintNeeded)
{
    if (moving_) {
        move(event->pos());
        *repaintNeeded = true;
    } else if (resizing_) {
        resize(event->pos());
        *repaintNeeded = true;
    }
}

void EditItemX::mouseHover(QMouseEvent *event, bool *repaintNeeded)
{
    Q_UNUSED(event);
    Q_UNUSED(repaintNeeded);
}

void EditItemX::keyPress(QKeyEvent *event, bool *repaintNeeded, QList<QUndoCommand *> *undoCommands, QSet<EditItemBase *> *items)
{
    Q_UNUSED(undoCommands); // not used, since the key press currently doesn't modify this item
    // (it may mark it for removal, but adding and removing items is handled on the outside)

    if (items) {
        if ((event->key() == Qt::Key_Backspace) || (event->key() == Qt::Key_Delete)) {
            Q_ASSERT(items->contains(this));
            items->remove(this);
            *repaintNeeded = true;
        }
    }
}

void EditItemX::keyRelease(QKeyEvent *event, bool *repaintNeeded)
{
    Q_UNUSED(event);
    Q_UNUSED(repaintNeeded);
}

void EditItemX::draw(DrawModes modes)
{
    // draw the basic item
    glBegin(GL_POLYGON);
    glColor3ub(color_.red(), color_.green(), color_.blue());
    glVertex2i(rect_.left(),  rect_.bottom());
    glVertex2i(rect_.right(), rect_.bottom());
    glVertex2i(rect_.right(), rect_.top());
    glVertex2i(rect_.left(),  rect_.top());
    glEnd();

    // draw control points if we're selected
    if (modes & Selected)
        drawControlPoints();

    // draw highlighting if we're hovered
    if (modes & Hovered)
        drawHoverHighlighting();
}

// Returns the index (0..3) of the control point hit by \a pos, or -1 if no
// control point was hit.
int EditItemX::hitControlPoint(const QPoint &pos) const
{
    for (int i = 0; i < controlPoints_.size(); ++i)
        if (controlPoints_.at(i).contains(pos))
            return i;
    return -1;
}

void EditItemX::move(const QPoint &pos)
{
    const QPoint delta = pos - baseMousePos_;
    rect_.moveTo(baseTopLeftPos_ + delta);
    updateControlPoints();
}

void EditItemX::resize(const QPoint &pos)
{
    const QPoint delta = pos - baseMousePos_;
    switch (currCtrlPointIndex_) {
    case 0: rect_.setBottomLeft(  baseBottomLeftPos_ + delta); break;
    case 1: rect_.setBottomRight(baseBottomRightPos_ + delta); break;
    case 2: rect_.setTopLeft(        baseTopLeftPos_ + delta); break;
    case 3: rect_.setTopRight(      baseTopRightPos_ + delta); break;
    }
    updateControlPoints();
}

void EditItemX::updateControlPoints()
{
    controlPoints_.clear();
    QList<QPoint> corners = QList<QPoint>()
        << rect_.bottomLeft()
        << rect_.bottomRight()
        << rect_.topLeft()
        << rect_.topRight();
    const int size = 10, size_2 = size / 2;
    foreach (QPoint c, corners)
        controlPoints_.append(QRect(c.x() - size_2, c.y() - size_2, size, size));
}

void EditItemX::drawControlPoints()
{
    glColor3ub(0, 0, 0);
    foreach (QRect c, controlPoints_) {
        glBegin(GL_POLYGON);
        glVertex3i(c.left(),  c.bottom(), 1);
        glVertex3i(c.right(), c.bottom(), 1);
        glVertex3i(c.right(), c.top(),    1);
        glVertex3i(c.left(),  c.top(),    1);
        glEnd();
    }
}

void EditItemX::drawHoverHighlighting()
{
    const int pad = 2;
    glColor3ub(255, 0, 0);
    glBegin(GL_POLYGON);
    glVertex2i(rect_.left() - pad,  rect_.bottom() + pad);
    glVertex2i(rect_.right() + pad, rect_.bottom() + pad);
    glVertex2i(rect_.right() + pad, rect_.top() - pad);
    glVertex2i(rect_.left() - pad,  rect_.top() - pad);
    glEnd();
}

void EditItemX::remove(bool *repaintNeeded, QSet<EditItemBase *> *items)
{
    Q_ASSERT(items);
    Q_ASSERT(items->contains(this));

    // Option 1: remove this item only:
    // items->remove(this);

    // Option 2: remove all items:
    QSet<EditItemBase *>::iterator i = items->begin();
    while (i != items->end())
        i = items->erase(i);

    *repaintNeeded = true;
}

// Splits all items of type EditItemX in two, including this item.
void EditItemX::split(bool *repaintNeeded, QList<QUndoCommand *> *undoCommands, QSet<EditItemBase *> *items)
{
    Q_UNUSED(repaintNeeded); // no need to set this (item state changes implies adding new items which causes a repaint in itself)
    Q_ASSERT(undoCommands);
    Q_ASSERT(items);
    Q_ASSERT(items->contains(this));

    QSet<EditItemBase *> addedItems;
    foreach (EditItemBase *item, *items) {
        EditItemX *itemx;
        if ((itemx = qobject_cast<EditItemX *>(item))) {
            // split itemx in two by
            // ... creating a new item
            EditItemX *addedItem = new EditItemX();
            addedItem->setGeometry(itemx->lowerHalf());
            addedItems.insert(addedItem);

            // ... and modifing the state of itemx through an undo command
            undoCommands->append(new SetGeometryCommand(itemx, itemx->geometry(), itemx->upperHalf()));
        }
    }
    if (!addedItems.empty()) {
        // inform the caller that new items were added (the caller is then responsible for registering the
        // appropriate undo command)
        items->unite(addedItems);
   }
}

// Merges items of type EditItemX into one by
//   1) resizing this item to the bounding box of all items of type EditItemX, and
//   2) removing all items of type EditItemX but this item
void EditItemX::merge(bool *repaintNeeded, QList<QUndoCommand *> *undoCommands, QSet<EditItemBase *> *items)
{
    Q_UNUSED(repaintNeeded); // no need to set this (item state changes implies removing items which causes a repaint in itself)
    Q_ASSERT(undoCommands);
    Q_ASSERT(items);
    Q_ASSERT(items->contains(this));

    // find all items of the same type as this item (excluding this item)
    QList<EditItemX *> mergeItems;
    foreach (EditItemBase *item, *items) {
        EditItemX *itemx;
        if ((itemx = qobject_cast<EditItemX *>(item)) && (itemx != this))
            mergeItems.append(itemx);
    }
    if (!mergeItems.empty()) {
        QRect boundingBox = geometry();
        // expand the bounding box gradually, removing each contributing item along the way
        foreach (EditItemX *item, mergeItems) {
            boundingBox = boundingBox.united(item->geometry());
            items->remove(item);
        }

        // modify the state of this item through an undo command
        undoCommands->append(new SetGeometryCommand(this, geometry(), boundingBox));
    }
}

void EditItemX::setGeometry(const QRect &r)
{
    rect_ = r;
    updateControlPoints();
}

QRect EditItemX::upperHalf() const
{
    QRect r = rect_;
    r.setBottom((r.top() + r.bottom()) / 2);
    return r;
}

QRect EditItemX::lowerHalf() const
{
    QRect r = rect_;
    r.setTop((r.top() + r.bottom()) / 2);
    return r;
}
