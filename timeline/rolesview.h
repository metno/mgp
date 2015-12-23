#ifndef ROLESVIEW_H
#define ROLESVIEW_H

#include <QGraphicsView>

class RolesScene;

class RolesView : public QGraphicsView
{
    Q_OBJECT

public:
    RolesView(RolesScene *, QWidget * = 0);

public slots:
    void updateScale(qreal, qreal);

private:
    virtual void resizeEvent(QResizeEvent *);
    virtual void leaveEvent(QEvent *);

signals:
    void resized();
};

#endif // ROLESVIEW_H
