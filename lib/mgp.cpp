#include "mgp.h"
#include "mgpmath.h"
#include "enor_fir.h"
#include "enob_fir.h"
#include "polygonintersector.h"
#include <QBitArray>
#include <QRegExp>
#include <QStack>
#include <algorithm>

#include <QDebug>

MGP_BEGIN_NAMESPACE

// Returns the SIGMET/AIRMET degrees form of a longitude value in radians ([-M_PI, M_PI]).
// Examples:
//        0 -> E00000)
//    -M_PI -> W18000
//   M_PI_2 -> E09000
static QString xmetFormatLon(double val)
{
    double lon = RAD2DEG(fmod(val, 2 * M_PI));
    if (lon > 180)
        lon -= 360;
    lon = qMin(180.0, qMax(-180.0, lon));
    int ipart = int(floor(fabs(lon)));
    //const int fbase = 100;
    const int fbase = 60; // minutes
    int fpart = int(floor(round(fbase * (fabs(lon) - ipart))));
    if (fpart >= fbase) {
        ipart++;
        fpart -= fbase;
    }
    Q_ASSERT((fpart >= 0) && (fpart < fbase));
    return QString("%1%2%3")
            .arg((lon < 0) ? "W" : "E")
            .arg(ipart, 3, 10, QLatin1Char('0'))
            .arg(QString("%1").arg(fpart, 2, 10, QLatin1Char('0')));
}

// Returns the SIGMET/AIRMET degrees form of a latitude value in radians ([-M_PI_2, M_PI_2]).
// Examples:
//         0 -> N0000
//   -M_PI_2 -> S9000
//    M_PI_4 -> N4500
static QString xmetFormatLat(double val)
{
    double lat = RAD2DEG(fmod(val, M_PI));
    lat = qMin(qMax(lat, -90.0), 90.0);
    int ipart = int(floor(fabs(lat)));
    //const int fbase = 100;
    const int fbase = 60; // minutes
    int fpart = int(floor(round(fbase * (fabs(lat) - ipart))));
    if (fpart >= fbase) {
        ipart++;
        fpart -= fbase;
    }
    Q_ASSERT((fpart >= 0) && (fpart < fbase));
    return QString("%1%2%3")
            .arg(lat < 0 ? "S" : "N")
            .arg(ipart, 2, 10, QLatin1Char('0'))
            .arg(QString("%1").arg(fpart, 2, 10, QLatin1Char('0')));
}

// Returns the longitude value in radians ([-M_PI, M_PI]) corresponding to a SIGMET/AIRMET longitude degrees expression.
// Examples:
//   E00000 -> 0
//   W18000 -> -M_PI
//   E09000 ->  M_PI_2
static double xmetExtractLon(const QString &s, bool &success)
{
    Q_ASSERT(s.size() == 6);
    Q_ASSERT((s[0].toLower() == 'e') || (s[0].toLower() == 'w'));
    bool ok;
    int ipart = s.mid(1, 3).toInt(&ok) % 360;
    Q_ASSERT(ok);
    if (ipart > 180)
        ipart = qAbs(ipart - 360);
    const int fpart = s.mid(4, 2).toInt(&ok);
    Q_ASSERT(ok);
    //const int fbase = 100;
    const int fbase = 60; // minutes
    if (!((fpart >= 0) && (fpart <= (fbase - 1)))) {
        success = false;
        return 0;
    }
    const double lon = DEG2RAD(ipart + fpart / double(fbase)) * ((s[0].toLower() == 'e') ? 1 : -1);
    success = true;
    return lon;
}

// Returns the latitude value in radians ([-M_PI_2, M_PI_2]) corresponding to a SIGMET/AIRMET latitude degrees expression.
// Examples:
//    N0000 -> 0
//    S0000 -> 0
//    S9000 -> -M_PI_2
//    N4500 ->  M_PI_4
static double xmetExtractLat(const QString &s, bool &success)
{
    Q_ASSERT(s.size() == 5);
    Q_ASSERT((s[0].toLower() == 'n') || (s[0].toLower() == 's'));
    bool ok;
    int ipart = s.mid(1, 2).toInt(&ok);
    Q_ASSERT(ok);
    Q_ASSERT(ipart >= 0);
    ipart = qMin(ipart, 90);
    const int fpart = s.mid(3, 2).toInt(&ok);
    Q_ASSERT(ok);
    //const int fbase = 100;
    const int fbase = 60; // minutes
    if (!((fpart >= 0) && (fpart <= (fbase - 1)))) {
        success = false;
        return 0;
    }
    const double lat = DEG2RAD(ipart + fpart / double(fbase)) * ((s[0].toLower() == 'n') ? 1 : -1);
    success = true;
    return lat;
}

void PolygonFilter::setPolygon(const Polygon &polygon)
{
    polygon_ = polygon;
}

Polygon PolygonFilter::polygon() const
{
    return polygon_;
}

PolygonFilter::PolygonFilter()
{
}

PolygonFilter::PolygonFilter(const Polygon &polygon)
    : polygon_(polygon)
{
}

void PolygonFilter::setFromVariant(const QVariant &var)
{
    polygon_->clear();
    const QVariantList list = var.toList();
    Q_ASSERT(!(list.size() % 2));
    for (int i = 0; i < list.size(); i += 2) {
        bool ok1 = false;
        bool ok2 = false;
        const double lon = list.at(i).toDouble(&ok1);
        const double lat = list.at(i + 1).toDouble(&ok2);
        Q_ASSERT(ok1 && ok2);
        polygon_->append(qMakePair(lon, lat));
    }
}

QVariant PolygonFilter::toVariant() const
{
    QVariantList list;
    for (int i = 0; i < polygon_->size(); ++i) {
        list.append(polygon_->at(i).first);
        list.append(polygon_->at(i).second);
    }
    return list;
}

bool PolygonFilter::isValid() const
{
    if (!polygon_)
        return false;

    if (polygon_->size() < 3)
        return false;

#if 0 // THE CODE BELOW CURRENTLY DOESN'T WORK, PROBABLY BECAUSE NEIGHBOUR EDGES ARE SOMETIMES CONSIDERED
      // AS INTERSECTING EVEN IF THEY ONLY TOUCH AT ENDPOINTS
    // check if polygon is self-intersecting by comparing each unique pair of edges
    for (int i = 0; i < (polygon_->size() - 1); ++i) {
        for (int j = i + 1; j < polygon_->size(); ++j) {
            if (Math::greatCircleArcsIntersect(
                        polygon_->at(i), polygon_->at(i + 1),
                        polygon_->at(j), polygon_->at((j + 1) % polygon_->size())))
                return false;
        }
    }
#endif

    return true;
}

