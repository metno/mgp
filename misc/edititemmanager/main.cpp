#include <QtGui>
#include <QtOpenGL>

class Canvas : public QGLWidget
{
    Q_OBJECT
public:
    Canvas(QWidget *parent = 0)
        : QGLWidget(parent)
        , focus_(false)
    {
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
    }

private:
    void initializeGL()
    {
        glEnable(GL_DEPTH_TEST);
    }

    void resizeGL(int w, int h)
    {
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, w, 0, h, -2, 2);
     }

    void paintGL()
    {
        qglClearColor(QColor(204, 204, 204));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        emit paint();
    }

    QMouseEvent flippedYPos(QMouseEvent *event) const
    {
        return QMouseEvent(
            event->type(),
            QPoint(event->pos().x(), height() - event->pos().y()),
            event->globalPos(),
            event->button(),
            event->buttons(),
            event->modifiers()
            );
    }

    void mousePressEvent(QMouseEvent *event)
    {
        QMouseEvent fe = flippedYPos(event);
        emit mousePressed(&fe);
    }
    void mouseReleaseEvent(QMouseEvent *event)
    {
        QMouseEvent fe = flippedYPos(event);
        emit mouseReleased(&fe);
    }
    void mouseMoveEvent(QMouseEvent *event)
    {
        QMouseEvent fe = flippedYPos(event);
        emit mouseMoved(&fe);
    }
    void keyPressEvent(QKeyEvent *event) { emit keyPressed(event); }
    void keyReleaseEvent(QKeyEvent *event) { emit keyReleased(event); }

    void focusInEvent(QFocusEvent *)
    {
        focus_ = true;
        update();
    }

    void focusOutEvent(QFocusEvent *)
    {
        focus_ = false;
        update();
    }

    bool focus_;

signals:
    void mousePressed(QMouseEvent *);
    void mouseReleased(QMouseEvent *);
    void mouseMoved(QMouseEvent *);
    void keyPressed(QKeyEvent *);
    void keyReleased(QKeyEvent *);
    void paint();

public slots:
    void doSwapBuffers()
    {
        if (focus_) {
            glColor3ub(0, 128, 0);
            renderText(2, height() - 2, "keyboard focus");
        } else {
            glColor3ub(255, 0, 0);
            renderText(2, height() - 2, "no keyboard focus");
        }

        swapBuffers();
    }

    void doRepaint() { update(); }
};


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
    // The second argument is, if non-null, a set of items that may potentially be operated on (always including this item).
    //
    // Before returning, the function must modify the set to reflect how items were removed or inserted.
    //
    // NOTE: The event handler may create new items, but not delete existing ones (i.e. by (in)directly invoking the delete operator).
    // Deletion must be handled from the outside.
    //
    virtual void mousePress(QMouseEvent *, bool *, QSet<EditItemBase *> * = 0, bool * = 0) = 0;

    virtual void mouseRelease(QMouseEvent *, bool *) = 0;
    virtual void mouseMove(QMouseEvent *, bool *) = 0;
    virtual void mouseHover(QMouseEvent *, bool *) = 0;

    // Handles a key press event (see documentation for mousePress()).
    virtual void keyPress(QKeyEvent *, bool *, QSet<EditItemBase *> * = 0) = 0;
    virtual void keyRelease(QKeyEvent *, bool *) = 0;

    virtual void draw(DrawModes) = 0;

    int id() const { return id_; }

protected:
    EditItemBase()
        : id_(nextId())
    {
    }

private:
    int id_;
    static int nextId_;
    int nextId()
    {
        return nextId_++; // ### not thread safe; use a mutex for that
    }
};

int EditItemBase::nextId_ = 0;

Q_DECLARE_OPERATORS_FOR_FLAGS(EditItemBase::DrawModes)


class EditItemX : public EditItemBase
{
    Q_OBJECT
public:
    EditItemX()
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

