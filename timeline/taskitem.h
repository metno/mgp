#ifndef TASKITEM_H
#define TASKITEM_H

#include <QGraphicsRectItem>
#include <QColor>
#include <QBrush>

class QGraphicsTextItem;

class TaskItem : public QGraphicsRectItem
{
public:
    TaskItem(qint64, qreal);
    qint64 taskId() const;
    qint64 roleId() const;
    void highlight(bool);
    void updateRect(const QRectF &);
    void updateName(const QString &);
    void updateSummary(const QString &);
    void updateDescription(const QString &);
    void updateColor(const QColor &);
    void updateTextPositions();
    void updateFontSize(qreal);

private:
    qint64 taskId_;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
    QGraphicsRectItem *hoverItem_;
    QGraphicsTextItem *nameItem_;
    QGraphicsTextItem *summaryItem_;
    //QGraphicsTextItem *descrItem_;

    static QColor defaultColor() { return QColor("#bbb"); }
};

#endif // TASKITEM_H
