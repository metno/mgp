#include "lanebgitem.h"
#include <QBrush>
#include <QColor>

LaneBGItem::LaneBGItem(qint64 roleId__)
    : roleId_(roleId__)
{
    setBrush(QBrush(QColor(128 + qrand() % 128, 128 + qrand() % 128, 128 + qrand() % 128)));
    setFlag(QGraphicsItem::ItemIsMovable);
}
