#include "laneheaderbgitem.h"
#include <QBrush>
#include <QColor>

LaneHeaderBGItem::LaneHeaderBGItem(qint64 roleId__)
    : roleId_(roleId__)
{
    setBrush(QBrush(QColor(128 + qrand() % 128, 128 + qrand() % 128, 128 + qrand() % 128)));
    setFlag(QGraphicsItem::ItemIsMovable);
}
