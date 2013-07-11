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
    void addItem(EditItemBase *);
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
private:
    QSet<EditItemBase *> items_;
    QSet<EditItemBase *> selItems_;
    EditItemBase *hoverItem_;
    bool repaintNeeded_;
    bool skipRepaint_;
    QUndoStack undoStack_;
    void addItem_(EditItemBase *);
    void addItems(const QSet<EditItemBase *> &);
    void removeItems(const QSet<EditItemBase *> &);
    QList<EditItemBase *> findHitItems(const QPoint &) const;
};

#endif // EDITITEMMANAGER_H
