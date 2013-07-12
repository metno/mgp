#include <QtOpenGL>
#include "rectangle.h"

namespace EditItem_Rectangle {

class SetGeometryCommand : public QUndoCommand
{
public:
    SetGeometryCommand(Rectangle *, const QRect &, const QRect &);
private:
    Rectangle *item_;
    QRect oldGeometry_;
    QRect newGeometry_;
    virtual void undo();
    virtual void redo();
};

SetGeometryCommand::SetGeometryCommand(
    Rectangle *item, const QRect &oldGeometry, const QRect &newGeometry)
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

Rectangle::Rectangle(PlacementMode placementMode)
    : rect_(QRect(20, 20, 100, 100))
    , placementMode_(placementMode)
    , moving_(false)
    , resizing_(false)
    , pressedCtrlPointIndex_(-1)
    , hoveredCtrlPointIndex_(-1)
    , placementPos1_(0)
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

Rectangle::~Rectangle()
{
    delete remove_;
    delete split_;
    delete merge_;
    delete contextMenu_;
}

bool Rectangle::hit(const QPoint &pos, bool selected) const
{
    return rect_.contains(pos) || (selected && (hitControlPoint(pos) >= 0));
}

bool Rectangle::hit(const QRect &bbox) const
{
    // qDebug() << "Rectangle::hit(QRect) ... (not implemented)";
    Q_UNUSED(bbox);
    return false; // for now
}

void Rectangle::mousePress(
    QMouseEvent *event, bool *repaintNeeded, QList<QUndoCommand *> *undoCommands, QSet<EditItemBase *> *items, bool *multiItemOp)
{
    Q_ASSERT(undoCommands);

    if (event->button() == Qt::LeftButton) {
        pressedCtrlPointIndex_ = hitControlPoint(event->pos());
        resizing_ = (pressedCtrlPointIndex_ >= 0);
        moving_ = !resizing_;
        baseRect_ = rect_;
        baseMousePos_ = event->pos();

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

// Handles a mouse press event when this item is incomplete, i.e. still in the process of being manually placed.
void Rectangle::incompleteMousePress(QMouseEvent *event, bool *repaintNeeded, bool *complete, bool *aborted)
{
    Q_UNUSED(repaintNeeded); // no need to set this (item state change implies setting *complete to true which causes a repaint in itself)
    Q_UNUSED(aborted);
    if (event->button() != Qt::LeftButton)
        return;

    if (placementMode_ == Instant) {
        rect_.moveTopLeft(event->pos());
        rect_.setSize(QSize(50, 50));
        updateControlPoints();
        *complete = true; // causes repaint
    } else {
        Q_ASSERT(placementMode_ == Resize);
        if (placementPos1_ == 0) {
            placementPos1_ = new QPoint(event->pos());
        } else {
            *complete = true; // causes repaint
            delete placementPos1_;
            placementPos1_ = 0;
        }
    }
}

void Rectangle::mouseRelease(QMouseEvent *event, bool *repaintNeeded, QList<QUndoCommand *> *undoCommands)
{
    Q_UNUSED(event); 
    Q_UNUSED(repaintNeeded); // no need to set this
    Q_ASSERT(undoCommands);
    if ((moving_ || resizing_) && (geometry() != baseGeometry()))
        undoCommands->append(new SetGeometryCommand(this, baseGeometry(), geometry()));

    moving_ = resizing_ = false;
}

// Handles a mouse release event when this item is incomplete, i.e. still in the process of being manually placed.
void Rectangle::incompleteMouseRelease(QMouseEvent *event, bool *repaintNeeded, bool *complete, bool *aborted)
{
    Q_UNUSED(event);
    Q_UNUSED(repaintNeeded);
    Q_UNUSED(complete);
    Q_UNUSED(aborted);
}

void Rectangle::mouseMove(QMouseEvent *event, bool *repaintNeeded)
{
    if (moving_) {
        move(event->pos());
        *repaintNeeded = true;
    } else if (resizing_) {
        resize(event->pos());
        *repaintNeeded = true;
    }
}

// Handles a mouse move event when this item is incomplete, i.e. still in the process of being manually placed.
void Rectangle::incompleteMouseMove(QMouseEvent *event, bool *repaintNeeded)
{
    Q_UNUSED(event);
    Q_UNUSED(repaintNeeded);
}

void Rectangle::mouseHover(QMouseEvent *event, bool *repaintNeeded)
{
    const int origHoveredCtrlPointIndex = hoveredCtrlPointIndex_;
    hoveredCtrlPointIndex_ = hitControlPoint(event->pos());
    if (hoveredCtrlPointIndex_ != origHoveredCtrlPointIndex)
        *repaintNeeded = true;
}

// Handles a mouse hover event when this item is incomplete, i.e. still in the process of being manually placed.
void Rectangle::incompleteMouseHover(QMouseEvent *event, bool *repaintNeeded)
{
    if ((placementMode_ == Resize) && placementPos1_) {
        rect_.setTopLeft(*placementPos1_);
        rect_.setBottomRight(event->pos());
        updateControlPoints();
        *repaintNeeded = true;
    }
}

void Rectangle::keyPress(QKeyEvent *event, bool *repaintNeeded, QList<QUndoCommand *> *undoCommands, QSet<EditItemBase *> *items)
{
    Q_UNUSED(repaintNeeded); // no need to set this
    Q_UNUSED(undoCommands); // not used, since the key press currently doesn't modify this item
    // (it may mark it for removal, but adding and removing items is handled on the outside)

    if (items && ((event->key() == Qt::Key_Backspace) || (event->key() == Qt::Key_Delete))) {
        Q_ASSERT(items->contains(this));
        items->remove(this);
    }
}

// Handles a key press event when this item is incomplete, i.e. still in the process of being manually placed.
void Rectangle::incompleteKeyPress(QKeyEvent *event, bool *repaintNeeded, bool *complete, bool *aborted)
{
    Q_UNUSED(repaintNeeded);
    Q_UNUSED(complete);
    if (event->key() == Qt::Key_Escape) {
        *aborted = true;
        if (placementPos1_) {
            delete placementPos1_;
            placementPos1_ = 0;
            *repaintNeeded = true;
        }
    }
}

void Rectangle::keyRelease(QKeyEvent *event, bool *repaintNeeded)
{
    Q_UNUSED(event);
    Q_UNUSED(repaintNeeded);
}

// Handles a key release event when this item is incomplete, i.e. still in the process of being manually placed.
void Rectangle::incompleteKeyRelease(QKeyEvent *event, bool *repaintNeeded)
{
    Q_UNUSED(event);
    Q_UNUSED(repaintNeeded);
}

void Rectangle::draw(DrawModes modes, bool incomplete)
{
    if (incomplete && ((placementMode_ == Instant) || ((placementMode_ == Resize) && (placementPos1_ == 0))))
        return;

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
        drawHoverHighlighting(incomplete);
}

// Returns the index (0..3) of the control point hit by \a pos, or -1 if no
// control point was hit.
int Rectangle::hitControlPoint(const QPoint &pos) const
{
    for (int i = 0; i < controlPoints_.size(); ++i)
        if (controlPoints_.at(i).contains(pos))
            return i;
    return -1;
}

void Rectangle::move(const QPoint &pos)
{
    const QPoint delta = pos - baseMousePos_;
    rect_.moveTo(baseRect_.topLeft() + delta);
    updateControlPoints();
}

void Rectangle::resize(const QPoint &pos)
{
    const QPoint delta = pos - baseMousePos_;
    switch (pressedCtrlPointIndex_) {
    case 0: rect_.setBottomLeft(  baseRect_.bottomLeft() + delta); break;
    case 1: rect_.setBottomRight(baseRect_.bottomRight() + delta); break;
    case 2: rect_.setTopLeft(        baseRect_.topLeft() + delta); break;
    case 3: rect_.setTopRight(      baseRect_.topRight() + delta); break;
    }
    updateControlPoints();
}

void Rectangle::updateControlPoints()
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

void Rectangle::drawControlPoints()
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

void Rectangle::drawHoverHighlighting(bool incomplete)
{
    const int pad = 1;
    if (incomplete)
        glColor3ub(0, 200, 0);
    else
        glColor3ub(255, 0, 0);

    const QRect *r = (hoveredCtrlPointIndex_ >= 0) ? &controlPoints_.at(hoveredCtrlPointIndex_) : &rect_;
    glPushAttrib(GL_LINE_BIT);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP);
    glVertex3i(r->left() - pad,  r->bottom() + pad, 1);
    glVertex3i(r->right() + pad, r->bottom() + pad, 1);
    glVertex3i(r->right() + pad, r->top() - pad, 1);
    glVertex3i(r->left() - pad,  r->top() - pad, 1);
    glEnd();
    glPopAttrib();
}

void Rectangle::remove(bool *repaintNeeded, QSet<EditItemBase *> *items)
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

// Splits all items of type Rectangle in two, including this item.
void Rectangle::split(bool *repaintNeeded, QList<QUndoCommand *> *undoCommands, QSet<EditItemBase *> *items)
{
    Q_UNUSED(repaintNeeded); // no need to set this (item state changes implies adding new items which causes a repaint in itself)
    Q_ASSERT(undoCommands);
    Q_ASSERT(items);
    Q_ASSERT(items->contains(this));

    QSet<EditItemBase *> addedItems;
    foreach (EditItemBase *item, *items) {
        Rectangle *itemx;
        if ((itemx = qobject_cast<Rectangle *>(item))) {
            // split itemx in two by
            // ... creating a new item
            Rectangle *addedItem = new Rectangle();
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

// Merges items of type Rectangle into one by
//   1) resizing this item to the bounding box of all items of type Rectangle, and
//   2) removing all items of type Rectangle but this item
void Rectangle::merge(bool *repaintNeeded, QList<QUndoCommand *> *undoCommands, QSet<EditItemBase *> *items)
{
    Q_UNUSED(repaintNeeded); // no need to set this (item state changes implies removing items which causes a repaint in itself)
    Q_ASSERT(undoCommands);
    Q_ASSERT(items);
    Q_ASSERT(items->contains(this));

    // find all items of the same type as this item (excluding this item)
    QList<Rectangle *> mergeItems;
    foreach (EditItemBase *item, *items) {
        Rectangle *itemx;
        if ((itemx = qobject_cast<Rectangle *>(item)) && (itemx != this))
            mergeItems.append(itemx);
    }
    if (!mergeItems.empty()) {
        QRect boundingBox = geometry();
        // expand the bounding box gradually, removing each contributing item along the way
        foreach (Rectangle *item, mergeItems) {
            boundingBox = boundingBox.united(item->geometry());
            items->remove(item);
        }

        // modify the state of this item through an undo command
        undoCommands->append(new SetGeometryCommand(this, geometry(), boundingBox));
    }
}

void Rectangle::setGeometry(const QRect &r)
{
    rect_ = r;
    updateControlPoints();
}

QRect Rectangle::upperHalf() const
{
    QRect r = rect_;
    r.setBottom((r.top() + r.bottom()) / 2);
    return r;
}

QRect Rectangle::lowerHalf() const
{
    QRect r = rect_;
    r.setTop((r.top() + r.bottom()) / 2);
    return r;
}

} // namespace EditItem_Rectangle
