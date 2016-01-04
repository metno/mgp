#include "laneheaderitem.h"
#include "taskmanager.h"
#include "role.h"
#include "common.h"
#include <QBrush>
#include <QLinearGradient>
#include <QColor>
#include <QGraphicsTextItem>
#include <QFont>
#include <QGraphicsScene>
#include <QGraphicsView>

LaneHeaderItem::LaneHeaderItem(qint64 roleId__)
    : roleId_(roleId__)
{
    setZValue(1);
    QLinearGradient grad;
    grad.setCoordinateMode(QGradient::ObjectBoundingMode);
    grad.setStart(0, 0.5);
    grad.setFinalStop(1, 0.5);
    grad.setColorAt(0, QColor("#ddd"));
    grad.setColorAt(1, QColor("#aaa"));
    setBrush(QBrush(grad));

    QSharedPointer<Role> role = TaskManager::instance().findRole(roleId_);
    Q_ASSERT(role);

    nameItem_ = new QGraphicsTextItem("<no name>", this);
    nameItem_->setFont(QFont("helvetica", 14, QFont::Normal));
    nameItem_->setZValue(2);
    nameItem_->setFlag(QGraphicsItem::ItemIgnoresTransformations);

    timeItem_ = new QGraphicsTextItem("<no time>", this);
    timeItem_->setFont(QFont("helvetica", 14, QFont::Normal));
    timeItem_->setDefaultTextColor(Qt::blue);
    timeItem_->setZValue(2);
    timeItem_->setFlag(QGraphicsItem::ItemIgnoresTransformations);

    updateProperties();
}

void LaneHeaderItem::updateRect(const QRectF &r)
{
    setRect(r);
    nameItem_->setPos(rect().x() + 5, rect().y());
    timeItem_->setPos(
                rect().x() + 5,
                rect().y() + nameItem_->boundingRect().height() / scene()->views().first()->transform().m22());
}

void LaneHeaderItem::updateProperties()
{
    QSharedPointer<Role> role = TaskManager::instance().findRole(roleId_);
    nameItem_->setPlainText(role->name());
    timeItem_->setPlainText(QString("%1-%2").arg(role->loTime().toString("hh:mm")).arg(role->hiTime().toString("hh:mm")));
}