WithinFilter::WithinFilter()
{
}

WithinFilter::WithinFilter(const Polygon &polygon)
    : PolygonFilter(polygon)
{
}

Polygons WithinFilter::apply(const Polygon &inPoly) const
{
    return math::polygonIntersection(inPoly, polygon_);
}

QVector<Point> WithinFilter::intersections(const Polygon &inPoly) const
{
    QVector<Point> points;

    for (int i = 0; i < polygon_->size(); ++i) {
        Point isctPoint;
        for (int j = 0; j < inPoly->size(); ++j) {
            if (math::greatCircleArcsIntersect(
                        polygon_->at(i), polygon_->at((i + 1) % polygon_->size()),
                        inPoly->at(j), inPoly->at((j + 1) % inPoly->size()),
                        &isctPoint))
                points.append(isctPoint);
        }
    }

    return points;
}

bool WithinFilter::rejected(const Point &point) const
{
    return !math::pointInPolygon(point, polygon_);
}

bool WithinFilter::setFromXmetExpr(const QString &expr, QPair<int, int> *matchedRange, QPair<int, int> *incompleteRange, QString *incompleteReason)
{
    // get first "WI"
    const int firstPos = expr.indexOf("WI", 0, Qt::CaseInsensitive);
    if (firstPos < 0)
        return false; // not found at all
    if ((firstPos > 0) && (!expr[firstPos - 1].isSpace()))
        return false; // not at beginning or after space
    int lastPos = firstPos + 1;
    QString s = expr.mid(lastPos + 1); // move to first position after "WI"

    // get as many coordinates as possible after the "WI"
    Polygon polygon(new QVector<Point>());
    const QString basePattern(
                "([NS]\\d\\d\\d\\d)"
                "\\s*"
                "([EW]\\d\\d\\d\\d\\d)"
                );
    QRegExp rx;
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    bool first = true;
    while (true) {
        rx.setPattern(QString("%1%2").arg(first ? "^\\s+" : "^\\s+-\\s+").arg(basePattern));
        first = false;

        // read next coordinate
        const int rxpos = s.indexOf(rx);
        if (rxpos >= 0) { // match
            bool ok1 = false;
            bool ok2 = false;
            const double lon = xmetExtractLon(rx.cap(2), ok1);
            const double lat = xmetExtractLat(rx.cap(1), ok2);
            if (ok1 && ok2)
                polygon->append(qMakePair(lon, lat));
            if (!(ok1 && ok2))
                break;
            const int nextPos = rxpos + rx.matchedLength();
            lastPos += nextPos;
            s = s.mid(nextPos);
        } else { // no match
            break;
        }
    }

    if (polygon->size() >= 3) {
        polygon_ = polygon;
        matchedRange->first = firstPos;
        matchedRange->second = lastPos;
        return true; // success
    }

    // error
    incompleteRange->first = firstPos;
    incompleteRange->second = lastPos;
    *incompleteReason = QString("failed to extract at least three valid coordinates after 'WI'");
    return false; // no match
}

QString WithinFilter::xmetExpr() const
{
    QString s("WI");
    for (int i = 0; i < polygon_->size(); ++i)
        s += QString("%1 %2 %3")
                .arg((i == 0) ? "" : " -").arg(xmetFormatLat(polygon_->at(i).second)).arg(xmetFormatLon(polygon_->at(i).first));
    return s;
}

UnionFilter::UnionFilter()
{
    qFatal("UnionFilter is not yet supported");
}

UnionFilter::UnionFilter(const Polygon &polygon)
    : PolygonFilter(polygon)
{
    qFatal("UnionFilter is not yet supported");
}

Polygons UnionFilter::apply(const Polygon &) const
{
    return Polygons(); // ### TBD when filter is supported
}

QVector<Point> UnionFilter::intersections(const Polygon &) const
{
    return QVector<Point>(); // ### TBD when filter is supported
}

bool UnionFilter::rejected(const Point &) const
{
    return false;
}

bool UnionFilter::setFromXmetExpr(const QString &, QPair<int, int> *, QPair<int, int> *, QString *)
{
    return false; // ### TBD when filter is supported
}

QString UnionFilter::xmetExpr() const
{
    return QString(); // ### TBD when filter is supported
}

QString LineFilter::directionName() const
{
    switch (type()) {
    case E_OF:
    case E_OF_LINE:
        return "E";
    case W_OF:
    case W_OF_LINE:
        return "W";
    case N_OF:
    case N_OF_LINE:
        return "N";
    case S_OF:
    case S_OF_LINE:
        return "S";
    case NE_OF_LINE:
        return "NE";
    case NW_OF_LINE:
        return "NW";
    case SE_OF_LINE:
        return "SE";
    case SW_OF_LINE:
        return "SW";
    default:
        ;
    }
    return QString();
}

Polygons LineFilter::apply(const Polygon &inPoly) const
{
    Polygons outPolys = Polygons(new QVector<Polygon>());

    // get rejection status for all points
    const int n = inPoly->size();
    QBitArray rej(n);
    for (int i = 0; i < n; ++i)
        rej[i] = rejected(inPoly->at(i));

    if (rej.count(true) == n) {
        // all points rejected, so return an empty list
        return outPolys;
    } else if (rej.count(true) == 0) {
        // all points accepted, so return a list with one item: a deep copy (although implicitly shared for efficiency) of the input polygon
        Polygon inPolyCopy(new QVector<Point>(*inPoly.data()));
        outPolys->append(inPolyCopy);
        return outPolys;
    }

    // general case

    // find the first transition from rejected to accepted
    int first; // index of the first point in the first polygon
    for (first = 0; first < n; ++first)
        if (rej[first] && (!rej[(first + 1) % n]))
            break;
    int curr = first;

    while (true) {
        // start new polygon at intersection on (curr, curr + 1)
        const int next = (curr + 1) % n;
        Q_ASSERT(rej[curr] && (!rej[next]));
        Polygon poly(new QVector<Point>());
        Point isctPoint1;
        const bool isct1 = intersects(inPoly->at(curr), inPoly->at(next), &isctPoint1);
        if (isct1) // we need this test for the cases where the intersection is located directly on a point etc.
            poly->append(isctPoint1);

        curr = next; // move to next point after intersection

        // append points to current polygon
        while (true) {
            if (!rej[curr]) {
                poly->append(inPoly->at(curr));
                curr = (curr + 1) % n;
            } else {
                // end the current polygon at intersection on (curr - 1, curr)
                const int prev = (curr - 1 + n) % n;
                Q_ASSERT(!rej[prev] && (rej[curr]));
                Point isctPoint2;
                const bool isct2 = intersects(inPoly->at(prev), inPoly->at(curr), &isctPoint2);
                if (isct2) // we need this test for the cases where the intersection is located directly on a point etc.
                    poly->append(isctPoint2);
                outPolys->append(poly);
                break;
            }
        }

        // find start of next polygon if any
        Q_ASSERT(rej[curr]);
        while ((curr != first) && (rej[curr] && rej[(curr + 1) % n]))
            curr = (curr + 1) % n;

        if (curr == first)
            break; // back to where we started!

        Q_ASSERT(rej[curr] && (!rej[(curr + 1) % n]));
    }

    return outPolys;
}

