#include "util3d.h"
#include <stdexcept>
#include <float.h>
#include <QDebug>
#include <QLinkedList>
#include <QHash>
#include <stdio.h> // 4 TESTING!
#include <iostream> // ### for debugging

void Math::normalize(double &x, double &y)
{
    double
	nrm = norm(x, y),
	xx = x / nrm,
	yy = y / nrm;

    if ((!finite(xx)) || (!finite(yy)))
        throw std::runtime_error("norm too small");

    x = xx;
    y = yy;
}

void Math::normalize(double &x, double &y, double &z)
{
    double
	nrm = norm(x, y, z),
	xx = x / nrm,
	yy = y / nrm,
	zz = z / nrm;

    if ((!finite(xx)) || (!finite(yy)) || (!finite(yy)))
        throw std::runtime_error("norm too small");

    x = xx;
    y = yy;
    z = zz;
}

double Math::angle(double x, double y)
{
    Math::normalize(x, y);
    double a = asin(y);
    if (x < 0) a = M_PI - a;
    if (a < 0)          a += (2 * M_PI); else
    if (a > (2 * M_PI)) a -= (2 * M_PI);
    return a;
}

/**
 * Returns the spherical distance (i.e. along the great circle on the unit sphere) between two points.
 *
 * @param   p1 - first point (lon,lat in radians).
 * @param   p2 - second point (lon,lat in radians).
 * @returns Distance between p1 and p2 on unit sphere.
 */
double Math::distance(const QPair<double, double> &p1, const QPair<double, double> &p2)
{
    const double theta1 = p1.first;
    const double phi1   = p1.second;
    const double theta2 = p2.first;
    const double phi2   = p2.second;

    const double dphi = phi2 - phi1;
    const double dtheta = theta2 - theta1;

    const double a =
            sin(dphi / 2) * sin(dphi / 2) +
            cos(phi1) * cos(phi2) *
            sin(dtheta / 2) * sin(dtheta / 2);

    return 2 * atan2(sqrt(a), sqrt(1 - a));
}

QVector<_3DPoint> Math::getGreatCirclePoints(double lon1, double lat1, double lon2, double lat2, int nSegments)
{
    QVector<_3DPoint> points;

    for (int i = 0; i <= nSegments; ++i) {
        const double t = i / double(nSegments);
        const double d = acos(sin(lat1) * sin(lat2) + cos(lat1) * cos(lat2) * cos(lon1 - lon2));
        const double A = sin((1 - t) * d) / sin(d);
        const double B = sin(t * d) / sin(d);
        const double x = A * cos(lat1) * cos(lon1) + B * cos(lat2) * cos(lon2);
        const double y = A * cos(lat1) * sin(lon1) + B * cos(lat2) * sin(lon2);
        const double z = A * sin(lat1) + B * sin(lat2);
        points.append(_3DPoint(x, y, z));
    }

    return points;
}

/**
 * Returns the (initial) bearing from one point to another.
 *
 * @param   p0 - source point (lon,lat in radians).
 * @param   p1 - destination point (lon,lat in radians).
 * @returns Initial bearing in radians from north.
 */
double Math::bearingBetween(const QPair<double, double> &p0, const QPair<double, double> &p1)
{
    const double phi_1 = p0.second;
    const double phi_2 = p1.second;
    const double delta_lambda = p1.first - p0.first;

    // see http://mathforum.org/library/drmath/view/55417.html
    const double y = sin(delta_lambda) * cos(phi_2);
    const double x = cos(phi_1) * sin(phi_2) - sin(phi_1) * cos(phi_2) * cos(delta_lambda);
    const double theta = atan2(y, x);

    return fmod(theta + 2 * M_PI, 2 * M_PI);
};

/**
 * Returns (signed) distance from a point to a great circle.
 *
 * @param   p0 - source point (lon,lat in radians).
 * @param   p1 - start point of great circle path (lon,lat in radians).
 * @param   p2 - end point of great circle path (lon, lat in radians).
 * @param   radius - Sphere radius.
 * @returns Distance to great circle (< 0 if to left, > 0 if to right of path).
 */
double Math::crossTrackDistanceToGreatCircle(
        const QPair<double, double> &p0, const QPair<double, double> &p1, const QPair<double, double> &p2, double radius)
{
    const double delta_13 = distance(p1, p0);
    const double theta_13 = bearingBetween(p1, p0);
    const double theta_12 = bearingBetween(p1, p2);

    return asin(sin(delta_13) * sin(theta_13 - theta_12)) * radius;
}