    virtual ~EditItemX()
    {
        delete remove_;
        delete contextMenu_;
    }

private:

    virtual bool hit(const QPoint &pos, bool selected) const
    {
        return rect_.contains(pos) || (selected && (hitControlPoint(pos) >= 0));
    }

    virtual bool hit(const QRect &bbox) const
    {
        // qDebug() << "EditItemX::hit(QRect) ... (not implemented)";
        Q_UNUSED(bbox);
        return false; // for now
    }

    virtual void mousePress(QMouseEvent *event, bool *repaintNeeded, QSet<EditItemBase *> *items, bool *multiItemOp)
    {
        if (event->button() == Qt::LeftButton) {
            currCtrlPointIndex_ = hitControlPoint(event->pos());
            resizing_ = (currCtrlPointIndex_ >= 0);
            moving_ = !resizing_;
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
                    split(repaintNeeded, items);
                } else if (action == merge_) {
                    merge(repaintNeeded, items);
                }
            }
        }
    }

    virtual void mouseRelease(QMouseEvent *event, bool *repaintNeeded)
    {
        Q_UNUSED(event);
        moving_ = resizing_ = false;
        *repaintNeeded = false;
    }

    virtual void mouseMove(QMouseEvent *event, bool *repaintNeeded)
    {
        if (moving_) {
            move(event->pos());
            *repaintNeeded = true;
        } else if (resizing_) {
            resize(event->pos());
            *repaintNeeded = true;
        }
    }

    virtual void mouseHover(QMouseEvent *event, bool *repaintNeeded)
    {
        Q_UNUSED(event);
        Q_UNUSED(repaintNeeded);
    }

    virtual void keyPress(QKeyEvent *event, bool *repaintNeeded, QSet<EditItemBase *> *items)
    {
        if (items) {
            if ((event->key() == Qt::Key_Backspace) || (event->key() == Qt::Key_Delete)) {
                Q_ASSERT(items->contains(this));
                items->remove(this);
                *repaintNeeded = true;
            }
        }
    }

    virtual void keyRelease(QKeyEvent *event, bool *repaintNeeded)
    {
        Q_UNUSED(event);
        Q_UNUSED(repaintNeeded);
    }

    virtual void draw(DrawModes modes)
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
    int hitControlPoint(const QPoint &pos) const
    {
        for (int i = 0; i < controlPoints_.size(); ++i)
            if (controlPoints_.at(i).contains(pos))
                return i;
        return -1;
    }

    void move(const QPoint &pos)
    {
        const QPoint delta = pos - baseMousePos_;
        rect_.moveTo(baseTopLeftPos_ + delta);
        updateControlPoints();
    }

    void resize(const QPoint &pos)
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

    void updateControlPoints()
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

    void drawControlPoints()
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

    void drawHoverHighlighting()
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

    void remove(bool *repaintNeeded, QSet<EditItemBase *> *items)
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

    // Splits all items of type EditItemX - including this item - in two.
    void split(bool *repaintNeeded, QSet<EditItemBase *> *items)
    {
        Q_ASSERT(items);
        Q_ASSERT(items->contains(this));
        QSet<EditItemBase *> addedItems;

        foreach (EditItemBase *item, *items) {
            EditItemX *itemx;
            if ((itemx = qobject_cast<EditItemX *>(item))) {
                EditItemX *addedItem = new EditItemX();
                addedItem->setGeometry(itemx->lowerHalf());
                itemx->setGeometry(itemx->upperHalf());
                addedItems.insert(addedItem);
            }
        }
        if (!addedItems.empty()) {
            items->unite(addedItems);
            *repaintNeeded = true; // strictly not needed since 'items' was updated
        }
    }

    // Merges items of type EditItemX into one by
    //   1) resizing this item to the bounding box of all items of type EditItemX, and
    //   2) removing all items of type EditItemX but this item
    void merge(bool *repaintNeeded, QSet<EditItemBase *> *items)
    {
        Q_ASSERT(items);
        Q_ASSERT(items->contains(this));
        QList<EditItemX *> mergeItems;
        foreach (EditItemBase *item, *items) {
            EditItemX *itemx;
            if ((itemx = qobject_cast<EditItemX *>(item)) && (itemx != this)) {
                mergeItems.append(itemx);
            }
        }
        if (!mergeItems.empty()) {
            QRect result = rect_;
            foreach (EditItemX *item, mergeItems) {
                result = result.united(item->rect_);
                items->remove(item);
            }
            rect_ = result;
            updateControlPoints();
            *repaintNeeded = true; // strictly not needed since 'items' was updated
        }
    }

    void setGeometry(const QRect &r)
    {
        rect_ = r;
        updateControlPoints();
    }

    QRect upperHalf() const
    {
        QRect r = rect_;
        r.setBottom((r.top() + r.bottom()) / 2);
        return r;
    }

    QRect lowerHalf() const
    {
        QRect r = rect_;
        r.setTop((r.top() + r.bottom()) / 2);
        return r;
    }

    QRect rect_;
    QList<QRect> controlPoints_;

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


