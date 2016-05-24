#include "kml.h"
#include <QVector>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QDomDocument>
#include <QDomElement>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
#include <QAbstractMessageHandler>
#include <QPointF>

KML_BEGIN_NAMESPACE

#if 0
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
// Upon success, the function returns true and a valid schema in \a schema.
// Otherwise, the function returns false and the failure reason in \a error.
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
#endif

// Creates a DOM document from the KML structure in \a data, validating against \a schema.
// Returns a non-null document upon success, or a null document and a failure reason in \a error upon failure.
QDomDocument createDomDocument(const QByteArray &data, const QXmlSchema &schema, const QUrl &docUri, QString *error)
{
#if 0
    Q_ASSERT(schema.isValid());
#endif
    *error = QString();

#if 0 // disabled for now
    MessageHandler msgHandler;

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

#if 0
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
#endif

// Returns the polygon in a <cooordinates> element. Leaves \a error empty iff no errors occurs.
mgp::Polygon getPolygon(const QDomNode &coordsNode, QString *error)
{
    const mgp::Polygon emptyPolygon = mgp::Polygon(new QVector<mgp::Point>());

    const QString coords = coordsNode.firstChild().nodeValue();
    mgp::Polygon polygon = mgp::Polygon(new QVector<mgp::Point>());

    foreach (QString coord, coords.split(QRegExp("\\s+"), QString::SkipEmptyParts)) {
        const QStringList coordComps = coord.split(",", QString::SkipEmptyParts);
        if (coordComps.size() < 2) {
            *error = QString("expected at least two components (i.e. lon, lat) in coordinate, found %1: %2")
                    .arg(coordComps.size()).arg(coord);
            return emptyPolygon;
        }
        bool ok;
        const double lon = coordComps.at(0).toDouble(&ok);
        if (!ok) {
            *error = QString("failed to convert longitude string to double value: %1").arg(coordComps.at(0));
            return emptyPolygon;
        }
        const double lat = coordComps.at(1).toDouble(&ok);
        if (!ok) {
            *error = QString("failed to convert latitude string to double value: %1").arg(coordComps.at(1));
            return emptyPolygon;
        }
        polygon->append(qMakePair(DEG2RAD(lon), DEG2RAD(lat)));
    }

    return polygon;
}

// Returns the polygons found in DOM document \a doc.
// Upon success, the function returns a non-empty list of polygons and leaves \a error empty.
// Upon failure, the function returns an empty list of polygons and a failure reason in \a error.
mgp::Polygons createFromDomDocument(const QDomDocument &doc, QString *error)
{
    *error = QString();
    const mgp::Polygons emptyPolygons = mgp::Polygons(new QVector<mgp::Polygon>);

    mgp::Polygons polygons = mgp::Polygons(new QVector<mgp::Polygon>);

    // loop over <coordinates> elements
    QDomNodeList coordsNodes = doc.elementsByTagName("coordinates");
    for (int i = 0; i < coordsNodes.size(); ++i) {
        // get polygon
        const QDomNode coordsNode = coordsNodes.item(i);
        mgp::Polygon polygon = getPolygon(coordsNode, error);
        if (!error->isEmpty())
            return emptyPolygons;
        polygons->append(polygon);
#if 0
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

        items.append(item);
#endif
    }

    return polygons;
}

// Returns the polygons found in a KML structure. Sets \a error to a non-empty string iff the operation fails.
mgp::Polygons kml2polygons(const QByteArray &kml, const QUrl &docUri, QString *error)
{
    const mgp::Polygons emptyPolygons = mgp::Polygons(new QVector<mgp::Polygon>);

    // load schema
    QXmlSchema schema;
#if 0 // disabled for now
    if (!loadSchema(schema, error)) {
        *error = QString("failed to load KML schema: %1").arg(*error);
        return emptyPolygons;
    }
#endif

    // create document from data validated against schema
    QDomDocument doc = createDomDocument(kml, schema, docUri, error);
    if (doc.isNull()) {
        *error = QString("failed to create DOM document: %1").arg(*error);
        return emptyPolygons;
    }

    // parse document and create items
    return createFromDomDocument(doc, error);
}

// Returns the polygons found in a KML structure. Sets \a error to a non-empty string iff the operation fails.
mgp::Polygons kml2polygons(const QByteArray &kml, QString *error)
{
    return kml2polygons(kml, QUrl(), error);
}

KML_END_NAMESPACE
