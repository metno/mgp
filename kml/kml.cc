#include <QList>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QDomElement>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QAbstractMessageHandler>
#include <QPointF>
#include <QDebug>
#include <iostream>
#include "item.h"

namespace KML {

class MessageHandler : public QAbstractMessageHandler
{
  virtual void handleMessage(QtMsgType, const QString &, const QUrl &, const QSourceLocation &);
  int n_;
  QtMsgType type_;
  QString descr_;
  QUrl id_;
  QSourceLocation srcLoc_;
public:
  MessageHandler();
  void reset();
  QString lastMessage() const;
};

void MessageHandler::handleMessage(QtMsgType type, const QString &descr, const QUrl &id, const QSourceLocation &srcLoc)
{
  n_++;
  type_ = type;
  descr_ = descr;
  id_ = id;
  srcLoc_ = srcLoc;
}

MessageHandler::MessageHandler() : n_(0) {}

void MessageHandler::reset() { n_ = 0; }

QString MessageHandler::lastMessage() const
{
  if (n_ == 0)
    return "<no message>";
  return QString("%1 (line %2, column %3)").arg(descr_).arg(srcLoc_.line()).arg(srcLoc_.column());
}

// Attempts to load \a fileName as a valid XML schema.
// Returns true upon success, or false and a reason in \a error upon failure.
static bool loadSchemaFromFile(QXmlSchema &schema, const QString &fileName, QString *error)
{
  *error = QString();

  QUrl schemaUrl(QUrl(QString("file://%1").arg(fileName)));
  if (!schemaUrl.isValid()) {
    *error = QString("invalid schema: %1, reason: %2").arg(schemaUrl.path()).arg(schemaUrl.errorString());
    return false;
  }

  MessageHandler msgHandler;
  schema.setMessageHandler(&msgHandler);
  msgHandler.reset();
  if (!schema.load(schemaUrl)) {
    Q_ASSERT(!schema.isValid());
    *error = QString("failed to load schema: %1, reason: %2").arg(schemaUrl.path()).arg(msgHandler.lastMessage());
    return false;
  }

  Q_ASSERT(schema.isValid());
  return true;
}

// Loads the KML schema from pre-defined candidate files in prioritized order.
// Upon success, the function returns true and a valid schema in \a schema.
// Otherwise, the function returns false and the failure reason for each candidate file in \a error.
bool loadSchema(QXmlSchema &schema, QString *error)
{
  *error = QString();

  // open the schema file from a list of candidates in prioritized order
  const QString schemaBaseFileName("ogckml22.xsd");
  QStringList candSchemaFileNames;
  candSchemaFileNames.append(QString("%1").arg(schemaBaseFileName));
  candSchemaFileNames.append(QString("%1/%2").arg(qgetenv("KMLSCHEMADIR").constData()).arg(schemaBaseFileName));
  QStringList candSchemaErrors;
  int i = 0;
  foreach (const QString schemaFileName, candSchemaFileNames) {
    i++;
    QString schemaLoadError;
    if (loadSchemaFromFile(schema, schemaFileName, &schemaLoadError)) {
      Q_ASSERT(schema.isValid());
      Q_ASSERT(schemaLoadError.isEmpty());
      break;
    }
    Q_ASSERT(!schema.isValid());
    Q_ASSERT(!schemaLoadError.isEmpty());
    candSchemaErrors.append(QString("error opening schema from candidate file %1:%2 (%3): %4")
                            .arg(i).arg(candSchemaFileNames.size()).arg(schemaFileName).arg(schemaLoadError));
  }

  if (!schema.isValid()) {
    *error = QString(candSchemaErrors.join(", "));
    return false;
  }

  return true;
}

// Creates a DOM document from the KML structure in \a data, validating against \a schema.
// Returns a non-null document upon success, or a null document and a failure reason in \a error upon failure.
QDomDocument createDomDocument(const QByteArray &data, const QXmlSchema &schema, const QUrl &docUri, QString *error)
{
  Q_ASSERT(schema.isValid());
  *error = QString();
  MessageHandler msgHandler;

#if 0 // disabled for now
  // validate against schema
  QXmlSchemaValidator validator(schema);
  validator.setMessageHandler(&msgHandler);
  msgHandler.reset();
  if (!validator.validate(data, docUri)) {
    *error = QString("failed to validate against schema: %1, reason: %2").arg(schema.documentUri().path()).arg(msgHandler.lastMessage());
    return QDomDocument();
  }
#else
  Q_UNUSED(schema);
  Q_UNUSED(docUri);
#endif

  // create DOM document
  int line;
  int col;
  QString err;
  QDomDocument doc;
  if (doc.setContent(data, &err, &line, &col) == false) {
    *error = QString("parse error at line %1, column %2: %3").arg(line).arg(col).arg(err);
    return QDomDocument();
  }

  Q_ASSERT(!doc.isNull());
  return doc;
}

// Finalizes KML document \a doc and returns a textual representation of it.
static QByteArray createKMLText(QDomDocument &doc, const QDomDocumentFragment &innerStruct)
{
  // add <kml> root element
  QDomElement kmlElem = doc.createElement("kml");
  kmlElem.setAttribute("xmlns", "http://www.opengis.net/kml/2.2"); // required to match schema
  doc.appendChild(kmlElem);

  // add <Document> element
  QDomElement docElem = doc.createElement("Document");
  kmlElem.appendChild(docElem);

  // compress innerStruct if necessary (so represent identical <Folder> elements as one <Folder> element for items etc.) ... TBD

  // add inner structure
  docElem.appendChild(innerStruct);

  QByteArray kml;
  kml.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n"); // XML declaration
  kml.append(doc.toByteArray(2)); // DOM structure

  return kml;
}

// Fills \a ancElems in with all ancestor elements of \a node. Assumes (and checks for) uniqueness
// of ancestor tag names. Leaves \a error empty iff no error occurs.
void findAncestorElements(const QDomNode &node, QMap<QString, QDomElement> *ancElems, QString *error)
{
  ancElems->clear();
  *error = QString();
  if (node.isNull())
    return;

  for (QDomNode n = node.parentNode(); !n.isNull(); n = n.parentNode()) {
    const QDomElement elem = n.toElement();
    if (!elem.isNull())
      ancElems->insert(elem.tagName(), elem);
  }
}

// Returns the string contained in the <name> child of \a elem.
// Leaves \a error empty iff no error occurs.
QString getName(const QDomElement &elem, QString *error)
{
  Q_ASSERT(!elem.isNull());
  *error = QString();
  const QDomElement nameElem = elem.firstChildElement("name");
  if (nameElem.isNull()) {
    *error = QString("element <%1> contains no <name> child").arg(elem.tagName());
    return "";
  }
  return nameElem.firstChild().nodeValue();
}

// Returns any extended data map located as a child of the nearest node
// with tag \a parentTag along the ancestor chain from (and including) \a node.
QHash<QString, QString> getExtendedData(const QDomNode &node, const QString &parentTag)
{
  QHash<QString, QString> extData;

  for (QDomNode n = node; !n.isNull(); n = n.parentNode()) {
    const QDomElement e = n.toElement();
    if ((!e.isNull()) && (e.tagName() == parentTag)) {
      const QDomElement extDataElem = n.firstChildElement("ExtendedData");
      if (!extDataElem.isNull()) {
        const QDomNodeList dataNodes = extDataElem.elementsByTagName("Data");
        for (int i = 0; i < dataNodes.size(); ++i) {
          const QDomElement dataElem = dataNodes.item(i).toElement();
          const QDomNodeList valueNodes = dataElem.elementsByTagName("value");
          for (int j = 0; j < valueNodes.size(); ++j) {
            const QDomElement valueElem = valueNodes.item(j).toElement();
            const QString value = valueElem.firstChild().nodeValue();
            extData[dataElem.attribute("name").trimmed()] = value;
          }
        }
      }
      break; // at nearest matching parent node, so stop searching
    }
  }

  return extData;
}

// Returns the sequence of (lat, lon) points of a <cooordinates> element.
// Leaves \a error empty iff no errors occurs.
QList<QPointF> getPoints(const QDomNode &coordsNode, QString *error)
{
  const QString coords = coordsNode.firstChild().nodeValue();
  QList<QPointF> points;

  foreach (QString coord, coords.split(QRegExp("\\s+"), QString::SkipEmptyParts)) {
    const QStringList coordComps = coord.split(",", QString::SkipEmptyParts);
    if (coordComps.size() < 2) {
      *error = QString("expected at least two components (i.e. lat, lon) in coordinate, found %1: %2")
          .arg(coordComps.size()).arg(coord);
      return QList<QPointF>();
    }
    bool ok;
    const double lon = coordComps.at(0).toDouble(&ok);
    if (!ok) {
      *error = QString("failed to convert longitude string to double value: %1").arg(coordComps.at(0));
      return QList<QPointF>();
    }
    const double lat = coordComps.at(1).toDouble(&ok);
    if (!ok) {
      *error = QString("failed to convert latitude string to double value: %1").arg(coordComps.at(1));
      return QList<QPointF>();
    }
    points.append(QPointF(lat, lon)); // note lat,lon order
  }

  return points;
}

// Returns the pair of strings that represents begin- and end time contained in the <TimeSpan> child of \a elem.
// Leaves \a error empty iff no error occurs (such as when \a elem doesn't have a <TimeSpan> child!).
QPair<QString, QString> getTimeSpan(const QDomElement &elem, QString *error)
{
  Q_ASSERT(!elem.isNull());
  *error = QString();

  const QDomElement timeSpanElem = elem.firstChildElement("TimeSpan");
  if (timeSpanElem.isNull()) {
    *error = QString("element <%1> contains no <TimeSpan> child").arg(elem.tagName());
    return QPair<QString, QString>();
  }

  const QDomElement beginTimeElem = timeSpanElem.firstChildElement("begin");
  if (beginTimeElem.isNull()) {
    *error = "time span contains no begin time";
    return QPair<QString, QString>();
  }
  //
  const QDomElement endTimeElem = timeSpanElem.firstChildElement("end");
  if (endTimeElem.isNull()) {
    *error = "time span contains no end time";
    return QPair<QString, QString>();
  }

  return qMakePair(beginTimeElem.firstChild().nodeValue(), endTimeElem.firstChild().nodeValue());
}

// Returns a list of items extracted from DOM document \a doc.
// Upon success, the function returns a non-empty list of items and leaves \a error empty.
// Upon failure, the function returns an empty list of items and a failure reason in \a error.
QList<Item> createFromDomDocument(const QDomDocument &doc, QString *error)
{
  *error = QString();

  QList<Item> items;

  // loop over <coordinates> elements
  QDomNodeList coordsNodes = doc.elementsByTagName("coordinates");
  for (int i = 0; i < coordsNodes.size(); ++i) {

    Item item;

    // get lat,lon point(s)
    const QDomNode coordsNode = coordsNodes.item(i);
    if (!error->isEmpty())
      return QList<Item>();
    const QList<QPointF> points = getPoints(coordsNode, error);
    if (!error->isEmpty())
      return QList<Item>();
    item.points = points;

    // get type
    const QHash<QString, QString> pmExtData = getExtendedData(coordsNode, "Placemark");
    if (pmExtData.isEmpty()) {
      *error = "<Placemark> element without <ExtendedData> element found";
      return QList<Item>();
    }
    item.properties.insert("type", pmExtData.value("met:objectType"));

    QMap<QString, QDomElement> ancElems;
    findAncestorElements(coordsNode, &ancElems, error);
    if (!error->isEmpty())
      return QList<Item>();

    // get placemark (the placemark typically represents the volcano itself at a single lat,lon position or a polygon at a flight level
    // (the placemark name is then the name of the volcano or the flight level respectively))
    if (ancElems.contains("Placemark")) {
      item.properties.insert("Placemark:name", getName(ancElems.value("Placemark"), error));
      if (!error->isEmpty())
        return QList<Item>();
    } else {
      *error = "found <coordinates> element outside a <Placemark> element";
      return QList<Item>();
    }

    // get folder (the folder typically represents a time range and may contain one or more placemarks (but typically not the placemark
    // for the volcano itself))
    if (ancElems.contains("Folder")) {
      item.properties.insert("Folder:name", getName(ancElems.value("Folder"), error));
      if (!error->isEmpty())
        return QList<Item>();

      QPair<QString, QString> timeSpan = getTimeSpan(ancElems.value("Folder"), error);
      if (!error->isEmpty())
        return QList<Item>();
      item.properties.insert("TimeSpan:begin", timeSpan.first);
      item.properties.insert("TimeSpan:end", timeSpan.second);
    }

    items.append(item);
  }

  return items;
}

// Returns a KML structure as a list of items. Sets \a error to a non-empty string iff the operation fails.
QList<Item> kml2items(const QByteArray &kml, const QUrl &docUri, QString *error)
{
  // load schema
  QXmlSchema schema;
#if 0 // disabled for now
  if (!loadSchema(schema, error)) {
    *error = QString("failed to load KML schema: %1").arg(*error);
    return QList<Item>();
  }
#endif

  // create document from data validated against schema
  QDomDocument doc = createDomDocument(kml, schema, docUri, error);
  if (doc.isNull()) {
    *error = QString("failed to create DOM document: %1").arg(*error);
    return QList<Item>();
  }

  // parse document and create items
  return createFromDomDocument(doc, error);
}

// Returns a KML structure as a list of items. Sets \a error to a non-empty string iff the operation fails.
QList<Item> kml2items(const QByteArray &kml, QString *error)
{
  return kml2items(kml, QUrl(), error);
}

// Returns a list of items read from a KML file. Sets \a error to a non-empty string iff the operation fails.
QList<Item> readFromFile(const QString &fileName, QString *error)
{
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    *error = QString("failed to open file %1 for reading").arg(fileName);
    return QList<Item>();
  }

  return kml2items(file.readAll(), QUrl::fromLocalFile(fileName), error);
}

// Returns a list of items as a KML structure. Sets \a error to a non-empty string iff the operation fails.
QByteArray items2kml(const QList<Item> &items, QString *error)
{
  *error = QString();
  QDomDocument doc;

  QDomDocumentFragment innerStruct = doc.createDocumentFragment();

  // insert items
  foreach (Item item, items)
    innerStruct.appendChild(item.toKML());

  return createKMLText(doc, innerStruct);
}

// Writes a list of items to a KML file. Sets \a error to a non-empty string iff the operation fails.
void writeToFile(const QList<Item> &items, const QString &fileName, QString *error)
{
  *error = QString();

  const QByteArray kml = items2kml(items, error);
  if (!error->isEmpty())
    return;

  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      *error = QString("failed to open %1 for writing").arg(fileName);
      return;
  }

  if (file.write(kml) == -1) {
    *error = QString("failed to write to %1: %2").arg(fileName).arg(file.errorString());
    return;
  }
}

} // namespace KML
