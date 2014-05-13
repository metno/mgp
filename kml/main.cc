#include <QCoreApplication>
#include <QList>
#include <QDir>
#include <QString>
#include <QDebug>
#include <iostream>
#include "item.h"
#include "kml.h"

void printError(const QString &error)
{
  std::cerr << error.toLatin1().data() << std::endl;
}

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  if ((argc < 2) || (argc > 3)) {
    printError(QString("usage: %1 <KML input file> [<KML output file>]").arg(argv[0]));
    return 1;
  }

  QString error;

  // read items from KML file
  QList<Item> items = KML::readFromFile(argv[1], &error);
  if (!error.isEmpty()) {
    printError(error);
    return 1;
  }

  // print items
  int i = 0;
  foreach (Item item, items) {
    qDebug() << "\nitem" << i++ << ":";
    qDebug() << "    points:" << item.points;
    qDebug() << "    properties:";
    foreach (QString key, item.properties.keys())
      qDebug() << "        " << key << ":" << item.properties.value(key);
  }

  // write items to KML file
  if (argc == 3) {
    if (QDir::cleanPath(QDir().absoluteFilePath(argv[1])) == QDir::cleanPath(QDir().absoluteFilePath(argv[2]))) {
      printError("input file == output file -> writing skipped");
      return 1;
    }

    KML::writeToFile(items, argv[2], &error);
    if (!error.isEmpty()) {
      printError(error);
      return 1;
    }
  }

  return 0;
}