class EditItemManager : public QObject
{
    Q_OBJECT
public:
    EditItemManager()
        : hoverItem_(0)
    {}

    void addItem(EditItemBase *item)
    {
        items_.insert(item);
        emit repaintNeeded();
    }

    void removeItem(EditItemBase *item)
    {
        // qDebug() << "EditItemManager::removeItem() ... (not implemented)";
        Q_UNUSED(item);
    }

public slots:
    void mousePress(QMouseEvent *event)
    {
        const QList<EditItemBase *> hitItems = findHitItems(event->pos());
        EditItemBase *hitItem = // consider only this item to be hit
            hitItems.empty()
            ? 0
            : hitItems.first(); // for now; eventually use the one with higher z-value etc. ... 2 B DONE
        const bool hitSelItem = selItems_.contains(hitItem); // whether an already selected item was hit
        const bool shiftPressed = event->modifiers() & Qt::ShiftModifier;

        bool rpNeeded = false; // whether at least one item needs to be repainted after processing the event
        bool itemSetsModified = false; // whether item sets were modified

        QSet<EditItemBase *> origSelItems(selItems_);

        // update selection and hit status
        if (!(hitSelItem || (hitItem && shiftPressed))) {
            selItems_.clear();
        } else if (shiftPressed && hitSelItem && (selItems_.size() > 1)) {
            selItems_.remove(hitItem);
            hitItem = 0;
        }

        if (hitItem) { // an item is still considered hit
            selItems_.insert(hitItem); // ensure the hit item is selected (it might already be)

            // send mouse press to the hit item
            bool multiItemOp = false;
            QSet<EditItemBase *> eventItems;
            prepareItemSetUpdate(eventItems);
            bool rpn = false;
            hitItem->mousePress(event, &rpn, &eventItems, &multiItemOp);
            if (rpn) rpNeeded = true;
            bool modified = false;
            updateItemSets(eventItems, &modified);
            if (modified) itemSetsModified = true;

            if (items_.contains(hitItem)) {
                // the hit item is still there
                if (multiItemOp) {
                    // send the mouse press to other selected items
                    // (note that these are not allowed to modify item sets, nor does it make sense for them to flag
                    // the event as the beginning of a potential multi-item operation)
                    foreach (EditItemBase *item, selItems_)
                        if (item != hitItem) {
                            rpn = false;
                            item->mousePress(event, &rpn);
                            if (rpn)
                                rpNeeded = true;
                        }
                }
            } else {
                // the hit item removed itself as a result of the mouse press and it makes no sense
                // to send the mouse press to other items
            }
        }

        if (rpNeeded || itemSetsModified || (selItems_ != origSelItems))
            emit repaintNeeded();
    }

