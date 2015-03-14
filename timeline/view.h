#ifndef VIEW_H
#define VIEW_H

#include <QtGui>

class View : public QGraphicsView
{
    Q_OBJECT
public:
    View(QGraphicsScene *, QWidget * = 0);
protected:
    void resizeEvent(QResizeEvent *);
};

#endif // VIEW_H
