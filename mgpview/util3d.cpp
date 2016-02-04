#include <stdio.h> // 4 TESTING!
#include "util3d.h"
#include <stdexcept>
#include <QDebug>

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

// Returns the spherical distance (i.e. along the great circle on the unit sphere) between two points.
double Math::distance(double lon1, double lat1, double lon2, double lat2)
{
    const double theta1 = (lon1 / 180) *  M_PI;
    const double phi1   = (lat1 /  90) * (M_PI / 2);
    const double theta2 = (lon2 / 180) *  M_PI;
    const double phi2   = (lat2 /  90) * (M_PI / 2);

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
 * Returns the distance along the great circle from one point to another (using haversine formula).
 *
 * @param   lon0, lat0 - Source point.
 * @param   lon1, lat1 - Destination point.
 * @param   radius - Sphere radius.
 * @returns Distance between the source and destination point, in same units as radius.
 */
double Math::distanceBetween(double lon0, double lat0, double lon1, double lat1, double radius)
{
    const double phi_1 = DEG2RAD(lat0);
    const double lambda_1 = DEG2RAD(lon0);
    const double phi_2 = DEG2RAD(lat1);
    const double lambda_2 = DEG2RAD(lon1);
    const double delta_phi = phi_2 - phi_1;
    const double delta_lambda = lambda_2 - lambda_1;

    const double a =
            sin(delta_phi / 2) * sin(delta_phi / 2) +
            cos(phi_1) * cos(phi_2) *
            sin(delta_lambda / 2) * sin(delta_lambda / 2);
    const double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return radius * c;
};

/**
 * Returns the (initial) bearing from one point to another.
 *
 * @param   lon0, lat0 - Source point.
 * @param   lon1, lat1 - Destination point.
 * @returns Initial bearing in degrees from north.
 */
double Math::bearingBetween(double lon0, double lat0, double lon1, double lat1)
{
    const double phi_1 = DEG2RAD(lat0);
    const double phi_2 = DEG2RAD(lat1);
    const double delta_lambda = DEG2RAD(lon1 - lon0);

    // see http://mathforum.org/library/drmath/view/55417.html
    const double y = sin(delta_lambda) * cos(phi_2);
    const double x = cos(phi_1) * sin(phi_2) - sin(phi_1) * cos(phi_2) * cos(delta_lambda);
    const double theta = atan2(y, x);

    return fmod((RAD2DEG(theta) + 360), 360);
};

/**
 * Returns (signed) distance from a point to a great circle.
 *
 * @param   lon0, lat0 - Source point.
 * @param   lon1, lat1 - Start point of great circle path.
 * @param   lon2, lat2 - End point of great circle path.
 * @param   radius - Sphere radius.
 * @returns Distance to great circle (< 0 if to left, > 0 if to right of path).
 */
double Math::crossTrackDistanceToGreatCircle(double lon0, double lat0, double lon1, double lat1, double lon2, double lat2, double radius)
{
    const double delta_13 = distanceBetween(lon1, lat1, lon0, lat0, radius) / radius;
    const double theta_13 = DEG2RAD(bearingBetween(lon1, lat1, lon0, lat0));
    const double theta_12 = DEG2RAD(bearingBetween(lon1, lat1, lon2, lat2));

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

        const double dist11 = distance(RAD2DEG(lon1), RAD2DEG(lat1), RAD2DEG(lon3_1), RAD2DEG(lat));
        const double dist21 = distance(RAD2DEG(lon2), RAD2DEG(lat2), RAD2DEG(lon3_1), RAD2DEG(lat));
        const double dist12 = distance(RAD2DEG(lon1), RAD2DEG(lat1), RAD2DEG(lon3_2), RAD2DEG(lat));
        const double dist22 = distance(RAD2DEG(lon2), RAD2DEG(lat2), RAD2DEG(lon3_2), RAD2DEG(lat));
        isctPoint->first = ((dist11 + dist21) < (dist12 + dist22)) ? lon3_1 : lon3_2;

        isctPoint->second = lat;

        return true;
    }

    return false; // no crossing
}

// Returns true iff the great circle arc from p1 to p2 intersects the great circle arc from (lon1, lat1) to (lon2, lat2).
// The intersection point closest to p1 and p2 is returned in isctPoint.
// NOTE: If the two great circles lie (approximately) in the same plane, the function returns false (even if there are infinite numbers
// of intersections!).
// Adopted from http://www.mathworks.com/matlabcentral/newsreader/view_thread/276271 .
// Alternative approaches:
//   1: http://stackoverflow.com/questions/2954337/great-circle-rhumb-line-intersection
//   2: http://www.boeing-727.com/Data/fly%20odds/distance.html
//   3: http://www.movable-type.co.uk/scripts/latlong-vectors.html
bool Math::greatCircleArcsIntersect(
        const QPair<double, double> &p1, const QPair<double, double> &p2, double lon1, double lat1, double lon2, double lat2,
        QPair<double, double> *isctPoint)
{
    const _3DPoint a0 = _3DPoint::fromSpherical(p1.first, p1.second);
    const _3DPoint a1 = _3DPoint::fromSpherical(p2.first, p2.second);
    const _3DPoint b0 = _3DPoint::fromSpherical(lon1, lat1);
    const _3DPoint b1 = _3DPoint::fromSpherical(lon2, lat2);

    const _3DPoint p = _3DPoint::cross(a0, a1); // normal of plane 1
    const _3DPoint q = _3DPoint::cross(b0, b1); // normal of plane 2

    const _3DPoint t = _3DPoint::cross(p, q);
    if (t.norm() < 0.00001)
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

    isctPoint->first = atan2(sign * t.y(), sign * t.x());
    isctPoint->second = atan2(sign * t.z(), sqrt(t.x() * t.x() + t.y() * t.y()));

    return true;
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