QVector<Point> LineFilter::intersections(const Polygon &inPoly) const
{
    Q_ASSERT(!((type() == N_OF) || (type() == S_OF)));

    QVector<Point> points;
    Point point;
    for (int i = 0; i < inPoly->size(); ++i) {
        const Point p1(inPoly->at(i));
        const Point p2(inPoly->at((i + 1) % inPoly->size()));
        if ((rejected(p1) != rejected(p2)) && intersects(p1, p2, &point))
            points.append(point);
    }
    return points;
}

LonOrLatFilter::LonOrLatFilter(double lolValue)
    : value_(lolValue)
{
}

void LonOrLatFilter::setValue(double lolValue)
{
    value_ = lolValue;
}

double LonOrLatFilter::value() const
{
    return value_;
}

bool LonOrLatFilter::isLonFilter() const
{
    return (type() == E_OF) || (type() == W_OF);
}

void LonOrLatFilter::setFromVariant(const QVariant &var)
{
    bool ok = false;
    value_ = var.toDouble(&ok);
    Q_ASSERT(ok);
}

QVariant LonOrLatFilter::toVariant() const
{
    return value_;
}

bool LonOrLatFilter::setFromXmetExpr(const QString &expr, QPair<int, int> *matchedRange, QPair<int, int> *incompleteRange, QString *incompleteReason)
{
    QRegExp rx;
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    // look for keyword
    rx.setPattern(QString("(?:^|\\s+)%1\\s+OF").arg(directionName()));
    int rxpos1 = expr.indexOf(rx);
    if (rxpos1 < 0)
        return false; // no match

    int matchedLen1 = rx.matchedLength();

    // strip off leading whitespace
    while (expr.at(rxpos1).isSpace()) {
        rxpos1++;
        matchedLen1--;
    }

    // look for value
    if (isLonFilter()) {
        rx.setPattern("^\\s+([EW]\\d\\d\\d\\d\\d)");
    } else {
        rx.setPattern("^\\s+([NS]\\d\\d\\d\\d)");
    }

    const int rxpos2 = expr.mid(rxpos1 + matchedLen1).indexOf(rx);
    if (rxpos2 >= 0) { // match
        bool ok = false;
        const double val = isLonFilter() ? xmetExtractLon(rx.cap(1), ok) : xmetExtractLat(rx.cap(1), ok);
        if (ok) {
            value_ = val;
            matchedRange->first = rxpos1;
            matchedRange->second = rxpos1 + matchedLen1 + rxpos2 + rx.matchedLength() - 1;
            return true; // success
        }
    }

    // error
    incompleteRange->first = rxpos1;
    incompleteRange->second = rxpos1 + matchedLen1 - 1;
    *incompleteReason = QString("failed to extract a valid %1 value after '%2 OF'")
            .arg(isLonFilter() ? "longitude" : "latitude")
            .arg(directionName());
    return false; // no match
}

QString LonOrLatFilter::xmetExpr() const
{
    return QString("%1 OF %2").arg(directionName()).arg(isLonFilter() ? xmetFormatLon(value_) : xmetFormatLat(value_));
}

LonFilter::LonFilter(double value)
    : LonOrLatFilter(value)
{
}

bool LonFilter::intersects(const Point &p1, const Point &p2, Point *isctPoint) const
{
    const double minLat = qMin(p1.second, p2.second);
    const double maxLat = qMax(p1.second, p2.second);
    double lat1;
    double lat2;
    if ((minLat > 0) && (maxLat > 0)) {
        lat1 = -M_PI_4;
        lat2 = M_PI_2;
    } else if ((minLat < 0) && (maxLat < 0)) {
        lat1 = -M_PI_2;
        lat2 = M_PI_4;
    } else {
        lat1 = 0.99 * -M_PI_2;
        lat2 = 0.99 * M_PI_2;
    }
    return math::greatCircleArcsIntersect(p1, p2, qMakePair(value_, lat1), qMakePair(value_, lat2), isctPoint);
}

EOfFilter::EOfFilter(double value)
    : LonFilter(value)
{
}

bool EOfFilter::isValid() const
{
    return (value_ >= -M_PI) || (value_ <= M_PI);
}

bool EOfFilter::rejected(const Point &point) const
{
    return point.first < value_;
}

WOfFilter::WOfFilter(double value)
    : LonFilter(value)
{
}

bool WOfFilter::isValid() const
{
    return (value_ >= -M_PI) || (value_ <= M_PI);
}

bool WOfFilter::rejected(const Point &point) const
{
    return point.first > value_;
}

LatFilter::LatFilter(double value)
    : LonOrLatFilter(value)
{
}

struct Node {
    Point point_; // lon,lat radians of point represented by the node
    bool rejected_; // true iff point is rejected by the filter
    bool isct_; // true iff node represents an intersecion (if not it represents a vertex of the original input polygon)
    bool entry_; // whether the intersection node represents an entry into (true) or an exit from (false) the area accepted by the filter
                 // when traversing the original input polygon in clockwise direction
    Node(const Point &point, bool rejected, bool isct, bool entry = false)
        : point_(point), rejected_(rejected), isct_(isct), entry_(entry) {}
    Node() {}
};

static bool longitudeLessThan(const Node &node1, const Node &node2)
{
    return node1.point_.first < node2.point_.first;
}

