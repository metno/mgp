#ifndef TOPHEADERVIEW_H
#define TOPHEADERVIEW_H

#include <QGraphicsView>

class TopHeaderScene;

class TopHeaderView : public QGraphicsView
{
    Q_OBJECT

public:
    TopHeaderView(TopHeaderScene *, QWidget * = 0);

public slots:
    void updateScale(qreal, qreal);

private:
    virtual void resizeEvent(QResizeEvent *);

signals:
    void resized();
};

#endif // TOPHEADERVIEW_H