// Returns true iff the great circle passing through p1 and p2 intersects lat. The intersection point closest to p1 and p2 is returned in isctPoint.
// Adopted from 'Crossing parallels' on http://williams.best.vwh.net/avform.htm .
// Alternative approach: http://math.stackexchange.com/questions/1157278/find-the-intersection-point-of-a-great-circle-arc-and-latitude-line .
bool Math::intersectsLatitude(const QPair<double, double> &p1, const QPair<double, double> &p2, double lat, QPair<double, double> *isctPoint)
{
    const double lon1 = p1.first;
    const double lat1 = p1.second;
    const double lon2 = p2.first;
    const double lat2 = p2.second;

    const double l12 = lon1 - lon2;
    const double A = sin(lat1) * cos(lat2) * cos(lat) * sin(l12);
    const double B = sin(lat1) * cos(lat2) * cos(lat) * cos(l12) - cos(lat1) * sin(lat2) * cos(lat);
    const double C = cos(lat1) * cos(lat2) * sin(lat) * sin(l12);
    const double lon = atan2(B, A); // atan2(y, x) convention
    if (fabs(C) <= sqrt(A * A + B * B)) {
        const double dlon = acos(C / sqrt(A * A + B * B));
        const double lon3_1 = fmod(lon1 + dlon + lon + M_PI, 2 * M_PI) - M_PI;
        const double lon3_2 = fmod(lon1 - dlon + lon + M_PI, 2 * M_PI) - M_PI;

        const QPair<double, double> p3_1(lon3_1, lat);
        const double dist11 = distance(p1, p3_1);
        const double dist21 = distance(p2, p3_1);
        const QPair<double, double> p3_2(lon3_2, lat);
        const double dist12 = distance(p1, p3_2);
        const double dist22 = distance(p2, p3_2);
        isctPoint->first = ((dist11 + dist21) < (dist12 + dist22)) ? lon3_1 : lon3_2;

        isctPoint->second = lat;

        return true;
    }

    return false; // no crossing
}

// Returns true iff the great circle arc from p1 to p2 intersects the great circle arc from p3 to p4.
// The intersection point closest to p1 and p2 is returned in isctPoint.
// NOTE: If the two great circles lie (approximately) in the same plane, the function returns false (even if there are infinite numbers
// of intersections!).
// Adopted from http://www.mathworks.com/matlabcentral/newsreader/view_thread/276271 .
// Alternative approaches:
//   1: http://stackoverflow.com/questions/2954337/great-circle-rhumb-line-intersection
//   2: http://www.boeing-727.com/Data/fly%20odds/distance.html
//   3: http://www.movable-type.co.uk/scripts/latlong-vectors.html
bool Math::greatCircleArcsIntersect(
        const QPair<double, double> &p1, const QPair<double, double> &p2,
        const QPair<double, double> &p3, const QPair<double, double> &p4,
        QPair<double, double> *isctPoint)
{
    const _3DPoint a0 = _3DPoint::fromSpherical(p1.first, p1.second);
    const _3DPoint a1 = _3DPoint::fromSpherical(p2.first, p2.second);
    const _3DPoint b0 = _3DPoint::fromSpherical(p3.first, p3.second);
    const _3DPoint b1 = _3DPoint::fromSpherical(p4.first, p4.second);

    const _3DPoint p = _3DPoint::cross(a0, a1); // normal of plane 1
    const _3DPoint q = _3DPoint::cross(b0, b1); // normal of plane 2

    const _3DPoint t = _3DPoint::cross(p, q);
    if (t.norm() < FLT_MIN)
        return false; // arcs lie (approximately) in the same plane => no intersections

    const double s1 = _3DPoint::dot(_3DPoint::cross(p, a0), t);
    const double s2 = _3DPoint::dot(_3DPoint::cross(a1, p), t);
    const double s3 = _3DPoint::dot(_3DPoint::cross(q, b0), t);
    const double s4 = _3DPoint::dot(_3DPoint::cross(b1, q), t);

    double sign = 0;
    if ((s1 > 0) && (s2 > 0) && (s3 > 0) && (s4 > 0))
        sign = 1; // the arcs intersect along +t
    else if ((s1 < 0) && (s2 < 0) && (s3 < 0) && (s4 < 0))
        sign = -1; // the arcs intersect along -t
    else
        return false; // the arcs don't intersect (or maybe lie in the same plane if this wasn't detected by the above test for this?)

    if (isctPoint) {
        isctPoint->first = atan2(sign * t.y(), sign * t.x());
        isctPoint->second = atan2(sign * t.z(), sqrt(t.x() * t.x() + t.y() * t.y()));
    }

    return true;
}