// Returns true if node2 is the immediate westward neighbour of node1 in ilist (assumed to be ordered on increasing longitude).
static bool isWestwardNeighbour(const Node &node1, const Node &node2, const QList<Node> &ilist)
{
    // find node1 in ilist
    int i = -1;
    for (i = 0; i < ilist.size(); ++i)
        if (node1.point_.first == ilist.at(i).point_.first)
            break;
    Q_ASSERT((i >= 0) && (i < ilist.size()));

    // check if longitude of node2 matches the one of the previous node (i.e. the westward neighbour)
    const int prev = (i - 1 + ilist.size()) % ilist.size();
    return node2.point_.first == ilist.at(prev).point_.first;
}

static void addIntersection(const Point point, bool &entry, QList<Node> *tlist, QList<Node> *ilist)
{
    const Node node(point, false, true, entry);
    entry = !entry; // assuming entries and exits always alternate strictly
                    // (WARNING: this assumption probably doesn't hold for self-intersection polygons!)
    // append to both lists
    tlist->append(node);
    ilist->append(node);
}

Polygons LatFilter::apply(const Polygon &inPoly) const
{
    Q_ASSERT(!inPoly->isEmpty());
    const double lat = value_;
    Polygons outPolys = Polygons(new QVector<Polygon>());

    // get clockwise version of input polygon
    const Polygon inPolyCW(math::isClockwise(inPoly) ? inPoly : math::reversed(inPoly));
    Q_ASSERT(math::isClockwise(inPolyCW));

    QList<Node> tlist; // trace list consisting of 1) vertices of input polygon in clockwise order and 2) filter intersections as they appear, e.g.:
                       //   ORGV ORGV ISCT ORGV ISCT ISCT ORGV ORGV ISCT
                       // notes:
                       //    1: the first item is always an ORGV, but the last one doesn't have to be
                       //    2: there may be 0, 1 or 2 ISCTs between any pair of ORGVs (since a great circle arc may cross a latitude 0, 1 or 2 times)
    QList<Node> ilist; // intersection list consisting of all the ISCTs in tlist sorted by longitude (direction doesn't matter, but the value
                       // obviously wraps at some point: e.g. 4 5 6 0 1 2 3,  6 5 4 3 2 1 0,  3 2 1 0 6 5 4, etc.)

    // --- BEGIN generate tlist and initial ilist ------

    // if the first original vertex lies outside the area accepted by the filter, A, the first intersection must represent an entry into A
    bool entry = isNOfFilter() ? (inPolyCW->first().second < lat) : (inPolyCW->first().second > lat);

    // generate tlist and initial ilist

    int nrejected = 0;

    for (int i = 0; i < inPolyCW->size(); ++i) {

        const Point p1 = inPolyCW->at(i);
        const Point p2 = inPolyCW->at((i + 1) % inPolyCW->size());

        const bool rej1 = rejected(p1);
        const bool rej2 = rejected(p2);

        // append original vertex
        if (rej1)
            nrejected++;
        tlist.append(Node(p1, rej1, false));

        // find 0, 1 or 2 intersections between latitude circle and this edge
        const QVector<Point> ipoints = math::latitudeIntersections(p1, p2, lat); // 2 iscts are ordered on increasing dist. from p1

        // add intersection(s)
        if (ipoints.size() == 2) { // intersection type 1
            for (int j = 0; j < ipoints.size(); ++j) {
                // ### The following line fixes one problem, but introduces another since it now no longer handles
                //     the case where the latitude circle intersects a very long base polygon edge in two places!
                if ((j == 1) && (ipoints.at(0).second != ipoints.at(1).second))
                    addIntersection(ipoints.at(j), entry, &tlist, &ilist);
            }
        } else if (rej1 != rej2) { // intersection type 2
            if (ipoints.size() == 1) {
                addIntersection(ipoints.at(0), entry, &tlist, &ilist);
            } else { // latitude circle passes exactly on one of the edge vertices!
                Q_ASSERT(ipoints.size() == 0);
                addIntersection((qAbs(p1.second - lat) < qAbs(p2.second - lat)) ? p1 : p2, entry, &tlist, &ilist);
            }
        }
    }

    // --- END generate tlist and initial ilist ------


    // check special cases when there's no intersections
    if (ilist.isEmpty()) {

        const bool aroundNorthPole = math::pointInPolygon(qMakePair(0.0,  M_PI_2), inPolyCW);
        const bool aroundSouthPole = math::pointInPolygon(qMakePair(0.0, -M_PI_2), inPolyCW);

        if ((isNOfFilter() && aroundNorthPole) || ((!isNOfFilter()) && aroundSouthPole)) {
            if (nrejected == 0) {
                // Q_ASSERT(nrejected == inPolyCW->size());
                // return a list with one item: a deep copy (although implicitly shared for efficiency) of the input polygon
                Polygon inPolyCopy(new QVector<Point>(*inPolyCW.data()));
                outPolys->append(inPolyCopy);
                return outPolys;
            } else {
                // return polygon along latitude
                const int res = 32;
                outPolys->append(math::latitudeArcPoints(lat, 0, (2 * M_PI) / res, false, res, true));
                return outPolys;
            }
        } else if ((isNOfFilter() && aroundSouthPole) || ((!isNOfFilter()) && aroundNorthPole)) {
            if (nrejected == 0) {
                // Q_ASSERT(nrejected == inPolyCW->size());
                // special case that is not yet supported (result may contain a hole etc.); return empty list for now
                //qWarning() << "special case not yet supported!";
                return outPolys;

            } else {
                // return empty list
                return outPolys;
            }
        }

        Q_ASSERT(!(aroundNorthPole || aroundSouthPole));

        if (nrejected == 0) {
            // all points accepted, and not around pole, so return a list with one item: a deep copy
            // (although implicitly shared for efficiency) of the input polygon
            Polygon inPolyCopy(new QVector<Point>(*inPolyCW.data()));
            outPolys->append(inPolyCopy);
            return outPolys;
        } else {
            // Q_ASSERT(nrejected == inPolyCW->size()); // ### fails in some cases, but should it?
            // all(?) points reject, and not around pole, so return empty list
            return outPolys;
        }
    }


    // general case: intersections exist


    // sort ilist on longitude
    qSort(ilist.begin(), ilist.end(), longitudeLessThan);

//    if (!ilist.isEmpty())
//        qDebug() << "";
//    for (int i = 0; i < ilist.size(); ++i)
//        qDebug() << i << (ilist.at(i).entry_ ? "E" : "X") << ":" << ilist.at(i).point_.first;


    // define the begin, end, and direction of tracing
    int begin = -1;
    for (begin = 0; begin < tlist.size(); ++begin) {
        const Node node(tlist.at(begin));
        const bool isct = node.isct_;
        const bool entry = node.entry_;
        // start new polygons at entries for N_OF and at exits for S_OF
        if (isct && ((isNOfFilter() && entry) || ((!isNOfFilter()) && (!entry))))
            break;
    }
    Q_ASSERT((begin >= 0) && (begin < tlist.size()));
    const int add = isNOfFilter() ? 1 : -1; // trace clockwise for N_OF and counterclockwise for S_OF

    QStack<Polygon> polyStack; // stack of polygons
    QStack<Node> isctStack; // stack of intersections

    bool firstIter = true;
    for (int i = begin; firstIter || (i != begin); i = (i + add + tlist.size()) % tlist.size(), firstIter = false) {
        const Node node(tlist.at(i));

        if (node.isct_) {

            if ((isNOfFilter() && node.entry_) || ((!isNOfFilter()) && (!node.entry_))) {

                // intersection may start a new polygon or connect with intersection of an already started one

                if ((!isctStack.isEmpty()) && (isWestwardNeighbour(isctStack.top(), node, ilist))) {
                    // create extra edges along latitude in westwards direction from previously visited intersection to this one
                    Q_ASSERT(!polyStack.isEmpty());
                    const Node node2(isctStack.pop());
                    *polyStack.top() += *math::latitudeArcPoints(lat, node2.point_.first, node.point_.first, false, 32, false);

                } else {
                    // start new polygon
                    polyStack.push(Polygon(new QVector<Point>()));

                    // record intersection for subsequent matching against an intersection of the opposite type
                    isctStack.push(node);
                }

                Q_ASSERT(!polyStack.isEmpty());
                polyStack.top()->append(node.point_); // in any case, add the intersection itself

            } else {

                // intersection may close the current polygon

                Q_ASSERT(!polyStack.isEmpty());
                polyStack.top()->append(node.point_); // in any case, add the intersection itself

                if ((!isctStack.isEmpty()) && (isWestwardNeighbour(node, isctStack.top(), ilist))) {
                    // create extra edges along latitude in westwards direction from this intersection to previously visited one
                    const Node node2(isctStack.pop());
                    *polyStack.top() += *math::latitudeArcPoints(lat, node.point_.first, node2.point_.first, false, 32, false);

                    // close current polygon
                    outPolys->append(polyStack.pop());

                } else {
                    // record intersection for subsequent matching against an intersection of the opposite type
                    isctStack.push(node);
                }

            }

        } else if (!node.rejected_) {
            // node is vertex of input polygon inside accepted area, so add it to any current polygon
            if (!polyStack.isEmpty())
                polyStack.top()->append(node.point_);

        } else {
            // node is vertex of input polygon outside accepted area, so just skip it
        }
    }

    return outPolys;
}

