#include "item.h"

static QDomElement createExtDataDataElement(QDomDocument &doc, const QString &name, const QString &value)
{
  QDomElement valueElem = doc.createElement("value");
  valueElem.appendChild(doc.createTextNode(value));
  QDomElement dataElem = doc.createElement("Data");
  dataElem.setAttribute("name", name);
  dataElem.appendChild(valueElem);
  return dataElem;
}

// Returns a new <ExtendedData> element.
QDomElement Item::createExtDataElement(QDomDocument &doc) const
{
  QDomElement extDataElem = doc.createElement("ExtendedData");
  extDataElem.appendChild(createExtDataDataElement(doc, "met:objectType", (points.size() == 1) ? "Symbol" : "PolyLine")); // for now
  return extDataElem;
}

// Returns a new <Point> or <Polygon> element (depending on whether points_.size() == 1 or > 1 respectively).
QDomElement Item::createPointOrPolygonElement(QDomDocument &doc) const
{
  // create the <coordinates> element
  QString coords;
  foreach (QPointF point, points)
    coords.append(QString("%1,%2,0\n").arg(point.y()).arg(point.x())); // note lon,lat order
  QDomElement coordsElem = doc.createElement("coordinates");
  coordsElem.appendChild(doc.createTextNode(coords));

  // create a <Point> or <Polygon> element
  QDomElement popElem;
  if (points.size() == 1) {
    popElem = doc.createElement("Point");
    popElem.appendChild(coordsElem);
  } else {
    QDomElement linearRingElem = doc.createElement("LinearRing");
    linearRingElem.appendChild(coordsElem);
    QDomElement outerBoundaryIsElem = doc.createElement("outerBoundaryIs");
    outerBoundaryIsElem.appendChild(linearRingElem);
    QDomElement tessellateElem = doc.createElement("tessellate");
    tessellateElem.appendChild(doc.createTextNode("1"));
    popElem = doc.createElement("Polygon");
    popElem.appendChild(tessellateElem);
    popElem.appendChild(outerBoundaryIsElem);
  }

  return popElem;
}

// Returns a new <TimeSpan> element.
QDomElement Item::createTimeSpanElement(QDomDocument &doc) const
{
  QDomElement beginTimeElem = doc.createElement("begin");
  beginTimeElem.appendChild(doc.createTextNode(properties.value("TimeSpan:begin").toString()));
  QDomElement endTimeElem = doc.createElement("end");
  endTimeElem.appendChild(doc.createTextNode(properties.value("TimeSpan:end").toString()));
  QDomElement timeSpanElem = doc.createElement("TimeSpan");
  timeSpanElem.appendChild(beginTimeElem);
  timeSpanElem.appendChild(endTimeElem);
  return timeSpanElem;
}

// Returns a new <Placemark> element.
QDomElement Item::createPlacemarkElement(QDomDocument &doc) const
{
  QDomElement nameElem = doc.createElement("name");
  const QString name = properties.contains("Placemark:name")
      ? properties.value("Placemark:name").toString()
      : QString("anonymous placemark");
  nameElem.appendChild(doc.createTextNode(name));
  QDomElement placemarkElem = doc.createElement("Placemark");
  placemarkElem.appendChild(nameElem);
  return placemarkElem;
}

QDomNode Item::toKML() const
{
  QDomDocument doc;
  QDomElement extDataElem = createExtDataElement(doc);
  QDomElement popElem = createPointOrPolygonElement(doc);
  QDomElement finalElem;

  if (properties.contains("Folder:name")) {
    QDomElement nameElem = doc.createElement("name");
    nameElem.appendChild(doc.createTextNode(properties.value("Folder:name").toString()));
    QDomElement timeSpanElem = createTimeSpanElement(doc);
    QDomElement placemarkElem = createPlacemarkElement(doc);
    placemarkElem.appendChild(extDataElem);
    placemarkElem.appendChild(popElem);
    QDomElement folderElem = doc.createElement("Folder");
    folderElem.appendChild(nameElem);
    folderElem.appendChild(timeSpanElem);
    folderElem.appendChild(placemarkElem);
    finalElem = folderElem;
  } else {
    QDomElement placemarkElem = createPlacemarkElement(doc);
    placemarkElem.appendChild(extDataElem);
    placemarkElem.appendChild(popElem);
    finalElem = placemarkElem;
  }

  return finalElem;
}
