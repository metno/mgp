#include <QtOpenGL>
#include "multiline.h"

namespace EditItem_MultiLine {

class SetGeometryCommand : public QUndoCommand
{
public:
    SetGeometryCommand(MultiLine *, const QList<QPoint> &, const QList<QPoint> &);
private:
    MultiLine *item_;
    QList<QPoint> oldGeometry_;
    QList<QPoint> newGeometry_;
    virtual void undo();
    virtual void redo();
};

SetGeometryCommand::SetGeometryCommand(
    MultiLine *item, const QList<QPoint> &oldGeometry, const QList<QPoint> &newGeometry)
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

MultiLine::MultiLine()
    : moving_(false)
    , resizing_(false)
    , pressedCtrlPointIndex_(-1)
    , hoveredCtrlPointIndex_(-1)
    , placementPos_(0)
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

MultiLine::~MultiLine()
{
    delete remove_;
    delete split_;
    delete merge_;
    delete contextMenu_;
}

bool MultiLine::hit(const QPoint &pos, bool selected) const
{
    const qreal proximityTolerance = 3.0;
    return ((points_.size() >= 2) && (distance(pos) < proximityTolerance)) || (selected && (hitControlPoint(pos) >= 0));
}

bool MultiLine::hit(const QRect &rect) const
{
    Q_UNUSED(rect);
    return false; // for now
}

void MultiLine::mousePress(
    QMouseEvent *event, bool *repaintNeeded, QList<QUndoCommand *> *undoCommands, QSet<EditItemBase *> *items, bool *multiItemOp)
{
    Q_ASSERT(undoCommands);

    if (event->button() == Qt::LeftButton) {
        pressedCtrlPointIndex_ = hitControlPoint(event->pos());
        resizing_ = (pressedCtrlPointIndex_ >= 0);
        moving_ = !resizing_;
        basePoints_ = points_;
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

void MultiLine::mouseRelease(QMouseEvent *event, bool *repaintNeeded, QList<QUndoCommand *> *undoCommands)
{
    Q_UNUSED(event); 
    Q_UNUSED(repaintNeeded); // no need to set this
    Q_ASSERT(undoCommands);
    if ((moving_ || resizing_) && (geometry() != baseGeometry()))
        undoCommands->append(new SetGeometryCommand(this, baseGeometry(), geometry()));
    moving_ = resizing_ = false;
}

void MultiLine::mouseMove(QMouseEvent *event, bool *repaintNeeded)
{
    if (moving_) {
        move(event->pos());
        *repaintNeeded = true;
    } else if (resizing_) {
        resize(event->pos());
        *repaintNeeded = true;
    }
}

void MultiLine::mouseHover(QMouseEvent *event, bool *repaintNeeded)
{
    const int origHoveredCtrlPointIndex = hoveredCtrlPointIndex_;
    hoveredCtrlPointIndex_ = hitControlPoint(event->pos());
    if (hoveredCtrlPointIndex_ != origHoveredCtrlPointIndex)
        *repaintNeeded = true;
}

void MultiLine::mouseDoubleClick(QMouseEvent *event, bool *repaintNeeded)
{
    Q_UNUSED(event);
    Q_UNUSED(repaintNeeded);
}

void MultiLine::keyPress(QKeyEvent *event, bool *repaintNeeded, QList<QUndoCommand *> *undoCommands, QSet<EditItemBase *> *items)
{
    Q_UNUSED(repaintNeeded); // no need to set this
    Q_UNUSED(undoCommands); // not used, since the key press currently doesn't modify this item
    // (it may mark it for removal, but adding and removing items is handled on the outside)

    if (items && ((event->key() == Qt::Key_Backspace) || (event->key() == Qt::Key_Delete))) {
        Q_ASSERT(items->contains(this));
        items->remove(this);
    }
}

void MultiLine::keyRelease(QKeyEvent *event, bool *repaintNeeded)
{
    Q_UNUSED(event);
    Q_UNUSED(repaintNeeded);
}

void MultiLine::incompleteMousePress(QMouseEvent *event, bool *repaintNeeded, bool *complete, bool *aborted)
{
    Q_UNUSED(complete);
    Q_UNUSED(aborted);
    if (event->button() != Qt::LeftButton)
        return;
    if (!placementPos_)
        placementPos_ = new QPoint(event->pos());
    points_.append(QPoint(event->pos()));
    updateControlPoints();
    *repaintNeeded = true;
}

void MultiLine::incompleteMouseRelease(QMouseEvent *event, bool *repaintNeeded, bool *complete, bool *aborted)
{
    Q_UNUSED(event);
    Q_UNUSED(repaintNeeded);
    Q_UNUSED(complete);
    Q_UNUSED(aborted);
}

void MultiLine::incompleteMouseMove(QMouseEvent *event, bool *repaintNeeded)
{
    Q_UNUSED(event);
    Q_UNUSED(repaintNeeded);
}

void MultiLine::incompleteMouseHover(QMouseEvent *event, bool *repaintNeeded)
{
    if (placementPos_) {
        *placementPos_ = event->pos();
        *repaintNeeded = true;
    }
}

void MultiLine::incompleteMouseDoubleClick(QMouseEvent *event, bool *repaintNeeded, bool *complete, bool *aborted)
{
    Q_UNUSED(event);
    Q_UNUSED(repaintNeeded);

    if (event->button() != Qt::LeftButton)
        return;

    Q_ASSERT(points_.size() >= 1); // the corresponding mouse press must have added a point

    Q_ASSERT(placementPos_);
    delete placementPos_;
    placementPos_ = 0;

    if (points_.size() >= 2) {
        *complete = true; // causes repaint
    } else {
        *aborted = true; // not a complete multiline
        *repaintNeeded = true;
    }
}

void MultiLine::incompleteKeyPress(QKeyEvent *event, bool *repaintNeeded, bool *complete, bool *aborted)
{
    Q_UNUSED(repaintNeeded);
    Q_UNUSED(complete);
    if (placementPos_ && ((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter))) {
        if (points_.size() >= 2) {
            *complete = true; // causes repaint
        } else {
            *aborted = true; // not a complete multiline
            *repaintNeeded = true;
        }
        delete placementPos_;
        placementPos_ = 0;
    } else if (event->key() == Qt::Key_Escape) {
        *aborted = true;
        if (placementPos_) {
            delete placementPos_;
            placementPos_ = 0;
            *repaintNeeded = true;
        }
    }
}

void MultiLine::incompleteKeyRelease(QKeyEvent *event, bool *repaintNeeded)
{
    Q_UNUSED(event);
    Q_UNUSED(repaintNeeded);
}

void MultiLine::draw(DrawModes modes, bool incomplete)
{
    if (incomplete) {
        if (placementPos_ == 0) {
            return;
        } else {
            // draw the line from the end of the multiline to the current placement position
            glBegin(GL_LINES);
            glColor3ub(0, 0, 255);
            glVertex2i(points_.last().x(), points_.last().y());
            glVertex2i(placementPos_->x(), placementPos_->y());
            glEnd();
        }
    }

    // draw the basic item
    glBegin(GL_LINE_STRIP);
    glColor3ub(color_.red(), color_.green(), color_.blue());
    foreach (QPoint p, points_)
        glVertex2i(p.x(), p.y());
    glEnd();

    // draw control points if we're selected
    if (modes & Selected)
        drawControlPoints();

    // draw highlighting if we're hovered
    if (modes & Hovered)
        drawHoverHighlighting(incomplete);
}

// Returns the index (>= 0)  of the control point hit by \a pos, or -1 if no
// control point was hit.
int MultiLine::hitControlPoint(const QPoint &pos) const
{
    for (int i = 0; i < controlPoints_.size(); ++i)
        if (controlPoints_.at(i).contains(pos))
            return i;
    return -1;
}

void MultiLine::move(const QPoint &pos)
{
    const QPoint delta = pos - baseMousePos_;
    Q_ASSERT(basePoints_.size() == points_.size());
    for (int i = 0; i < points_.size(); ++i)
        points_[i] = basePoints_.at(i) + delta;
    updateControlPoints();
}

void MultiLine::resize(const QPoint &pos)
{
    const QPoint delta = pos - baseMousePos_;
    Q_ASSERT(pressedCtrlPointIndex_ >= 0);
    Q_ASSERT(pressedCtrlPointIndex_ < controlPoints_.size());
    Q_ASSERT(basePoints_.size() == points_.size());
    points_[pressedCtrlPointIndex_] = basePoints_.at(pressedCtrlPointIndex_) + delta;
    updateControlPoints();
}

void MultiLine::updateControlPoints()
{
    controlPoints_.clear();
    const int size = 10, size_2 = size / 2;
    foreach (QPoint p, points_)
        controlPoints_.append(QRect(p.x() - size_2, p.y() - size_2, size, size));
}

void MultiLine::drawControlPoints()
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

void MultiLine::drawHoverHighlighting(bool incomplete)
{
    const int pad = 1;
    if (incomplete)
        glColor3ub(0, 200, 0);
    else
        glColor3ub(255, 0, 0);

    if (hoveredCtrlPointIndex_ >= 0) {
        // highlight a control point
        const QRect *r = &controlPoints_.at(hoveredCtrlPointIndex_);
        glPushAttrib(GL_LINE_BIT);
        glLineWidth(2);
        glBegin(GL_LINE_LOOP);
        glVertex3i(r->left() - pad,  r->bottom() + pad, 1);
        glVertex3i(r->right() + pad, r->bottom() + pad, 1);
        glVertex3i(r->right() + pad, r->top() - pad, 1);
        glVertex3i(r->left() - pad,  r->top() - pad, 1);
        glEnd();
        glPopAttrib();
    } else {
        // highlight the multiline itself
        glPushAttrib(GL_LINE_BIT);
        glLineWidth(4);
        glBegin(GL_LINE_STRIP);
        foreach (QPoint p, points_)
            glVertex3i(p.x(), p.y(), 1);
        glEnd();
        glPopAttrib();
    }
}

void MultiLine::remove(bool *repaintNeeded, QSet<EditItemBase *> *items)
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
void MultiLine::split(bool *repaintNeeded, QList<QUndoCommand *> *undoCommands, QSet<EditItemBase *> *items)
{
    // FOR NOW:
    Q_UNUSED(repaintNeeded);
    Q_UNUSED(undoCommands);
    Q_UNUSED(items);

    qDebug() << "split not implemented";

    //  Q_UNUSED(repaintNeeded); // no need to set this (item state changes implies adding new items which causes a repaint in itself)
    //  Q_ASSERT(undoCommands);
    //  Q_ASSERT(items);
    //  Q_ASSERT(items->contains(this));

    //  QSet<EditItemBase *> addedItems;
    //  foreach (EditItemBase *item, *items) {
    //      Rectangle *itemx;
    //      if ((itemx = qobject_cast<Rectangle *>(item))) {
    //          // split itemx in two by
    //          // ... creating a new item
    //          Rectangle *addedItem = new Rectangle();
    //          addedItem->setGeometry(itemx->lowerHalf());
    //          addedItems.insert(addedItem);

    //          // ... and modifing the state of itemx through an undo command
    //          undoCommands->append(new SetGeometryCommand(itemx, itemx->geometry(), itemx->upperHalf()));
    //      }
    //  }
    //  if (!addedItems.empty()) {
    //      // inform the caller that new items were added (the caller is then responsible for registering the
    //      // appropriate undo command)
    //      items->unite(addedItems);
    // }
}

// Merges items of type Rectangle into one by
//   1) resizing this item to the bounding box of all items of type Rectangle, and
//   2) removing all items of type Rectangle but this item
void MultiLine::merge(bool *repaintNeeded, QList<QUndoCommand *> *undoCommands, QSet<EditItemBase *> *items)
{
    // FOR NOW:
    Q_UNUSED(repaintNeeded);
    Q_UNUSED(undoCommands);
    Q_UNUSED(items);

    qDebug() << "merge not implemented";

    // Q_UNUSED(repaintNeeded); // no need to set this (item state changes implies removing items which causes a repaint in itself)
    // Q_ASSERT(undoCommands);
    // Q_ASSERT(items);
    // Q_ASSERT(items->contains(this));

    // // find all items of the same type as this item (excluding this item)
    // QList<Rectangle *> mergeItems;
    // foreach (EditItemBase *item, *items) {
    //     Rectangle *itemx;
    //     if ((itemx = qobject_cast<Rectangle *>(item)) && (itemx != this))
    //         mergeItems.append(itemx);
    // }
    // if (!mergeItems.empty()) {
    //     QRect boundingBox = geometry();
    //     // expand the bounding box gradually, removing each contributing item along the way
    //     foreach (Rectangle *item, mergeItems) {
    //         boundingBox = boundingBox.united(item->geometry());
    //         items->remove(item);
    //     }

    //     // modify the state of this item through an undo command
    //     undoCommands->append(new SetGeometryCommand(this, geometry(), boundingBox));
    // }
}

void MultiLine::setGeometry(const QList<QPoint> &points)
{
    points_ = points;
    updateControlPoints();
}

// to be used by split()
QList<QPoint> MultiLine::firstSegment(int ctrlPointIndex) const
{
    Q_UNUSED(ctrlPointIndex);
    return QList<QPoint>();
}

// to be used by split()
QList<QPoint> MultiLine::secondSegment(int ctrlPointIndex) const
{
    Q_UNUSED(ctrlPointIndex);
    return QList<QPoint>();
}

static qreal sqr(qreal x) { return x * x; }

static qreal dist2(const QPointF &v, const QPointF &w) { return sqr(v.x() - w.x()) + sqr(v.y() - w.y()); }

// Returns the distance between \a p and the line between \a v and \a w.
static qreal distance2(const QPointF &p, const QPointF &v, const QPointF &w)
{
    const qreal l2 = dist2(v, w);
    if (l2 == 0) return sqrt(dist2(p, v));
    Q_ASSERT(l2 > 0);
    const qreal t = ((p.x() - v.x()) * (w.x() - v.x()) + (p.y() - v.y()) * (w.y() - v.y())) / l2;
    if (t < 0) return sqrt(dist2(p, v));
    if (t > 1) return sqrt(dist2(p, w));
    QPointF p2(v.x() + t * (w.x() - v.x()), v.y() + t * (w.y() - v.y()));
    return sqrt(dist2(p, p2));
}

// Returns the distance between \a p and the multiline (i.e. the mimimum distance between \a p and any of the line segments).
// If the multiline contains fewer than two points, the function returns -1.
qreal MultiLine::distance(const QPoint &p) const
{
    if (points_.size() < 2)
        return -1;

    qreal minDist = -1;
    for (int i = 1; i < points_.size(); ++i) {
        const qreal dist = distance2(QPointF(p), QPointF(points_.at(i - 1)), QPointF(points_.at(i)));
        minDist = (i == 1) ? dist : qMin(minDist, dist);
    }
    Q_ASSERT(minDist >= 0);
    return minDist;
}

} // namespace EditItem_MultiLine