// Returns true iff a point is considered inside a polygon.
bool Math::pointInPolygon(const QPair<double, double> &point, const PointVector &points, bool inverse)
{
    // define an external point (i.e. a point that is assumed to be outside the polygon)
    double avgLon = 0;
    double minLat;
    double maxLat;
    minLat = maxLat = points->first().second;
    for (int i = 0; i < points->size(); ++i) {
        avgLon += points->at(i).first;
        const double lat = points->at(i).second;
        minLat = qMin(lat, minLat);
        maxLat = qMax(lat, maxLat);
    }
    avgLon /= points->size();
    QPair<double, double> extPoint(avgLon, 0.99 * M_PI / 2);
    if ((M_PI / 2 - maxLat) < (minLat - (-M_PI / 2)))
        // polygon is closer to the north pole, so use a point close to the south pole as the external point
        extPoint.second = -extPoint.second;

    if (inverse)
        extPoint.second = - extPoint.second;

    // compute the number of intersections between 1) the arc from the point to the external point and
    // 2) argcs forming the polygon
    int nisct = 0;
    for (int i = 0; i < points->size(); ++i) {
        if (Math::greatCircleArcsIntersect(
                    point, extPoint,
                    points->at(i), points->at((i + 1) % points->size())))
            nisct++;
    }

    // the point is considered inside the polygon if there is an odd number of intersections
    return nisct % 2;
}

// Returns true iff a polygon is oriented clockwise.
// (Adopted from http://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order .
// See also: https://en.wikipedia.org/wiki/Curve_orientation . )
bool Math::isClockwise(const PointVector &points)
{
    double sum = 0;
    for (int i = 0; i < points->size(); ++i) {
        const int next = (i + 1) % points->size();
        sum += (points->at(next).first - points->at(i).first) * (points->at(next).second + points->at(i).second);
    }
    return (sum > 0);
}

double * Math::sphericalToCartesian(double radius, double phi, double theta)
{
    static double c[3];
    c[0] = radius * cos(phi) * cos(theta);
    c[1] = radius * cos(phi) * sin(theta);
    c[2] = radius * sin(phi);
    return c;
}

void Math::sphericalToCartesian(double radius, double phi, double theta, double &x, double &y, double &z)
{
    x = radius * cos(phi) * cos(theta);
    y = radius * cos(phi) * sin(theta);
    z = radius * sin(phi);
}

void Math::cartesianToSpherical(double x, double y, double z, double &phi, double &theta)
{
    _4x4Matrix m;
    theta = Math::angle(x, y);
    m.loadRotateZ(-theta);
    _4DPoint p;
    p.set(x, y, z);
    p.mulMatPoint(m);
    phi = Math::angle(p.get(0), p.get(2));
}

void Math::computeCamera(
    double radius, double phi_eye, double theta_eye, double alt_eye, double phi_tgt, double theta_tgt, double alpha, _4DPoint &eye, _4DPoint &tgt_alpha, _4DPoint &up)
{
    _4x4Matrix m, m0;

    // Compute eye point ...
    eye.set(radius + alt_eye, 0, 0);
    m0.loadRotateZ(theta_eye);
    m.loadRotateY(phi_eye);
    m0.mulMat(m);
    eye.mulMatPoint(m0);

    // Compute non-pitched target point ...
    _4DPoint tgt;
    tgt.set(radius + alt_eye, 0, 0);
    m0.loadRotateZ(theta_tgt);
    m.loadRotateY(phi_tgt);
    m0.mulMat(m);
    tgt.mulMatPoint(m0);

    // Compute pitched target point and up vector ...
    _4DPoint tgt_cross_eye = tgt;
    tgt_cross_eye.cross(eye);
    tgt_alpha = tgt;
    tgt_alpha.subtract(eye);
    tgt_alpha.rotate(tgt_cross_eye, alpha);
    up = tgt_alpha;
    up.rotate(tgt_cross_eye, 0.5 * M_PI);
    tgt_alpha.add(eye);
}