    void mouseRelease(QMouseEvent *event)
    {
        bool rpNeeded = false; // whether at least one item needs to be repainted after processing the event

        // send to selected items
        foreach (EditItemBase *item, selItems_)
            item->mouseRelease(event, &rpNeeded);

        if (rpNeeded)
            emit repaintNeeded();
    }

    void mouseMove(QMouseEvent *event)
    {
        // Check if the event is part of a multi-select operation using a rubberband-rectangle.
        // In that case, the event should only be used to update the selection (and tell items to redraw themselves as
        // approproate), and NOT be passed on through the mouseMove() functions of the selected items ... 2 B DONE!

        bool rpNeeded = false; // whether at least one item needs to be repainted after processing the event

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
                hoverItem_->mouseHover(event, &rpNeeded);
            } else {
                // ignore hovering outside any item
            }
        } else {
            // send move event to all selected items
            foreach (EditItemBase *item, selItems_) {
                bool rpn = false;
                item->mouseMove(event, &rpn);
                if (rpn)
                    rpNeeded = true;
            }
        }

        if (rpNeeded || (hoverItem_ != origHoverItem))
            emit repaintNeeded();
    }

    void keyPress(QKeyEvent *event)
    {
        bool rpNeeded = false; // whether at least one item needs to be repainted after processing the event
        bool itemSetsModified = false; // whether item sets were modified

        //QSet<EditItemBase *> origSelItems(selItems_);

        QSet<int> origSelIds; // IDs of the originally selected items
        foreach (EditItemBase *item, selItems_)
            origSelIds.insert(item->id());

        // process each of the originally selected items
        foreach (int origSelId, origSelIds) {

            // at this point, the item may or may not exist (it may have been removed in an earlier iteration)

            EditItemBase *origSelItem = idToItem(items_, origSelId);
            if (origSelItem) {
                // it still exists, so pass the event
                QSet<EditItemBase *> eventItems;
                prepareItemSetUpdate(eventItems);
                bool rpn = false;
                origSelItem->keyPress(event, &rpn, &eventItems);
                if (rpn) rpNeeded = true;
                bool modified = false;
                updateItemSets(eventItems, &modified);
                if (modified) itemSetsModified = true;
            }
        }

        if (rpNeeded || itemSetsModified)
            emit repaintNeeded();
    }

    void keyRelease(QKeyEvent *event)
    {
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

    void draw()
    {
        foreach (EditItemBase *item, items_) {
            EditItemBase::DrawModes modes = EditItemBase::Normal; // ### always set, so redundant?
            if (selItems_.contains(item))
                modes |= EditItemBase::Selected;
            if (item == hoverItem_)
                modes |= EditItemBase::Hovered;
            item->draw(modes);
        }
        emit paintDone();
    }

signals:
    void paintDone();
    void repaintNeeded();

private:
    QSet<EditItemBase *> items_;
    QSet<EditItemBase *> selItems_;
    EditItemBase * hoverItem_;

    QList<EditItemBase *> findHitItems(const QPoint &pos) const
    {
        QList<EditItemBase *> hitItems;
        foreach (EditItemBase *item, items_)
            if (item->hit(pos, selItems_.contains(item)))
                hitItems.append(item);
        return hitItems;
    }

    QMap<EditItemBase *, int> addr2id_;

    // Function to be called before calling an event handler that may modify a set of items.
    // Upon returning, eventItems contains the set of items that the event handler is allowed to operate on.
    void prepareItemSetUpdate(QSet<EditItemBase *> &eventItems)
    {
        // allow event handler to operate on all currently selected items
        eventItems = selItems_;

        // prepare ID validation
        addr2id_.clear();
        foreach (EditItemBase *item, eventItems)
            addr2id_.insert(item, item->id());
    }

    // Function to be called after calling an event handler that may modify a set of items.
    // eventItems contains the set of items resulting from the operation executed by the event handler.
    // Some items may have been removed, some may have been added.
    void updateItemSets(const QSet<EditItemBase *> &eventItems, bool *modified)
    {
        // validate IDs
        foreach (EditItemBase *item, eventItems)
            if (addr2id_.contains(item)) {
                // if the following assertion fails, it would mean that the event handler deleted an item A and
                // created an item B that happened to be allocated at the same memory address; note that items
                // are not supposed to be deleted in the event handler
                Q_ASSERT(addr2id_.value(item) == item->id());
            }

        // update items_ and selItems_ according to how the event handler modified the set of items
        // it was allowed to operate on (i.e. selItems_ --- see prepareItemSetUpdate())
        QSet<EditItemBase *> removedItems = selItems_ - eventItems;
        QSet<EditItemBase *> addedItems = eventItems - selItems_;
        foreach (EditItemBase *item, removedItems) {
            items_.remove(item);
            selItems_.remove(item);
            delete item;
            *modified = true;
        }
        foreach (EditItemBase *item, addedItems) {
            items_.insert(item);
            if (false) selItems_.insert(item); // for now, don't pre-select new items
           *modified = true;
        }
    }

    static EditItemBase *idToItem(const QSet<EditItemBase *> &items, int id)
    {
        foreach (EditItemBase *item, items)
            if (id == item->id())
                return item;
        return 0;
    }
};


