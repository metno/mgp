#ifndef LEFTHEADERVIEW_H
#define LEFTHEADERVIEW_H

#include <QGraphicsView>

class LeftHeaderScene;

class LeftHeaderView : public QGraphicsView
{
    Q_OBJECT

public:
    LeftHeaderView(LeftHeaderScene *, QWidget * = 0);

public slots:
    void updateScale(qreal, qreal);

private:
    virtual void resizeEvent(QResizeEvent *);

signals:
    void resized();
};

#endif // LEFTHEADERVIEW_H
