#include "leftheaderlaneitem.h"
#include "taskmanager.h"
#include "role.h"
#include "common.h"
#include <QBrush>
#include <QColor>
#include <QGraphicsTextItem>
#include <QFont>
#include <QGraphicsScene>
#include <QGraphicsView>

LeftHeaderLaneItem::LeftHeaderLaneItem(qint64 roleId__)
    : roleId_(roleId__)
{
    setZValue(1);
    setBrush(QBrush(QColor(192 + qrand() % 64, 192 + qrand() % 64, 192 + qrand() % 64)));

    QSharedPointer<Role> role = TaskManager::instance()->findRole(roleId_);

    const QString name = role ? role->name() : "<no name>";
    nameItem_ = new QGraphicsTextItem(name, this);
    nameItem_->setFont(QFont("helvetica", 14, QFont::Normal));
    nameItem_->setZValue(2);
    nameItem_->setFlag(QGraphicsItem::ItemIgnoresTransformations);

    const QString time = role ? QString("%1-%2").arg(role->beginTime().toString("hh:mm")).arg(role->endTime().toString("hh:mm")) : "<no time";
    timeItem_ = new QGraphicsTextItem(time, this);
    timeItem_->setFont(QFont("helvetica", 14, QFont::Normal));
    timeItem_->setDefaultTextColor(Qt::blue);
    timeItem_->setZValue(2);
    timeItem_->setFlag(QGraphicsItem::ItemIgnoresTransformations);
}

void LeftHeaderLaneItem::updateRect(const QRectF &r)
{
    setRect(r);
    nameItem_->setPos(rect().x() + 5, rect().y());
    timeItem_->setPos(
                rect().x() + 5,
                rect().y() + nameItem_->boundingRect().height() / scene()->views().first()->transform().m22());
}