// Determines if the ray originating from point (px, py, pz) in
// direction (rx, ry, rz) intersects the sphere having center
// (cx, cy, cz) and radius r. In the case of intersection, the
// nearest intersection point is returned in (x, y, z).
//
bool Math::raySphereIntersect(double px, double py, double pz, double rx, double ry, double rz, double cx, double cy, double cz, double r, double &x, double &y, double &z)
{
    // Equation of ray:
    //   x = px + t * rx
    //   y = py + t * ry
    //   z = pz + t * rz
    //
    // Equation of sphere:
    //   (x - cx)^2 + (y - cy)^2 + (z - cz)^2 = r^2
    //
    // Subsititute for x, y, z and solve for t (2nd degree polynomial):

    double
	a = Math::sqr(rx) +  Math::sqr(ry) +  Math::sqr(rz),
	b = 2 * (rx * (px - cx) + ry * (py - cy) + rz * (pz - cz)),
	c = Math::sqr(px - cx) + Math::sqr(py - cy) +
	    Math::sqr(pz - cz) - Math::sqr(r),
	sqrt_arg = Math::sqr(b) - 4.0 * a * c;

    if (sqrt_arg < 0)
	return false; // No intersection.

    // Compute nearest intersection point ...
    double
	t1 = (-b + sqrt(sqrt_arg)) / (2 * a),
	t2 = (-b - sqrt(sqrt_arg)) / (2 * a),
	t_lo = Math::min(t1, t2);

    if (t_lo < 0)
	return false; // Intersection behind (px, py, pz)

    x = px + t_lo * rx;
    y = py + t_lo * ry;
    z = pz + t_lo * rz;

    return true;
}

void Math::computeLatLon(double x, double y, double z, double &lat, double &lon)
{
    Math::normalize(x, y, z);

    lat = asin(z);

/* NEW: */
    try
    {
        lon = Math::angle(x, y);
    }
    catch (std::runtime_error &)
    {
        lon = 0;
    }

/* OLD:
    lon = atan(y / x);
    if (isnan(lon))
	lon = (y > 0) ? M_PI_2 : (3 * M_PI_2);
    else if (x < 0)
	lon += M_PI;
    else if (y < 0)
	lon += (2 * M_PI);
*/
}

static PointVector reversed(const PointVector &points)
{
    PointVector copy(new QVector<QPair<double, double> >());
    for (int i = points->size() - 1; i >= 0; --i)
        copy->append(points->at(i));
    return copy;
}

struct IsctInfo {
    int isctId_; // non-negative intersection ID
    int c_; // intersection on line (c, (c + 1) % C.size()) in clip polygon C for 0 <= c < C.size()
    int s_; // intersection on line (s, (s + 1) % S.size()) in subject polygon S for 0 <= s < S.size()
    QPair<double, double> point_; // lon,lat radians of intersection point
    double cdist_; // distance between C->at(c) and point_
    double sdist_; // distance between S->at(s) and point_
    IsctInfo(int isctId, int c, int s, const QPair<double, double> &point, double cdist, double sdist)
        : isctId_(isctId)
        , c_(c)
        , s_(s)
        , point_(point)
        , cdist_(cdist)
        , sdist_(sdist)
    {}
};

struct Node {
    QPair<double, double> point_; // lon,lat radians of point represented by the node
    int isctId_; // non-negative intersection ID, or < 0 if the node does not represent an intersection
    QLinkedList<Node>::iterator neighbour_; // pointer to corresponding intersection node in other list
    bool entry_; // whether the intersection node represents an entry into (true) or exit from (false) the clipped polygon
    bool visited_; // whether the intersection node has already been processed in the generation of output polygons
    Node(const QPair<double, double> &point, int isctId = -1) : point_(point), isctId_(isctId), entry_(false), visited_(false) {}
};