class Window : public QWidget
{
    Q_OBJECT
public:
    Window(EditItemManager *editItemMgr)
        : editItemMgr_(editItemMgr)
    {
        QHBoxLayout *layout = new QHBoxLayout;

        QVBoxLayout *leftLayout = new QVBoxLayout;

        QPushButton *button1 = new QPushButton("create rectangle");
        button1->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        connect(button1, SIGNAL(clicked()), this, SLOT(addItemType1()));
        leftLayout->addWidget(button1);

        QPushButton *button2 = new QPushButton("create polygon");
        button2->setEnabled(false); // for now
        button2->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        leftLayout->addWidget(button2);

        QPushButton *button3 = new QPushButton("create polygon cutter");
        button3->setEnabled(false); // for now
        button3->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        leftLayout->addWidget(button3);

        leftLayout->addStretch();
        layout->addLayout(leftLayout);

        QVBoxLayout *rightLayout = new QVBoxLayout;
        Canvas *canvas = new Canvas;
        canvas->show();
        rightLayout->addWidget(canvas);
        layout->addLayout(rightLayout);

        setLayout(layout);

        connect(canvas, SIGNAL(mousePressed(QMouseEvent *)), editItemMgr, SLOT(mousePress(QMouseEvent *)));
        connect(canvas, SIGNAL(mouseReleased(QMouseEvent *)), editItemMgr, SLOT(mouseRelease(QMouseEvent *)));
        connect(canvas, SIGNAL(mouseMoved(QMouseEvent *)), editItemMgr, SLOT(mouseMove(QMouseEvent *)));
        connect(canvas, SIGNAL(keyPressed(QKeyEvent *)), editItemMgr, SLOT(keyPress(QKeyEvent *)));
        connect(canvas, SIGNAL(keyReleased(QKeyEvent *)), editItemMgr, SLOT(keyRelease(QKeyEvent *)));
        connect(canvas, SIGNAL(paint()), editItemMgr, SLOT(draw()));
        connect(editItemMgr, SIGNAL(paintDone()), canvas, SLOT(doSwapBuffers()));
        connect(editItemMgr, SIGNAL(repaintNeeded()), canvas, SLOT(doRepaint()));
    }

private:
    EditItemManager *editItemMgr_;

private slots:
    void addItemType1()
    {
        editItemMgr_->addItem(new EditItemX);
    }
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Window window(new EditItemManager);
    window.resize(800, 600);
    window.show();
    return app.exec();
}

#include "main.moc"
