#include "taskitem.h"
#include "taskmanager.h"
#include "common.h"
#include <QCursor>
#include <QPen>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QAction>
#include <QMenu>
#include <QCursor>
#include <QGraphicsTextItem>
#include <QFont>
#include <QGraphicsPathItem>
#include <QPainterPath>

TaskItem::TaskItem(qint64 taskId__, qreal fontSizeBaseFrac)
    : taskId_(taskId__)
{
    QSharedPointer<Task> task = TaskManager::instance().findTask(taskId_);
    Q_ASSERT(task);

    setBrush(task->color().isValid() ? task->color() : defaultColor());
    setPen(QPen(QColor("#555")));
    setCursor(Qt::ArrowCursor);
    setAcceptHoverEvents(true);

    hoverItem_ = new QGraphicsRectItem(this);
    hoverItem_->setFlag(QGraphicsItem::ItemStacksBehindParent);
    hoverItem_->setBrush(QColor("#ff0"));
    hoverItem_->setPen(QPen(QColor("#880")));
    hoverItem_->setVisible(false);

    nameItem_ = new QGraphicsTextItem(task->name(), this);
    nameItem_->setFont(QFont("helvetica", 14, QFont::Bold));
    nameItem_->setZValue(2);
    nameItem_->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    //nameItem_->setPos(rect().x() + 5, rect().y());

    summaryItem_ = new QGraphicsTextItem(task->summary(), this);
    summaryItem_->setFont(QFont("helvetica", 12, QFont::Normal));
    summaryItem_->setZValue(2);
    summaryItem_->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    //summaryItem_->setPos(rect().x() + 5, rect().y() + 5);

    shadowItem1_ = new QGraphicsPathItem(this);
    shadowItem1_->setPen(Qt::NoPen);
    shadowItem1_->setZValue(1.5);

    shadowItem2_ = new QGraphicsPathItem(this);
    shadowItem2_->setPen(Qt::NoPen);
    shadowItem2_->setZValue(1.5);

    updateTextPositions();
    updateFontSize(fontSizeBaseFrac);
    updateShadowPositions();
}

qint64 TaskItem::taskId() const
{
    return taskId_;
}

qint64 TaskItem::roleId() const
{
    QSharedPointer<Task> task = TaskManager::instance().findTask(taskId());
    return task ? task->roleId() : -1;
}

void TaskItem::mousePressEvent(QGraphicsSceneMouseEvent *)
{
//    qDebug() << "mousePressEvent for task" << taskId_;
}

void TaskItem::highlight(bool enabled)
{
    if (enabled) {
        Q_ASSERT(!scene()->views().isEmpty());
        const qreal w = 5;
        const qreal addh = w / scene()->views().first()->transform().m11();
        const qreal addv = w / scene()->views().first()->transform().m22();
        hoverItem_->setRect(rect().adjusted(-addh, -addv, addh, addv));
    }

    hoverItem_->setVisible(enabled);
}

void TaskItem::updateRect(const QRectF &r)
{
    setRect(r);
    summaryItem_->setTextWidth(scene()->views().first()->transform().m11() * r.width());
    updateTextPositions();
    updateShadowPositions();
}

void TaskItem::updateName(const QString &name)
{
    nameItem_->setPlainText(name);
}

void TaskItem::updateSummary(const QString &summary)
{
    summaryItem_->setPlainText(summary);
}

void TaskItem::updateDescription(const QString &descr)
{
    Q_UNUSED(descr);
//    descrItem_->setPlainText(descr);
}

void TaskItem::updateColor(const QColor &color)
{
    setBrush(color.isValid() ? color : defaultColor());
    shadowItem1_->setBrush(brush().color().darker(120));
    shadowItem2_->setBrush(brush().color().lighter(120));
}

void TaskItem::updateTextPositions()
{
    int addh = 5; // for now
    int addv_summary = 25; // for now (depends on font size etc.)
    if (scene() && (!scene()->views().isEmpty())) {
        addh /= scene()->views().first()->transform().m11();
        addv_summary /= scene()->views().first()->transform().m22();
    }
    const int addv_name = addh;

    nameItem_->setPos(rect().x() + addh, rect().y() + addv_name);
    summaryItem_->setPos(rect().x() + addh, rect().y() + addv_summary);
}

void TaskItem::updateFontSize(qreal baseFrac)
{
    const int minSize = 6;
    const int maxSize = 18;
    const int nameSize = minSize + baseFrac * (maxSize - minSize);
    const int summarySize = 0.8 * nameSize;

    QFont font = nameItem_->font();
    font.setPointSize(nameSize);
    nameItem_->setFont(font);

    font = summaryItem_->font();
    font.setPointSize(summarySize);
    summaryItem_->setFont(font);
}

void TaskItem::updateShadowPositions()
{
    if ((!scene()) || (scene()->views().isEmpty()))
        return;
    const qreal tx = scene()->views().first()->transform().m11();
    const qreal ty = scene()->views().first()->transform().m22();

    const qreal size = 3;
    const qreal sx = size / tx;
    const qreal sy = size / ty;
    const QRectF brect = boundingRect();
    const QPointF topLeft = brect.topLeft();
    const QPointF botRight = brect.bottomRight();

    const qreal pad = 1;
    const qreal px = pad / tx;
    const qreal py = pad / ty;
    const qreal x0 = topLeft.x() + px;
    const qreal y0 = topLeft.y() + py;
    const qreal x1 = botRight.x() - px;
    const qreal y1 = botRight.y() - py;

    QPainterPath path;
    path.moveTo(x0, y0);
    path.lineTo(x0, y1);
    path.lineTo(x1, y1);
    path.lineTo(x1 - sx, y1 - sy);
    path.lineTo(x0 + sx, y1 - sy);
    path.lineTo(x0 + sx, y0 + sy);
    path.moveTo(x0, y0);
    shadowItem1_->setPath(path);

    path = QPainterPath();
    path.moveTo(x0, y0);
    path.lineTo(x1, y0);
    path.lineTo(x1, y1);
    path.lineTo(x1 - sx, y1 - sy);
    path.lineTo(x1 - sx, y0 + sy);
    path.lineTo(x0 + sx, y0 + sy);
    path.moveTo(x0, y0);
    shadowItem2_->setPath(path);
}