static void printLists(const QString &tag, const QLinkedList<Node> &slist, const QLinkedList<Node> &clist)
{
    {
        std::cout << tag.toLatin1().data() << "; subj: ";
        QLinkedList<Node>::const_iterator it;
        for (it = slist.constBegin(); it != slist.constEnd(); ++it) {
            if (it->isctId_ < 0) {
                std::cout << "V  ";
            } else {
                std::cout << it->isctId_ << "<" << (it->entry_ ? "entry" : "exit") << ">  ";
                Q_ASSERT(it->isctId_ == it->neighbour_->isctId_);
            }
        }
        std::cout << std::endl;
    }

    {
        std::cout << tag.toLatin1().data() << "; clip: ";
        QLinkedList<Node>::const_iterator it;
        for (it = clist.constBegin(); it != clist.constEnd(); ++it) {
            if (it->isctId_ < 0) {
                std::cout << "V  ";
            } else {
                std::cout << it->isctId_ << "<" << (it->entry_ ? "entry" : "exit") << ">  ";
                Q_ASSERT(it->isctId_ == it->neighbour_->isctId_);
            }
        }
        std::cout << std::endl << std::endl;
    }
}

// This function implements the Greiner-Hormann clipping algorithm:
// - http://www.inf.usi.ch/hormann/papers/Greiner.1998.ECO.pdf
//
// See also:
// - https://en.wikipedia.org/wiki/Greiner%E2%80%93Hormann_clipping_algorithm
// - https://en.wikipedia.org/wiki/Weiler%E2%80%93Atherton_clipping_algorithm
// - https://www.jasondavies.com/maps/clip/
PointVectors Math::polygonIntersection(const PointVector &subject, const PointVector &clip, bool inverse)
{
    // set up output polygons
    PointVectors outPolys = PointVectors(new QVector<PointVector>());

#if 0
    // set up input polygons and ensure they are oriented clockwise
    const PointVector C(Math::isClockwise(clip) ? clip : reversed(clip));
    Q_ASSERT(Math::isClockwise(C));
    const PointVector S(Math::isClockwise(subject) ? subject : reversed(subject));
    Q_ASSERT(Math::isClockwise(S));
#else
    // set up input polygons regardless of orientation (seems to work fine)
    const PointVector C(clip);
    const PointVector S(subject);
#endif

    // find intersections
    int isctId = 0;
    QHash<int, QList<IsctInfo> > sIscts; // intersections for edges in S
    QHash<int, QList<IsctInfo> > cIscts; // intersections for edges in C
    for (int s = 0; s < S->size(); ++s) { // loop over vertices in S
        for (int c = 0; c < C->size(); ++c) { // loop over vertices in C
            QPair<double, double> isctPoint;
            if (Math::greatCircleArcsIntersect(S->at(s), S->at((s + 1) % S->size()), C->at(c), C->at((c + 1) % C->size()), &isctPoint)) {
                const IsctInfo isctInfo(
                            isctId++, c, s, isctPoint,
                            Math::distance(C->at(c), isctPoint),
                            Math::distance(S->at(s), isctPoint));

                // insert the intersection for the S edge in increasing distance from vertex s
                if (!sIscts.contains(s))
                    sIscts.insert(s, QList<IsctInfo>());
                QList<IsctInfo> &silist = sIscts[s];
                {
                    int i = 0;
                    for (; (i < silist.size()) && (silist.at(i).sdist_ < isctInfo.sdist_); ++i) ;
                    silist.insert(i, isctInfo);
                }

                // insert the intersection for the C edge in increasing distance from vertex c
                if (!cIscts.contains(c))
                    cIscts.insert(c, QList<IsctInfo>());
                QList<IsctInfo> &cilist = cIscts[c];
                {
                    int i = 0;
                    for (; (i < cilist.size()) && (cilist.at(i).cdist_ < isctInfo.cdist_); ++i) ;
                    cilist.insert(i, isctInfo);
                }
            }
        }
    }

    Q_ASSERT(!sIscts.isEmpty() || cIscts.isEmpty());
    Q_ASSERT(!cIscts.isEmpty() || sIscts.isEmpty());


    // ************************************************************************
    // * CASE 1: No intersections exist between clip and subject polygons     *
    // ************************************************************************
    if (sIscts.isEmpty()) {
        Q_ASSERT(cIscts.isEmpty());

        // compute number of subject points inside clip polygon
        int sPointsInC = 0;
        for (int i = 0; i < S->size(); ++i)
            if (Math::pointInPolygon(S->at(i), C, inverse))
                sPointsInC++;

        if (sPointsInC == S->size()) {
            // the subject polygon is completely enclosed within the clip polygon, so return a list with one item:
            // a deep copy (although implicitly shared for efficiency) of the subject polygon
            PointVector sCopy(new QVector<QPair<double, double> >(*S.data()));
            outPolys->append(sCopy);
            return outPolys;
        }

        // ### disable this test for now; the assertion could be caused by pointInPolygon() not being 100% robust
        //if (sPointsInC != 0)
        //    qDebug() << "WARNING: more than zero subject points inside clip polygon:" << sPointsInC;
        // Q_ASSERT(sPointsInC == 0); // otherwise there would be at least one intersection!

        // compute number of clip points inside subject polygon
        int cPointsInS = 0;
        for (int i = 0; i < C->size(); ++i)
            if (Math::pointInPolygon(C->at(i), S, inverse))
                cPointsInS++;

        if (cPointsInS == C->size()) {
            // the clip polygon is completely enclosed within the subject polygon, so return a list with one item:
            // a deep copy (although implicitly shared for efficiency) of the clip polygon
            PointVector cCopy(new QVector<QPair<double, double> >(*C.data()));
            outPolys->append(cCopy);
            return outPolys;
        }

        // ### disable this test for now; the assertion could be caused by pointInPolygon() not being 100% robust
        //if (cPointsInS != 0)
        //    qDebug() << "WARNING: more than zero clip points inside subject polygon:" << cPointsInS;
        // Q_ASSERT(cPointsInS == 0); // otherwise there would be at least one intersection!

        // at this point, the clip and subject polygons are completely disjoint, so return an empty list
        return outPolys;
    }


    // **********************************************************************
    // * CASE 2: Intersections exist between clip and subject polygons      *
    // **********************************************************************

    // *** PHASE 0: Create initial linked lists *********

    // create list with original vertices and intersections for subject polygon
    QLinkedList<Node> slist;
    for (int i = 0; i < S->size(); ++i) {
        // append node for point i
        slist.push_back(Node(S->at(i)));

        // append nodes for intersections on line (i, (i + 1) % S.size())
        if (sIscts.contains(i)) {
            const QList<IsctInfo> silist = sIscts.value(i);
            for (int j = 0; j < silist.size(); ++j) {
                Q_ASSERT((j == 0) || (silist.at(j - 1).sdist_ <= silist.at(j).sdist_));
                slist.push_back(Node(silist.at(j).point_, silist.at(j).isctId_));
            }
        }
    }

    // create list with original vertices and intersections for clip polygon
    QLinkedList<Node> clist;
    for (int i = 0; i < C->size(); ++i) {
        // append node for point i
        clist.push_back(Node(C->at(i)));

        // append nodes for intersections on line (i, (i + 1) % C.size())
        if (cIscts.contains(i)) {
            const QList<IsctInfo> cilist = cIscts.value(i);
            for (int j = 0; j < cilist.size(); ++j) {
                Q_ASSERT((j == 0) || (cilist.at(j - 1).cdist_ <= cilist.at(j).cdist_));
                clist.push_back(Node(cilist.at(j).point_, cilist.at(j).isctId_));
            }
        }
    }


    // *** PHASE 1: Connect corresponding intersection nodes *********

    {
        QLinkedList<Node>::iterator sit;
        for (sit = slist.begin(); sit != slist.end(); ++sit) {
            if (sit->isctId_ >= 0) {
                // find the corresponding intersection node in the clist and connect them together
                QLinkedList<Node>::iterator cit;
                for (cit = clist.begin(); cit != clist.end(); ++cit) {
                    if (cit->isctId_ == sit->isctId_) {
                        sit->neighbour_ = cit;
                        cit->neighbour_ = sit;
                    }
                }
            }
        }
    }


    // *** PHASE 2: Set entry/exit status for each intersection node *********

    {
        // whether the next intersection represents an entry into the clip polygon
        bool entry = !Math::pointInPolygon(slist.first().point_, C, inverse);

        QLinkedList<Node>::iterator it;
        for (it = slist.begin(); it != slist.end(); ++it) {
            if (it->isctId_ >= 0) {
                it->entry_ = entry;
                entry = !entry; // if this intersection was an entry, the next one must be an exit and vice versa
            }
        }
    }

    {
        // whether the next intersection represents an entry into the subject polygon
        bool entry = !Math::pointInPolygon(clist.first().point_, S, inverse);

        QLinkedList<Node>::iterator it;
        for (it = clist.begin(); it != clist.end(); ++it) {
            if (it->isctId_ >= 0) {
                it->entry_ = entry;
                entry = !entry; // if this intersection was an entry, the next one must be an exit and vice versa
            }
        }
    }

//    printLists(QString::number(nn), slist, clist);


    // *** PHASE 3: Generate clipped polygons *********
    {

        // loop over original vertices and intersections in subject polygon
        QLinkedList<Node>::iterator sit;
        for (sit = slist.begin(); sit != slist.end(); ++sit) {
            if ((sit->isctId_ >= 0) && (!sit->visited_)) {
                // this is an unvisited intersection, so start tracing a new polygon
                PointVector poly(new QVector<QPair<double, double> >());

                QLinkedList<Node>::iterator it(sit);
                bool forward = it->entry_;
                do {

                    // move one step along the current list
                    if (forward) {
                        it++;
                        if (it == slist.end())
                            it = slist.begin();
                        else if (it == clist.end())
                            it = clist.begin();
                    } else {
                        if (it == slist.begin())
                            it = slist.end();
                        else if (it == clist.begin())
                            it = clist.end();
                        it--;
                    }

                    if (it->isctId_ < 0) {
                        // original vertex
                        poly->append(it->point_); // append to new polygon
                    } else {
                        // intersection
                        poly->append(it->point_); // append to new polygon
                        it = it->neighbour_; // move to corresponding intersection in other list
                        it->visited_ = it->neighbour_->visited_ = true; // indicate that we're done with this intersection
                        forward = it->entry_; // update direction
                    }

                    // return an empty result if the algorithm has seemed to entered an infinite loop
                    // (this could for example happen when Math::greatCircleArcsIntersect() fails to find an intersection)
                    if (poly->size() > 2 * S->size() * C->size())
                        return PointVectors();

                } while (it->isctId_ != sit->isctId_); // as long as tracing has not got back to where it started

                if (poly->size() >= 3) // hm ... wouldn't this always be the case?
                    outPolys->append(poly);
            }
        }
    }

    return outPolys;
}