QVector<Point> LatFilter::intersections(const Polygon &inPoly) const
{
    QVector<Point> points;
    for (int i = 0; i < inPoly->size(); ++i)
        points += math::latitudeIntersections(inPoly->at(i), inPoly->at((i + 1) % inPoly->size()), value_);
    return points;
}

bool LatFilter::intersects(const Point &p1, const Point &p2, Point *isctPoint) const
{
    const QVector<Point> points = math::latitudeIntersections(p1, p2, value_);
    if (!points.isEmpty()) {
        *isctPoint = points.first();
        return true;
    }
    return false;
}

bool LatFilter::isNOfFilter() const
{
    return directionName() == "N";
}

NOfFilter::NOfFilter(double value)
    : LatFilter(value)
{
}

bool NOfFilter::isValid() const
{
    return (value_ >= -M_PI_2) || (value_ <= M_PI_2);
}

bool NOfFilter::rejected(const Point &point) const
{
    return point.second < value_;
}

SOfFilter::SOfFilter(double value)
    : LatFilter(value)
{
}

bool SOfFilter::isValid() const
{
    return (value_ >= -M_PI_2) || (value_ <= M_PI_2);
}

bool SOfFilter::rejected(const Point &point) const
{
    return point.second > value_;
}

void FreeLineFilter::setLine(const Point &p1, const Point &p2)
{
    line_ = qMakePair(p1, p2);
}

void FreeLineFilter::setLine(const QPair<Point, Point> &line)
{
    line_ = line;
}

void FreeLineFilter::setPoint1(const Point &point)
{
    setLine(point, point2());
}

void FreeLineFilter::setPoint2(const Point &point)
{
    setLine(point1(), point);
}

FreeLineFilter::FreeLineFilter(const QPair<Point, Point> &line)
    : line_(line)
{
}

void FreeLineFilter::setFromVariant(const QVariant &var)
{
    const QVariantList list = var.toList();
    Q_ASSERT(list.size() == 4);
    bool ok0 = false;
    bool ok1 = false;
    bool ok2 = false;
    bool ok3 = false;
    Point p1(qMakePair(list.at(0).toDouble(&ok0), list.at(1).toDouble(&ok1)));
    Point p2(qMakePair(list.at(2).toDouble(&ok2), list.at(3).toDouble(&ok3)));
    Q_ASSERT(ok0 && ok1 && ok2 && ok3);
    setLine(p1, p2);
}

QVariant FreeLineFilter::toVariant() const
{
    QVariantList list;
    list.append(lon1());
    list.append(lat1());
    list.append(lon2());
    list.append(lat2());
    return list;
}

bool FreeLineFilter::isValid() const
{
    if ((type() == E_OF_LINE) || (type() == W_OF_LINE)) {
        return lat1() != lat2();
    } else if ((type() == N_OF_LINE) || (type() == S_OF_LINE)) {
        return lon1() != lon2();
    } else if ((type() == NW_OF_LINE) || (type() == SE_OF_LINE)) {
        return ((lon1() < lon2()) && (lat1() < lat2())) || ((lon1() > lon2()) && (lat1() > lat2()));
    }
    // (type() == NE_OF_LINE) || (type() == SW_OF_LINE)
    return ((lon1() < lon2()) && (lat1() > lat2())) || ((lon1() > lon2()) && (lat1() < lat2()));
}

bool FreeLineFilter::intersects(const Point &p1, const Point &p2, Point *isctPoint) const
{
    Point isctPoint1;
    Point isctPoint2;
    const int n = math::greatCircleArcIntersectsGreatCircle(p1, p2, qMakePair(lon1(), lat1()), qMakePair(lon2(), lat2()), &isctPoint1, &isctPoint2);
    if (n == 1) {
        *isctPoint = isctPoint1;
        return true; // exactly one intersection
    }
    return false; // either zero, two or infinitely many intersections
}

