#ifndef KML_H
#define KML_H

#include <QList>
#include <QString>
#include <QByteArray>
#include "item.h"

namespace KML {
QList<Item> kml2items(const QByteArray &, QString *);
QList<Item> readFromFile(const QString &, QString *);
QByteArray items2kml(const QList<Item> &, QString *);
void writeToFile(const QList<Item> &, const QString &, QString *);
} // namespace KML

#endif // KML_H
