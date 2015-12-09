#ifndef TIMELINEVIEW_H
#define TIMELINEVIEW_H

#include <QGraphicsView>

class TimelineScene;

class TimelineView : public QGraphicsView
{
    Q_OBJECT

public:
    TimelineView(TimelineScene *, QWidget * = 0);

public slots:
    void updateScale(qreal, qreal);

private:
    virtual void resizeEvent(QResizeEvent *);

signals:
    void resized();
};

#endif // TIMELINEVIEW_H
