#include "leftheaderbgitem.h"
#include <QBrush>
#include <QColor>

LeftHeaderBGItem::LeftHeaderBGItem(qint64 roleId__)
    : roleId_(roleId__)
{
    setBrush(QBrush(QColor(128 + qrand() % 128, 128 + qrand() % 128, 128 + qrand() % 128)));
    setFlag(QGraphicsItem::ItemIsMovable);
}