_3DPoint::_3DPoint()
{
    for (int i = 0; i < 3; i++) c_[i] = 0;
}

_3DPoint::_3DPoint(const _3DPoint &src)
{
    c_[0] = src.x();
    c_[1] = src.y();
    c_[2] = src.z();
}

_3DPoint::_3DPoint(double x, double y, double z)
{
    c_[0] = x;
    c_[1] = y;
    c_[2] = z;
}

double _3DPoint::norm() const
{
    return sqrt(c_[0] * c_[0] + c_[1] * c_[1] + c_[2] * c_[2]);
}

_3DPoint _3DPoint::fromSpherical(double lon, double lat)
{
    return _3DPoint(cos(lon) * cos(lat), sin(lon) * cos(lat), sin(lat));
}

_3DPoint _3DPoint::cross(const _3DPoint &a, const _3DPoint &b)
{
    return _3DPoint(
                a.y() * b.z() - a.z() * b.y(),
                a.z() * b.x() - a.x() * b.z(),
                a.x() * b.y() - a.y() * b.x());
}

double _3DPoint::dot(const _3DPoint &a, const _3DPoint &b)
{
    return a.x() * b.x() + a.y() * b.y() + a.z() * b.z();
}

void _3DPoint::normalize()
{
    Math::normalize(c_[0], c_[1], c_[2]);
}