bool FreeLineFilter::setFromXmetExpr(const QString &expr, QPair<int, int> *matchedRange, QPair<int, int> *incompleteRange, QString *incompleteReason)
{
    QRegExp rx;
    rx.setCaseSensitivity(Qt::CaseInsensitive);

    // look for keyword
    rx.setPattern(QString("(?:^|\\s+)%1\\s+OF\\s+LINE").arg(directionName()));
    const int rxpos1 = expr.indexOf(rx);
    if (rxpos1 < 0)
        return false; // no match

    const int matchedLen1 = rx.matchedLength();

    // look for values
    rx.setPattern(
                "^\\s+([NS]\\d\\d\\d\\d)\\s*([EW]\\d\\d\\d\\d\\d)"
                "\\s+-\\s+"
                "([NS]\\d\\d\\d\\d)\\s*([EW]\\d\\d\\d\\d\\d)"
                );

    const int rxpos2 = expr.mid(rxpos1 + matchedLen1).indexOf(rx);
    if (rxpos2 >= 0) { // match
        bool ok1 = false;
        bool ok2 = false;
        bool ok3 = false;
        bool ok4 = false;
        const double lon1 = xmetExtractLon(rx.cap(2), ok1);
        const double lat1 = xmetExtractLat(rx.cap(1), ok2);
        const double lon2 = xmetExtractLon(rx.cap(4), ok3);
        const double lat2 = xmetExtractLat(rx.cap(3), ok4);
        if (!(ok1 && ok2 && ok3 && ok4))
            return false;
        setLine(qMakePair(qMakePair(lon1, lat1), qMakePair(lon2, lat2)));
        matchedRange->first = rxpos1;
        matchedRange->second = rxpos1 + matchedLen1 + rxpos2 + rx.matchedLength() - 1;
        return true; // success
    }

    // error
    incompleteRange->first = rxpos1;
    incompleteRange->second = rxpos1 + matchedLen1 - 1;
    *incompleteReason = QString("failed to extract two valid coordinates after '%1 OF LINE'").arg(directionName());
    return false;
}

QString FreeLineFilter::xmetExpr() const
{
    return QString("%1 OF LINE %2 %3 - %4 %5")
            .arg(directionName())
            .arg(xmetFormatLat(lat1()))
            .arg(xmetFormatLon(lon1()))
            .arg(xmetFormatLat(lat2()))
            .arg(xmetFormatLon(lon2()));
}

bool FreeLineFilter::rejected(const Point &point) const
{
    const double lon0 = point.first;
    const double lat0 = point.second;
    double lon1_ = lon1();
    double lat1_ = lat1();
    double lon2_ = lon2();
    double lat2_ = lat2();
    if (
            (((type() == NE_OF_LINE) || (type() == NW_OF_LINE) || (type() == N_OF_LINE) || (type() == S_OF_LINE)) && (lon1_ > lon2_)) ||
            (((type() == SE_OF_LINE) || (type() == SW_OF_LINE) || (type() == E_OF_LINE) || (type() == W_OF_LINE)) && (lat1_ > lat2_))) {
        std::swap(lon1_, lon2_);
        std::swap(lat1_, lat2_);
    }

    const double crossDist = math::crossTrackDistanceToGreatCircle(
                qMakePair(lon0, lat0),
                qMakePair(lon1_, lat1_),
                qMakePair(lon2_, lat2_));

    switch (type()) {
    case NE_OF_LINE: return crossDist > 0;
    case NW_OF_LINE: return crossDist > 0;
    case SE_OF_LINE: return crossDist < 0;
    case SW_OF_LINE: return crossDist > 0;
    case  E_OF_LINE: return crossDist < 0;
    case  W_OF_LINE: return crossDist > 0;
    case  N_OF_LINE: return crossDist > 0;
    case  S_OF_LINE: return crossDist < 0;
    default:
        ;
    }

    return true;
}

EOfLineFilter::EOfLineFilter(const QPair<Point, Point> &line)
    : FreeLineFilter(line)
{
}

WOfLineFilter::WOfLineFilter(const QPair<Point, Point> &line)
    : FreeLineFilter(line)
{
}

NOfLineFilter::NOfLineFilter(const QPair<Point, Point> &line)
    : FreeLineFilter(line)
{
}

SOfLineFilter::SOfLineFilter(const QPair<Point, Point> &line)
    : FreeLineFilter(line)
{
}

NEOfLineFilter::NEOfLineFilter(const QPair<Point, Point> &line)
    : FreeLineFilter(line)
{
}

NWOfLineFilter::NWOfLineFilter(const QPair<Point, Point> &line)
    : FreeLineFilter(line)
{
}

SEOfLineFilter::SEOfLineFilter(const QPair<Point, Point> &line)
    : FreeLineFilter(line)
{
}

SWOfLineFilter::SWOfLineFilter(const QPair<Point, Point> &line)
    : FreeLineFilter(line)
{
}

FIR &FIR::instance()
{
    static FIR fir;
    return fir;
}

static Polygon createENORFIR()
{
    Polygon points = Polygon(new QVector<Point>);

    const int npoints = sizeof(enor_fir) / sizeof(float) / 2;
    for (int i = 0; i < npoints; ++i) {
        const double lon = DEG2RAD(enor_fir[2 * i + 1]);
        const double lat = DEG2RAD(enor_fir[2 * i]);
        points->append(qMakePair(lon, lat));
    }
    return points;
}

static Polygon createENOBFIR()
{
    Polygon points = Polygon(new QVector<Point>);

    const int npoints = sizeof(enob_fir) / sizeof(float) / 2;
    for (int i = 0; i < npoints; ++i) {
        const double lon = DEG2RAD(enob_fir[2 * i + 1]);
        const double lat = DEG2RAD(enob_fir[2 * i]);
        points->append(qMakePair(lon, lat));
    }
    return points;
}

FIR::FIR()
{
    fir_.insert(ENOR, FIRInfo(createENORFIR(), "ENOR NORWAY FIR"));
    fir_.insert(ENOB, FIRInfo(createENOBFIR(), "ENOB BODO OCEANIC FIR"));
}

QStringList FIR::supportedNames() const
{
    QStringList snames;
    foreach (Code code, fir_.keys()) {
        snames.append(fir_.value(code).name_);
    }
    return snames;
}

