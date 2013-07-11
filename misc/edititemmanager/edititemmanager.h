#ifndef EDITITEMMANAGER_H
#define EDITITEMMANAGER_H

#include <QtGui> // ### TODO: include relevant headers only

class EditItemBase;

class EditItemManager : public QObject
{
    Q_OBJECT
    friend class AddOrRemoveItemsCommand;
public:
    EditItemManager();
    void addItem(EditItemBase *, bool = false);
    void removeItem(EditItemBase *);
    QUndoStack *undoStack();
public slots:
    void mousePress(QMouseEvent *);
    void mouseRelease(QMouseEvent *);
    void mouseMove(QMouseEvent *);
    void keyPress(QKeyEvent *);
    void keyRelease(QKeyEvent *);
    void draw();
    void repaint();
    void undo();
    void redo();
signals:
    void paintDone();
    void repaintNeeded();
    void canUndoChanged(bool);
    void canRedoChanged(bool);
    void incompleteEditing(bool);
private:
    QSet<EditItemBase *> items_;
    QSet<EditItemBase *> selItems_;
    EditItemBase *hoverItem_;
    EditItemBase *incompleteItem_; // item in the process of being completed (e.g. having its control points manually placed)
    bool repaintNeeded_;
    bool skipRepaint_;
    QUndoStack undoStack_;
    void addItem_(EditItemBase *);
    void addItems(const QSet<EditItemBase *> &);
    void removeItems(const QSet<EditItemBase *> &);
    QList<EditItemBase *> findHitItems(const QPoint &) const;
    void incompleteMousePress(QMouseEvent *);
    void incompleteMouseRelease(QMouseEvent *);
    void incompleteMouseMove(QMouseEvent *);
    void incompleteKeyPress(QKeyEvent *);
    void incompleteKeyRelease(QKeyEvent *);
};

#endif // EDITITEMMANAGER_H