_3DPoint _3DPoint::normalized(const _3DPoint &p)
{
    _3DPoint np(p);
    np.normalize();
    return np;
}

void _3DPoint::print(char lead[]) const
{
    fprintf(stderr, "%s:\n", lead);
    for (int i = 0; i < 3; i++)
	fprintf(stderr, "  %20.10f", c_[i]);
    fprintf(stderr, "\n");
}

_4DPoint::_4DPoint()
{
    for (int i = 0; i < 3; i++) c_[i] = 0;
}

_4DPoint::_4DPoint(const _4DPoint &src)
{
    for (int i = 0; i < 4; i++) c_[i] = src.get(i);
}

_4DPoint::_4DPoint(double x, double y, double z)
{
    set(x, y, z);
}

// Multiplies m with this point (m * this) storing the result in
// this point.
void _4DPoint::mulMatPoint(const _4x4Matrix &m)
{
    _4DPoint orig =  *this;
    for (int i = 0; i < 4; i++)
    {
	c_[i] = 0;
	for (int j = 0; j < 4; j++)
	    c_[i] += (m.get(i, j) * orig.get(j));
    }
}

// Computes and returns the dot product of this point and m.
double _4DPoint::dot(const _4DPoint &p)
{
    return
	c_[0] * p.get(0) + 
	c_[1] * p.get(1) + 
	c_[2] * p.get(2);
}

