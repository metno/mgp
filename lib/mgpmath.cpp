#include "mgpmath.h"
#include <math.h>
#include <QHash>
#include <QList>
#include <QLinkedList>
#include <float.h>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <QDebug>

MGP_BEGIN_NAMESPACE
MGPMATH_BEGIN_NAMESPACE

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
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++)
            fprintf(stderr, "  %20.10f", c_[i][j]);
        fprintf(stderr, "\n");
    }
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

// Computes and returns the dot product of this point and p (this · p).
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

_3DPoint & _3DPoint::operator=(const _3DPoint &src)
{
    if (this != &src) {
        c_[0] = src.x();
        c_[1] = src.y();
        c_[2] = src.z();
    }
    return *this;
}

_3DPoint::_3DPoint(double x, double y, double z)
{
    c_[0] = x;
    c_[1] = y;
    c_[2] = z;
}

_3DPoint::_3DPoint(const Point &p)
{
    Math::sphericalToCartesian(1, p.second, p.first, c_[0], c_[1], c_[2]);
}

double _3DPoint::norm() const
{
    return sqrt(c_[0] * c_[0] + c_[1] * c_[1] + c_[2] * c_[2]);
}

Point _3DPoint::toSpherical() const
{
    double lon, lat;
    Math::cartesianToSpherical(c_[0], c_[1], c_[2], lat, lon);

    return qMakePair(lon, lat);
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

void _3DPoint::subtract(const _3DPoint &p)
{
    c_[0] -= p.get(0);
    c_[1] -= p.get(1);
    c_[2] -= p.get(2);
}

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

double Math::norm(double x, double y)
{
    return sqrt(x * x + y * y);
}

double Math::norm(double x, double y, double z)
{
    return sqrt(x * x + y * y + z * z);
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

double Math::angle(const Point &p0, const Point &p1, const Point &p2, bool *valid, bool debug)
{
    const _3DPoint cp0(p0);

    _3DPoint a(p1); a.subtract(cp0);
    _3DPoint b(p2); b.subtract(cp0);

    const double normA = a.norm();
    const double normB = b.norm();
    const double cosTheta = _3DPoint::dot(a, b) / (normA * normB);
    if (isnan(cosTheta)) {
        if (debug)
            qDebug() << "   cosTheta (nan):" << cosTheta << ", normA:" << normA << ", normB:" << normB << ", p0 p1 p2:" << p0 << p1 << p2
                     << ", p0==p1:" << (p0 == p1) << ", p0==p2:" << (p0 == p2)
                     << ", cp0 a b:" << cp0 << a << b;
        if (valid)
            *valid = false;
        return -1;
    }

    if (valid)
        *valid = true;
    return acos(cosTheta);
}

double Math::distance(const Point &p1, const Point &p2)
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

double Math::bearingBetween(const Point &p1, const Point &p2)
{
    const double phi_1 = p1.second;
    const double phi_2 = p2.second;
    const double delta_lambda = p2.first - p1.first;

    // see http://mathforum.org/library/drmath/view/55417.html
    const double y = sin(delta_lambda) * cos(phi_2);
    const double x = cos(phi_1) * sin(phi_2) - sin(phi_1) * cos(phi_2) * cos(delta_lambda);
    const double theta = atan2(y, x);

    return fmod(theta + 2 * M_PI, 2 * M_PI);
}

void Math::sphericalToCartesian(double radius, double phi, double theta, double &x, double &y, double &z)
{
    x = radius * cos(phi) * cos(theta);
    y = radius * cos(phi) * sin(theta);
    z = radius * sin(phi);
}

void Math::cartesianToSpherical(double x, double y, double z, double &phi, double &theta)
{
    phi = atan2(z, sqrt(x * x + y * y));
    theta = atan2(y, x);
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
    t_lo = std::min(t1, t2);

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

bool greatCircleArcsIntersect(const Point &p1, const Point &p2, const Point &p3, const Point &p4, Point *isctPoint)
{
    // Adopted from http://www.mathworks.com/matlabcentral/newsreader/view_thread/276271 .
    // Alternative approaches:
    //   1: http://stackoverflow.com/questions/2954337/great-circle-rhumb-line-intersection
    //   2: http://www.boeing-727.com/Data/fly%20odds/distance.html
    //   3: http://www.movable-type.co.uk/scripts/latlong-vectors.html

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

bool greatCirclesIntersect(const Point &p1, const Point &p2, const Point &p3, const Point &p4, Point *isctPoint1, Point *isctPoint2)
{
    // Adopted from http://stackoverflow.com/questions/2954337/great-circle-rhumb-line-intersection

    const _3DPoint a0 = _3DPoint::fromSpherical(p1.first, p1.second);
    const _3DPoint a1 = _3DPoint::fromSpherical(p2.first, p2.second);
    const _3DPoint b0 = _3DPoint::fromSpherical(p3.first, p3.second);
    const _3DPoint b1 = _3DPoint::fromSpherical(p4.first, p4.second);

    const _3DPoint p = _3DPoint::cross(a0, a1); // normal of plane 1
    const _3DPoint q = _3DPoint::cross(b0, b1); // normal of plane 2

    const _3DPoint t = _3DPoint::cross(p, q);
    if (t.norm() < FLT_MIN)
        return false; // great circles lie (approximately) in the same plane => no intersections

    // normalize t to find one of the intersections
    const _3DPoint t1 = _3DPoint::normalized(t);
    // the other intersection is on the opposite side of the sphere
    const _3DPoint t2 = _3DPoint(-t1.x(), -t1.y(), -t1.z());

    isctPoint1->first = atan2(t1.y(), t1.x());
    //isctPoint1->second = atan2(t1.z(), sqrt(t1.x() * t1.x() + t1.y() * t1.y()));
    isctPoint1->second = asin(t1.z());

    isctPoint2->first = atan2(t2.y(), t2.x());
    //isctPoint2->second = atan2(t2.z(), sqrt(t2.x() * t2.x() + t2.y() * t2.y()));
    isctPoint2->second = asin(t2.z());

    return true;
}

int greatCircleArcIntersectsGreatCircle(const Point &p1, const Point &p2, const Point &p3, const Point &p4, Point *isctPoint1, Point *isctPoint2)
{
    // find the intersections between the two great circles
    if (!greatCirclesIntersect(p1, p2, p3, p4, isctPoint1, isctPoint2))
        return 0; // no intersections

    // find the arc length
    const double arcLen = Math::distance(p1, p2);

    // check if each intersection is on the arc
    const bool onArc1 = (Math::distance(*isctPoint1, p1) <= arcLen) && (Math::distance(*isctPoint1, p2) <= arcLen);
    const bool onArc2 = (Math::distance(*isctPoint2, p1) <= arcLen) && (Math::distance(*isctPoint2, p2) <= arcLen);

    // set return value and out parameter(s)
    int n = 0;
    if (onArc1) {
        n = 1;
        if (onArc2)
            n = 2;
    } else if (onArc2) {
        n = 1;
        *isctPoint1 = *isctPoint2;
    }

    return n;
}

double crossTrackDistanceToGreatCircle(const Point &p0, const Point &p1, const Point &p2)
{
    const double delta_13 = Math::distance(p1, p0);
    const double theta_13 = Math::bearingBetween(p1, p0);
    const double theta_12 = Math::bearingBetween(p1, p2);
    return asin(sin(delta_13) * sin(theta_13 - theta_12));
}


// Returns the projection of p0 on the great circle between p1 and p2.
// NOTE: We assume that this function may throw std::runtime_error iff p1 and p2 are identical or very close.
static Point pointProjOnGreatCircle(const Point &p0, const Point &p1, const Point &p2)
{
    // Adopted from http://gis.stackexchange.com/questions/144007/project-location-on-a-great-circle-path

    // compute normal of great circle plane
    const _3DPoint U = _3DPoint::normalized(_3DPoint::cross(_3DPoint(p1), _3DPoint(p2)));

    const _3DPoint P = _3DPoint(p0);

    // compute distance from p0 to great circle plane
    const double d = _3DPoint::dot(P, U);

    // project p0 to great circle plane
    // >>>>> X' = (x', y', z') = X - d*U = (x - d*xu, y - d*yu, z - d*zu).
    const _3DPoint P_mark1 = _3DPoint(P.x() - d * U.x(), P.y() - d * U.y(), P.z() - d * U.z());

    // extend radially to great circle itself (### should be combined with previous statement for better performance!)
    // >>>>> X'' = X' / |X'|.
    const _3DPoint P_mark2 = _3DPoint::normalized(P_mark1);

    // convert back to spherical coordinate
    return P_mark2.toSpherical();
}


double distanceToGreatCircleArc(const Point &p0, const Point &p1, const Point &p2)
{
    // check if the projected p0 is located inside or outside of the arc
    {
        const double d12 = Math::distance(p1, p2);

        Point p0_proj;
        try {
            p0_proj = pointProjOnGreatCircle(p0, p1, p2);

            const double d01_proj = Math::distance(p0_proj, p1);
            const double d02_proj = Math::distance(p0_proj, p2);

            if (qMax(d01_proj, d02_proj) > d12) {
                // ... outside, so return the distance between p0 and the nearest endpoint
                const double d01 = Math::distance(p0, p1);
                const double d02 = Math::distance(p0, p2);
                return qMin(d01, d02);
            }

        } catch (std::runtime_error e) {
            // assume this is because p1 and p2 are identical or very close
            return qMin(Math::distance(p0, p1), Math::distance(p0, p2));
        }
    }

    // ... inside (and p1 and p2 sufficiently far apart), so return the unsigned projection distance
    return qAbs(crossTrackDistanceToGreatCircle(p0, p1, p2));
}


QVector<_3DPoint> greatCirclePoints(const QPair<double, double> &p1, const QPair<double, double> &p2, int nSegments, bool segmentOnly)
{
    QVector<_3DPoint> points;
    const double lon1 = p1.first;
    const double lat1 = p1.second;
    const double lon2 = p2.first;
    const double lat2 = p2.second;

    if (segmentOnly) {
        // compute the points between p1 and p2
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
    } else {
        // compute the points of the complete circle
        const _3DPoint a0 = _3DPoint::fromSpherical(p1.first, p1.second);
        const _3DPoint a1 = _3DPoint::fromSpherical(p2.first, p2.second);
        _3DPoint p;
        try {
            p = _3DPoint::normalized(_3DPoint::cross(a0, a1)); // plane normal
        } catch (std::runtime_error e) {
            return points; // this could for example happen if a0 and a1 are identical, in which case the great circle is undefined
        }

        // compute the plane normal
        double planePhi;
        double planeTheta;
        Math::cartesianToSpherical(p.x(), p.y(), p.z(), planePhi, planeTheta);

        // compute elements of rotation matrices
        const double alpha = M_PI / 2 - planePhi;
        const double cos1 = cos(alpha);
        const double sin1 = sin(alpha);
        const double cos2 = cos(planeTheta);
        const double sin2 = sin(planeTheta);

        const double deltaTheta = (2 * M_PI) / nSegments;
        double theta = 0;
        for (int i = 0; i <= nSegments; ++i, theta += deltaTheta) {
            // compute unrotated point along equator
            const double x0 = cos(theta);
            const double y0 = sin(theta);
            const double z0 = 0;

            // rotate according to plane normal ...
            // ... rotate M_PI / 2 - planePhi around Y axis
            const double x1 = x0 * cos1 + z0 * sin1;
            const double y1 = y0;
            const double z1 = -x0 * sin1 + z0 * cos1;

            // ... rotate planeTheta around Z axis
            const double x2 = x1 * cos2 - y1 * sin2;
            const double y2 = x1 * sin2 + y1 * cos2;
            const double z2 = z1;

            points.append(_3DPoint(x2, y2, z2));
        }
    }

    return points;
}

bool pointInPolygon(const Point &point, const Polygon &polygon)
{
    // define an external point (i.e. a point that is assumed to be outside the polygon)
    double avgLon = 0;
    double minLat;
    double maxLat;
    minLat = maxLat = polygon->first().second;
    for (int i = 0; i < polygon->size(); ++i) {
        avgLon += polygon->at(i).first;
        const double lat = polygon->at(i).second;
        minLat = qMin(lat, minLat);
        maxLat = qMax(lat, maxLat);
    }
    avgLon /= polygon->size();
    Point extPoint(avgLon, 0.99 * M_PI_2);
    if ((M_PI_2 - maxLat) < (minLat - (-M_PI_2)))
        // polygon is closer to the north pole, so use a point close to the south pole as the external point
        extPoint.second = -extPoint.second;

    // compute the number of intersections between 1) the arc from the point to the external point and
    // 2) argcs forming the polygon
    int nisct = 0;
    for (int i = 0; i < polygon->size(); ++i) {
        if (greatCircleArcsIntersect(
                    point, extPoint,
                    polygon->at(i), polygon->at((i + 1) % polygon->size())))
            nisct++;
    }

    // the point is considered inside the polygon if there is an odd number of intersections
    return nisct % 2;
}

struct IsctInfo {
    int isctId_; // non-negative intersection ID
    int c_; // intersection on line (c, (c + 1) % C.size()) in clip polygon C for 0 <= c < C.size()
    int s_; // intersection on line (s, (s + 1) % S.size()) in subject polygon S for 0 <= s < S.size()
    Point point_; // lon,lat radians of intersection point
    double cdist_; // distance between C->at(c) and point_
    double sdist_; // distance between S->at(s) and point_
    IsctInfo(int isctId, int c, int s, const Point &point, double cdist, double sdist)
        : isctId_(isctId)
        , c_(c)
        , s_(s)
        , point_(point)
        , cdist_(cdist)
        , sdist_(sdist)
    {}
};

struct Node {
    Point point_; // lon,lat radians of point represented by the node
    int isctId_; // non-negative intersection ID, or < 0 if the node does not represent an intersection
    QLinkedList<Node>::iterator neighbour_; // pointer to corresponding intersection node in other list
    bool entry_; // whether the intersection node represents an entry into (true) or an exit from (false) the clipped polygon
    bool visited_; // whether the intersection node has already been processed in the generation of output polygons
    Node(const Point &point, int isctId = -1) : point_(point), isctId_(isctId), entry_(false), visited_(false) {}
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


// Removes coincident neighbours in p.
static void removeCoincidentNeighbours(const Polygon &p)
{
    const double epsilon = 0.001;
    for (int i = p->size() - 1; i >= 0; --i) {
        if (Math::distance(p->at(i), p->at((i + 1) % p->size())) < epsilon)
            p->remove(i);
    }

}


// Returns a version of p0 that is perturbed at least epsilon away from the segment between p1 and p2.
static Point perturbedVertex(const Point &p0, const Point &p1, const Point &p2, double epsilon)
{
    const bool moreSouthNorth = (qAbs(p1.first - p2.first) < qAbs(p1.second - p2.second));

    Point p(p0);
    if (moreSouthNorth)
        p.first += (3 * epsilon);
    else
        p.second += (3 * epsilon);

    return p;
}


// Attempts to ensure that each edge in the clip polygon (c) is intersectable with relevant edges in the subject polygon (s)
// by eliminating degenerate cases as far as possible. This reduces the possibility of polygonIntersection() generating a false
// result. Degenerate cases are essentially those in which a vertex is too close to an edge. This may in turn lead to ambiguities
// that cause the main algorithm to fail.
//
static void fixDegenerate(const Polygon &s, const Polygon &c)
{
    const double epsilon = 0.0001; // seems appropriate for cases we have encountered in practice so far
    const int nc = c->size();
    const int ns = s->size();

    // *** STEP 1: perturb each vertex in s so that none is too close to an edge in c. ***
    for (int i = 0; i < nc; ++i) {
        for (int j = 0; j < ns; ++j) {
            if (distanceToGreatCircleArc(s->at(j), c->at(i), c->at((i + 1) % nc)) < epsilon) {
                // vertex(s, j) is too close to edge(c, i, i + 1), so perturb vertex(s, j) ...
                (*s)[j] = perturbedVertex(s->at(j), c->at(i), c->at((i + 1) % nc), epsilon);
            }
        }
    }

    // *** STEP 2: perturb each vertex in c so that none is too close to an edge in s. ***
    for (int j = 0; j < ns; ++j) {
        for (int i = 0; i < nc; ++i) {
            if (distanceToGreatCircleArc(c->at(i), s->at(j), s->at((j + 1) % ns)) < epsilon) {
                // vertex(c, i) is too close to edge(s, j, j + 1), so perturb vertex(c, i) ...
                (*c)[i] = perturbedVertex(c->at(i), s->at(j), s->at((j + 1) % ns), epsilon);
            }
        }
    }
}


Polygons polygonIntersection(const Polygon &subject, const Polygon &clip)
{
    // This function implements the Greiner-Hormann clipping algorithm:
    // - http://www.inf.usi.ch/hormann/papers/Greiner.1998.ECO.pdf
    //
    // See also:
    // - https://en.wikipedia.org/wiki/Greiner%E2%80%93Hormann_clipping_algorithm
    // - https://en.wikipedia.org/wiki/Weiler%E2%80%93Atherton_clipping_algorithm
    // - https://www.jasondavies.com/maps/clip/

    // set up output polygons
    Polygons outPolys = Polygons(new QVector<Polygon>());

#if 0
    // set up input polygons and ensure they are oriented clockwise
    Polygon subject_orient(isClockwise(subject) ? subject : reversed(subject));
    Q_ASSERT(isClockwise(subject_orient));
    Polygon clip_orient(isClockwise(clip) ? clip : reversed(clip));
    Q_ASSERT(isClockwise(clip_orient));
#else
    // set up input polygons regardless of orientation (seems to work fine ... but check both cases!)
    Polygon subject_orient(new QVector<Point>(*subject.data()));
    Polygon clip_orient(new QVector<Point>(*clip.data()));
#endif

    // deep-copy both polygons in order not to modify the originals (hm ... maybe do this right at the beginning instead?)
    const Polygon S(new QVector<Point>(*subject_orient.data()));
    const Polygon C(new QVector<Point>(*clip_orient.data()));

    // eliminate coincident neighbours in C (assuming for now that S doesn't have any)
    removeCoincidentNeighbours(C);

    // ensure that C is still large enough for an intersection to make sense
    if (C->size() < 3)
        return Polygons();

    // eliminate degenerate cases
    fixDegenerate(S, C);

    // find intersections
    int isctId = 0;
    QHash<int, QList<IsctInfo> > sIscts; // intersections for edges in S
    QHash<int, QList<IsctInfo> > cIscts; // intersections for edges in C
    for (int s = 0; s < S->size(); ++s) { // loop over vertices in S
        for (int c = 0; c < C->size(); ++c) { // loop over vertices in C
            Point isctPoint;
            if (greatCircleArcsIntersect(S->at(s), S->at((s + 1) % S->size()), C->at(c), C->at((c + 1) % C->size()), &isctPoint)) {
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
            if (pointInPolygon(S->at(i), C))
                sPointsInC++;

        if (sPointsInC == S->size()) {
            // the subject polygon is completely enclosed within the clip polygon, so return a list with one item:
            // a deep copy (although implicitly shared for efficiency) of the subject polygon
            Polygon sCopy(new QVector<Point>(*S.data()));
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
            if (pointInPolygon(C->at(i), S))
                cPointsInS++;

        if (cPointsInS == C->size()) {
            // the clip polygon is completely enclosed within the subject polygon, so return a list with one item:
            // a deep copy (although implicitly shared for efficiency) of the clip polygon
            Polygon cCopy(new QVector<Point>(*C.data()));
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
        bool entry = !pointInPolygon(slist.first().point_, C);

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
        bool entry = !pointInPolygon(clist.first().point_, S);

        QLinkedList<Node>::iterator it;
        for (it = clist.begin(); it != clist.end(); ++it) {
            if (it->isctId_ >= 0) {
                it->entry_ = entry;
                entry = !entry; // if this intersection was an entry, the next one must be an exit and vice versa
            }
        }
    }

    if (false) {
        static int nn = 0;
        printLists(QString::number(nn++), slist, clist);
    }


    // *** PHASE 3: Generate clipped polygons *********
    {

        // loop over original vertices and intersections in subject polygon
        QLinkedList<Node>::iterator sit;
        for (sit = slist.begin(); sit != slist.end(); ++sit) {
            if ((sit->isctId_ >= 0) && (!sit->visited_)) {
                // this is an unvisited intersection, so start tracing a new polygon
                Polygon poly(new QVector<Point>());

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
                        return Polygons();

                } while (it->isctId_ != sit->isctId_); // as long as tracing has not got back to where it started

                if (poly->size() >= 3) // hm ... wouldn't this always be the case?
                    outPolys->append(poly);
            }
        }
    }

    return outPolys;
}

QVector<Point> latitudeIntersections(const Point &p1, const Point &p2, double lat)
{
    // Adopted from 'Crossing parallels' on http://williams.best.vwh.net/avform.htm .
    // Alternative approach: http://math.stackexchange.com/questions/1157278/find-the-intersection-point-of-a-great-circle-arc-and-latitude-line .

    QVector<Point> points;

    const double lon1 = p1.first;
    const double lat1 = p1.second;
    const double lon2 = p2.first;
    const double lat2 = p2.second;

    const double l12 = lon1 - lon2;
    const double A = sin(lat1) * cos(lat2) * cos(lat) * sin(l12);
    const double B = sin(lat1) * cos(lat2) * cos(lat) * cos(l12) - cos(lat1) * sin(lat2) * cos(lat);
    const double C = cos(lat1) * cos(lat2) * sin(lat) * sin(l12);
    const double lon = atan2(B, A); // atan2(y, x) convention
    const double lenAB = sqrt(A * A + B * B);
    if (fabs(C) <= lenAB) {
        const double dlon = acos(C / lenAB);
        const double lon3 = fmod(lon1 + dlon + lon + M_PI, 2 * M_PI) - M_PI;
        const double lon4 = fmod(lon1 - dlon + lon + M_PI, 2 * M_PI) - M_PI;

        const double dist12 = Math::distance(p1, p2);

        const Point p3(lon3, lat);
        const double dist13 = Math::distance(p1, p3);
        const double dist23 = Math::distance(p2, p3);
        if ((dist13 < dist12) && (dist23 < dist12))
            points.append(p3);

        const Point p4(lon4, lat);
        const double dist14 = Math::distance(p1, p4);
        const double dist24 = Math::distance(p2, p4);
        if ((dist14 < dist12) && (dist24 < dist12)) {
            if ((!points.isEmpty()) && (dist14 < dist13))
                points.prepend(p4);
            else
                points.append(p4);
        }
    }

    return points;
}

Polygon latitudeArcPoints(double lat, double lon1, double lon2, bool eastwards, int nSegments, bool inclusive)
{
    Polygon points(new QVector<Point>());

//    Q_ASSERT((lon1 >= -M_PI) && (lon1 <= M_PI));
//    Q_ASSERT((lon2 >= -M_PI) && (lon2 <= M_PI));
    const double lonDist0 = (lon1 < lon2) ? (lon2 - lon1) : ((lon2 + 2 * M_PI) - lon1);
    const double lonDist = eastwards ? lonDist0 : (2 * M_PI - lonDist0);
    const double lonDelta = (lonDist / nSegments) * (eastwards ? 1 : -1);
    double lon = lon1 + (inclusive ? 0 : lonDelta);
    const int n = inclusive ? (nSegments + 1) : (nSegments - 1);

    for (int i = 0; i < n; ++i, lon += lonDelta)
        points->append(qMakePair(fmod(lon, 2 * M_PI), lat));

    return points;
}

Polygon reversed(const Polygon &polygon)
{
    Polygon copy(new QVector<Point>());
    for (int i = polygon->size() - 1; i >= 0; --i)
        copy->append(polygon->at(i));
    return copy;
}

bool isClockwise(const Polygon &polygon)
{
    // (Adopted from http://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order .
    // See also: https://en.wikipedia.org/wiki/Curve_orientation . )
    double sum = 0;
    for (int i = 0; i < polygon->size(); ++i) {
        const int next = (i + 1) % polygon->size();
        sum += (polygon->at(next).first - polygon->at(i).first) * (polygon->at(next).second + polygon->at(i).second);
    }
    return (sum > 0);
}

Polygon removeInvalidVertices(const Polygon &polygon, double minDegrees)
{
    const double minRadians = DEG2RAD(minDegrees);
    Polygon outPoly(new QVector<Point>());
    Polygon p1(new QVector<Point>(*polygon.data()));

    // loop over vertex scans until either 1) at least three vertices remain after a scan that found no invalid vertices, or 2) fewer than three vertices remain after a scan
    int k = 0;
    while (true) {

        Polygon p2(new QVector<Point>());
        int invalidIndex = -1; // index of first invalid vertex in this scan, or -1 if no invalid vertex has been found

        const int n = p1->size();
        // scan vertices, removing at most one invalid one
        for (int i = 0; i < n; ++i) {

            if (invalidIndex == -1) {
                // check if vertex i is the invalid vertex in this scan

                const int i1 = (i - 1 + n) % n; // index of vertex before i (circular)
                const int i2 = (i + 1) % n; // index of vertex after i (circular)
                //{
                //    bool validComp2 = true;
                //    qDebug() << "k:" << k << ", i1 i i2:" << i1 << i << i2 << ", angle:" << Math::angle(p1->at(i), p1->at(i1), p1->at(i2), &validComp2, true)
                //             << ", minRadians:" << minRadians << ", valid:" << validComp2;
                //}

                if ((p1->at(i) != p1->at(i1)) && (p1->at(i) != p1->at(i2))) {
                    bool validComp = true;
                    const double angle = Math::angle(p1->at(i), p1->at(i1), p1->at(i2), &validComp);
                    if (validComp) {
                        //qDebug() << "    valid computation, allowing angles to be compared";
                        if (angle < minRadians) {
                            //qDebug() << "    sharp angle!";
                            invalidIndex = i; // vertex i is at a sharp angle, so remove it
                        }
                    } else {
                        //qDebug() << "    invalid computation!";
                        invalidIndex = i; // vertex i was involved in an invalid computation, so remove it
                    }
                } else {
                    //qDebug() << "    coinciding!";
                    invalidIndex = i; // vertex i coincides with one of its neighbours, so remove it
                }
            }

            if (i != invalidIndex) {
                // vertex i is either 1) valid or 2) invalid, but not the first one in this scan, so keep it
                //qDebug() << "    keeping vertex" << i;
                p2->append(p1->at(i));
            } else {
                // vertex i is the first invalid vertex in this scan, so remove it (by not copying it to p2)
                //qDebug() << "    removing vertex" << i << "!";
            }
        }

        if (p2->size() < 3) {
            // too many invalid vertices found, so return an empty polygon
            //qDebug() << "    too many invalid vertices found; returning an empty polygon!";
            outPoly = Polygon();
            break;
        }

        if (p1->size() == p2->size()) {
            // no invalid vertices found in this scan, so return final result
            //qDebug() << "    no invalid vertices found in this scan; return final result ...";
            outPoly = p2;
            break;
        }

        // copy p2 to p1, empty p2, and iterate
        //qDebug() << "    invalid vertices found in this scan; iterating ...";
        p1 = Polygon(new QVector<Point>(*p2.data()));
        p2 = Polygon(new QVector<Point>());

        k++;
    }

    return outPoly;
}

Polygons removeInvalidVertices(const Polygons &polygons, double minDegrees)
{
    Polygons outPolys = Polygons(new QVector<Polygon>());
    for (int i = 0; i < polygons->size(); ++i)
        outPolys->append(removeInvalidVertices(polygons->at(i), minDegrees));
    return outPolys;
}

MGPMATH_END_NAMESPACE
MGP_END_NAMESPACE