Polygon FIR::polygon(Code code) const
{
    return fir_.value(code).polygon_;
}

FIR::Code FIR::firFromText(const QString &text)
{
    foreach (Code code, fir_.keys()) {
        if (text.indexOf(fir_.value(code).name_, 0, Qt::CaseInsensitive) >= 0)
            return code;
    }
    return FIR::Unsupported;
}

//------------------------------------------------------------------------------------------------

Polygons applyFilters(const Polygons &inPolys, const Filters &filters)
{
    // initialize final output
    Polygons outPolys(new QVector<Polygon>());
    if (inPolys && (!inPolys->isEmpty()))
        *outPolys += *inPolys;
    else
        return outPolys; // empty input, so return empty output

    // apply valid filters
    for (int i = 0; filters && (i < filters->size()); ++i) {
        const Filter filter = filters->at(i);
        if (!filter->isValid())
            continue;

        // set the input polygons for this filter to be the output polygons from the previous filter
        Polygons inPolys2(outPolys);

        // create an empty list of output polygons to be filled in by this filter
        outPolys = Polygons(new QVector<Polygon>());

        // loop over input polygons
        for (int j = 0; j < inPolys2->size(); ++j) {
            if (inPolys2->at(j)) {
                // apply the filter to the input polygon and add the resulting polygons to the final output
                Polygons outPolys2 = filter->apply(inPolys2->at(j));
                if (outPolys2) {
                    for (int k = 0; k < outPolys2->size(); ++k)
                        outPolys->append(outPolys2->at(k));
                }
            }
        }
    }

    return outPolys;
}

Polygons applyFilters(const Polygon &polygon, const Filters &filters)
{
    Polygons polygons = Polygons(new QVector<Polygon>());
    polygons->append(polygon ? polygon : Polygon(new QVector<Point>()));
    return applyFilters(polygons, filters);
}

QString xmetExprFromFilters(const Filters &filters)
{
    QString s;
    bool addSpace = false;
    bool prevValidIsLonOrLat = false;
    for (int i = 0; i < filters->size(); ++i) {
        const Filter filter = filters->at(i);
        if (filter->isValid()) {
            if (addSpace)
                s += " ";
            addSpace = true; // add a space from now on

            const bool isLonOrLat = dynamic_cast<LonOrLatFilter *>(filter.data());
            if (isLonOrLat && prevValidIsLonOrLat)
                s += "AND ";
            prevValidIsLonOrLat = isLonOrLat;

            s += filter->xmetExpr();
        }
    }
    return s;
}

struct ParseMatchInfo
{
    Filter filter_;
    int loPos_;
    int hiPos_;
    bool missingAnd_;
    ParseMatchInfo(Filter filter, int loPos, int hiPos)
        : filter_(filter), loPos_(loPos), hiPos_(hiPos), missingAnd_(false) {}
};

static bool parseMatchInfoLessThan(const ParseMatchInfo &pmi1, const ParseMatchInfo &pmi2)
{
    return pmi1.loPos_ < pmi2.loPos_;
}

struct ParseIncompleteInfo
{
    Filter filter_;
    int loPos_;
    int hiPos_;
    QString reason_;
    ParseIncompleteInfo(Filter filter, int loPos, int hiPos, const QString &reason)
        : filter_(filter), loPos_(loPos), hiPos_(hiPos), reason_(reason) {}
};

static bool parseIncompleteInfoLessThan(const ParseIncompleteInfo &pii1, const ParseIncompleteInfo &pii2)
{
    return pii1.loPos_ < pii2.loPos_;
}