// Computes the cross product of this point and p (this X p) storing the
// result in this point.
void _4DPoint::cross(const _4DPoint &p)
{
    _4DPoint orig =  *this;

    c_[0] = orig.get(1) * p.get(2) - orig.get(2) * p.get(1);
    c_[1] = orig.get(2) * p.get(0) - orig.get(0) * p.get(2);
    c_[2] = orig.get(0) * p.get(1) - orig.get(1) * p.get(0);
}

// Rotates this point an angle alpha around p storing the result in this point.
void _4DPoint::rotate(const _4DPoint &p, const double alpha)
{
    _4DPoint p0(p);
    _4x4Matrix m, m1;

    // Compute theta and phi ...
    double theta, phi;
    Math::computeLatLon(p0.x(), p0.y(), p0.z(), phi, theta);

    // Compute final transformation ...
    m1.loadRotateZ(theta);

    m.loadRotateY(-phi);
    m1.mulMat(m);
 
    m.loadRotateX(alpha);
    m1.mulMat(m);
 
    m.loadRotateY(phi);
    m1.mulMat(m);
 
    m.loadRotateZ(-theta);
    m1.mulMat(m);

    // Multiply with this point ...
    mulMatPoint(m1);
}

void _4DPoint::normalize()
{
    Math::normalize(c_[0], c_[1], c_[2]);
}

void _4DPoint::scale(double fact)
{
    c_[0] *= fact;
    c_[1] *= fact;
    c_[2] *= fact;
}

void _4DPoint::add(const _4DPoint &p)
{
    c_[0] += p.get(0);
    c_[1] += p.get(1);
    c_[2] += p.get(2);
}

void _4DPoint::subtract(const _4DPoint &p)
{
    c_[0] -= p.get(0);
    c_[1] -= p.get(1);
    c_[2] -= p.get(2);
}

void _4DPoint::print(char lead[]) const
{
    fprintf(stderr, "%s:\n", lead);
    for (int i = 0; i < 4; i++)
	fprintf(stderr, "  %20.10f", c_[i]);
    fprintf(stderr, "\n");
}

_4x4Matrix::_4x4Matrix()
{
    loadIdentity();
}

void _4x4Matrix::mulMat(const _4x4Matrix &m)
{
    _4x4Matrix orig = *this;

    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	{
	    c_[i][j] = 0;
	    for (int k = 0; k < 4; k++)
		c_[i][j] += (orig.get(i, k) * m.get(k, j));
	}
}

void _4x4Matrix::mulMatLeft(const _4x4Matrix &m)
{
    _4x4Matrix orig = *this;

    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	{
	    c_[i][j] = 0;
	    for (int k = 0; k < 4; k++)
		c_[i][j] += (m.get(i, k) * orig.get(k, j));
	}
}

void _4x4Matrix::loadIdentity()
{
    for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
	    c_[i][j] = (i == j) ? 1 : 0;
}

void _4x4Matrix::loadRotateX(double theta)
{
    loadIdentity();
    c_[1][1] =  cos(theta);
    c_[1][2] = -sin(theta);
    c_[2][1] =  sin(theta);
    c_[2][2] =  cos(theta);
}

void _4x4Matrix::loadRotateY(double theta)
{
    loadIdentity();
    c_[0][0] =  cos(theta);
    c_[0][2] =  sin(theta);
    c_[2][0] = -sin(theta);
    c_[2][2] =  cos(theta);
}

void _4x4Matrix::loadRotateZ(double theta)
{
    loadIdentity();
    c_[0][0] =  cos(theta);
    c_[0][1] = -sin(theta);
    c_[1][0] =  sin(theta);
    c_[1][1] =  cos(theta);
}

void _4x4Matrix::loadTranslate(double x, double y, double z)
{
    loadIdentity();
    c_[0][3] = x;
    c_[1][3] = y;
    c_[2][3] = z;
}

void _4x4Matrix::print(char lead[]) const
{
    fprintf(stderr, "%s:\n", lead);
    for (int i = 0; i < 4; i++)
    {
	for (int j = 0; j < 4; j++)
	    fprintf(stderr, "  %20.10f", c_[i][j]);
	fprintf(stderr, "\n");
    }
}
