#include "leftheaderbgitem.h"
#include "taskmanager.h"
#include "role.h"
#include "common.h"
#include <QBrush>
#include <QColor>
#include <QGraphicsTextItem>
#include <QFont>

LeftHeaderBGItem::LeftHeaderBGItem(qint64 roleId__)
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
}

void LeftHeaderBGItem::updateRect(const QRectF &r)
{
    setRect(r);
    nameItem_->setPos(rect().x() + 5, rect().y());
}