Filters filtersFromXmetExpr(
        const QString &expr, QList<QPair<int, int> > *matchedRanges, QList<QPair<QPair<int, int>, QString> > *incompleteRanges,
        bool wiExclusive)
{
    // ### NOTE: for now, we assume that any given filter may match at most once. Whether this makes sense depends on the filter:
    // - 'S OF' shouldn't match more than once (but if it does, only the one with the southernmost value should apply!)
    // - 'WI' and 'S OF LINE' could both match more than once.
    // and so on ...

    // candidate filters (the result will consist of a subset of these, ordered according to their appearance in the)
    QList<Filter> candFilters;
    candFilters.append(Filter(new WithinFilter));
    //
    candFilters.append(Filter(new EOfFilter));
    candFilters.append(Filter(new WOfFilter));
    candFilters.append(Filter(new NOfFilter));
    candFilters.append(Filter(new SOfFilter));
    //
    candFilters.append(Filter(new EOfLineFilter));
    candFilters.append(Filter(new WOfLineFilter));
    candFilters.append(Filter(new NOfLineFilter));
    candFilters.append(Filter(new SOfLineFilter));
    //
    candFilters.append(Filter(new NEOfLineFilter));
    candFilters.append(Filter(new NWOfLineFilter));
    candFilters.append(Filter(new SEOfLineFilter));
    candFilters.append(Filter(new SWOfLineFilter));

    // find initial matched and incomplete ranges
    QList<ParseMatchInfo> pmInfos;
    QList<ParseIncompleteInfo> piInfos;
    foreach (Filter filter, candFilters) {
        QPair<int, int> matchedRange(-1, -1);
        QPair<int, int> incompleteRange(-1, -1);
        QString incompleteReason;
        if (filter->setFromXmetExpr(expr, &matchedRange, &incompleteRange, &incompleteReason)) {
            //matchedRanges->append(matchedRange);
            pmInfos.append(ParseMatchInfo(filter, matchedRange.first, matchedRange.second));
        } else if (incompleteRange.first >= 0) {
            //initIncompleteRanges.append(qMakePair(incompleteRange, incompleteReason));
            piInfos.append(ParseIncompleteInfo(filter, incompleteRange.first, incompleteRange.second, incompleteReason));
        }
    }

    // only keep incomplete ranges that are not already part of a matched range (e.g. 'E OF' is part of 'E OF LINE')
    {
        QList<ParseIncompleteInfo> piInfos2;
        for (int i = 0; i < piInfos.size(); ++i) {
            const int ilo = piInfos.at(i).loPos_;
            const int ihi = piInfos.at(i).hiPos_;
            bool partOfMatchedRange = false;
            for (int j = 0; j < pmInfos.size(); ++j) {
                if ((ilo >= pmInfos.at(j).loPos_) && (ihi <= pmInfos.at(j).hiPos_)) {
                    partOfMatchedRange = true;
                    break;
                }
            }
            if (!partOfMatchedRange)
                piInfos2.append(piInfos.at(i));
        }
        piInfos = piInfos2;
    }

    // sort matched ranges on position
    qSort(pmInfos.begin(), pmInfos.end(), parseMatchInfoLessThan);

    // require the single word 'AND' between adjacent lon/lat filters
    QList<QPair<int, int> > extraMatchedRanges;
    {
        QRegExp rx("^\\s+and\\s+$");
        for (int i = 1; i < pmInfos.size(); ++i) {
            const LonFilter *lon1 = dynamic_cast<LonFilter *>(pmInfos.at(i - 1).filter_.data());
            const LonFilter *lon2 = dynamic_cast<LonFilter *>(pmInfos.at(i).filter_.data());
            const LatFilter *lat1 = dynamic_cast<LatFilter *>(pmInfos.at(i - 1).filter_.data());
            const LatFilter *lat2 = dynamic_cast<LatFilter *>(pmInfos.at(i).filter_.data());
            if ((lon1 || lat1) && (lon2 || lat2)) {
                const int lo = pmInfos.at(i - 1).hiPos_ + 1;
                const int hi = pmInfos.at(i).loPos_ - 1;
                const QString text = expr.mid(lo, (hi - lo) + 1);
                if (!text.toLower().contains(rx))
                    pmInfos[i - 1].missingAnd_ = true;
                else
                    extraMatchedRanges.append(qMakePair(lo, hi));
            }
        }
    }

    // change status of each matched lon/lat filter that misses an 'AND' before an adjacent lon/lat filter from
    // matched to incomplete
    {
        QList<ParseMatchInfo> pmInfos2;
        foreach (ParseMatchInfo pmInfo, pmInfos) {
            if (pmInfo.missingAnd_) {
                const QPair<int, int> range = qMakePair(pmInfo.loPos_, pmInfo.hiPos_);
                piInfos.append(ParseIncompleteInfo(pmInfo.filter_, range.first, range.second, QString("missing single 'AND' between expressions")));
            } else {
                pmInfos2.append(pmInfo);
            }
        }
        pmInfos = pmInfos2;
    }

    // sort incomplete ranges on position
    qSort(piInfos.begin(), piInfos.end(), parseIncompleteInfoLessThan);

    // handle 'WI exclusive' mode
    bool keepWIFilter = false;
    if (wiExclusive) {
        int wiMatchLoPos = -1;
        int wiMatchHiPos = -1;
        for (int i = 0; i < pmInfos.size(); ++i) {
            if (dynamic_cast<WithinFilter *>(pmInfos.at(i).filter_.data())) {
                wiMatchLoPos = pmInfos.at(i).loPos_;
                wiMatchHiPos = pmInfos.at(i).hiPos_;
                break;
            }
        }
        int wiIncompleteLoPos = -1;
        int wiIncompleteHiPos = -1;
        if (wiMatchLoPos == -1) {
            for (int i = 0; i < piInfos.size(); ++i) {
                if (dynamic_cast<WithinFilter *>(piInfos.at(i).filter_.data())) {
                    wiIncompleteLoPos = piInfos.at(i).loPos_;
                    wiIncompleteHiPos = piInfos.at(i).hiPos_;
                    break;
                }
            }
        }

        if ((wiMatchLoPos == -1) && (wiIncompleteLoPos == -1)) {
            // no WI filter found, neither matched nor incomplete
            keepWIFilter = false;
        } else if ((pmInfos.size() + piInfos.size()) == 1) {
            // no other (matched or incomplete) filters found in addition to the (matched or incomplete) WI filter
            keepWIFilter = true;
        } else {
            int wiLoPos = -1;
            int wiHiPos = -1;
            if (wiMatchLoPos >= 0) {
                wiLoPos = wiMatchLoPos;
                wiHiPos = wiMatchHiPos;
            } else {
                wiLoPos = wiIncompleteLoPos;
                wiHiPos = wiIncompleteHiPos;
            }
            Q_ASSERT(wiLoPos >= 0);
            Q_ASSERT(wiLoPos < wiHiPos);

            int loPos = -1;
            if (!pmInfos.isEmpty())
                loPos = pmInfos.first().loPos_;
            if (!piInfos.isEmpty())
                loPos = (loPos == -1) ? piInfos.first().loPos_ : qMin(loPos, piInfos.first().loPos_);
            Q_ASSERT(loPos >= 0);

            if (loPos < wiLoPos) {
                // another (matched or incomplete) filter occurs before the (matched or incomplete) WI filter
                keepWIFilter = false;
            } else {
                // the (matched or incomplete) WI filter occurs before any other (matched or incomplete) filter
                keepWIFilter = true;
            }
        }
    } // handle 'WI exclusive' mode

    // copy to matched ranges and result filters
    Filters resultFilters(new QList<Filter>());
    foreach (ParseMatchInfo pmInfo, pmInfos) {
        const WithinFilter *wiFilter = dynamic_cast<WithinFilter *>(pmInfo.filter_.data());
        if ((!wiExclusive) || ((keepWIFilter && wiFilter) || ((!keepWIFilter) && (!wiFilter)))) {
            matchedRanges->append(qMakePair(pmInfo.loPos_, pmInfo.hiPos_));
            resultFilters->append(pmInfo.filter_);
        }
    }
    if ((!wiExclusive) || (!keepWIFilter)) {
        // add extra matched ranges
        for (int i = 0; i < extraMatchedRanges.size(); ++i)
            matchedRanges->append(extraMatchedRanges.at(i));
    }

    // copy to incomplete ranges
    foreach (ParseIncompleteInfo piInfo, piInfos) {
        const WithinFilter *wiFilter = dynamic_cast<WithinFilter *>(piInfo.filter_.data());
        if ((!wiExclusive) || ((keepWIFilter && wiFilter) || ((!keepWIFilter) && (!wiFilter)))) {
            incompleteRanges->append(qMakePair(qMakePair(piInfo.loPos_, piInfo.hiPos_), piInfo.reason_));
        }
    }

    return resultFilters;
}

void setIntersectablePolygons(const Polygons &polygons)
{
    PolygonIntersector::instance().setPolygons(polygons); // creates new quadtree based on lat/lon bounding boxes ...
}

QList<QPair<int, Polygons> > intersectedPolygons(const Polygons &intersectors)
{
    return PolygonIntersector::instance().intersection(intersectors);
}

double area(const Polygon &polygon)
{
    return -1; // ### TBD
}

double area(const Polygons &polygons)
{
    return -1; // ### TBD
}


MGP_END_NAMESPACE
