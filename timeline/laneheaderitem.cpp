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

    filterItem_ = new QGraphicsTextItem("<filter>", this);
    filterItem_->setFont(QFont("helvetica", 10, QFont::Normal));
    filterItem_->setHtml("<div style='color:ff0000; background-color:#ffff00;'>&lt;filter&gt;</div>");
    //filterItem_->setDefaultTextColor(Qt::white);
    filterItem_->setZValue(2);
    filterItem_->setFlag(QGraphicsItem::ItemIgnoresTransformations);

    updateProperties();
}

void LaneHeaderItem::updateRect(const QRectF &r)
{
    setRect(r);
    nameItem_->setPos(rect().x() + 5, rect().y());
    const qreal voffset = nameItem_->boundingRect().height() / scene()->views().first()->transform().m22();
    timeItem_->setPos(rect().x() + 5, rect().y() + voffset);
    filterItem_->setPos(
                rect().x() + rect().width() - filterItem_->boundingRect().width() / scene()->views().first()->transform().m11(),
                rect().y() + voffset);
}

void LaneHeaderItem::updateProperties()
{
    QSharedPointer<Role> role = TaskManager::instance().findRole(roleId_);
    nameItem_->setPlainText(role->name());
    timeItem_->setPlainText(QString("%1-%2").arg(role->loTime().toString("hh:mm")).arg(role->hiTime().toString("hh:mm")));
}
