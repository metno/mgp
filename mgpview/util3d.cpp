#include <stdio.h> // 4 TESTING!
#include "util3d.h"
#include <stdexcept>

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
