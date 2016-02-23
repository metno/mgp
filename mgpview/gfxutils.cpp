#include "gfxutils.h"
#include "common.h"
#include "coast_data.h"
#include <GL/glut.h>

#include <stdio.h> // 4 TESTING!

const double GfxUtils::earth_radius_ = 6378000;

GfxUtils::GfxUtils()
{
    createCoast();
}

GfxUtils::~GfxUtils()
{
    delete[] points_;
    delete[] polys_;
}

void GfxUtils::drawAxes()
{
    glLineWidth(1);

    const float size = 1.1 * earth_radius_;
    glBegin(GL_LINES);
    // x axis
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(size, 0, 0);
    // y axis
    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, size, 0);
    // z axis
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, size);
    glEnd();
}

// NOTE: The following function is based on code found in the
// vtkEarthSource class!
//
void GfxUtils::createCoast()
{
    int on_ratio = 1;

    int	points_indx  = 0;

    int i;
    int maxPts;
    int maxPolys;
    double x[3], base[3];
    int Pts[4000];

    int npts, land, offset;
    int actualpts, actualpolys;
    double scale = 1/30000.0;

    //
    // Set things up; allocate memory
    //

//    maxPts = 12000 / on_ratio;
//    maxPts = 10611; // Hard-coded to actual number of points found on file!!!
    maxPts = 12000; // Hard-coded to (at least) the actual number of points
                    // found on the file!!!

    maxPolys = 30;
    actualpts = actualpolys = 0;

    points_ = new _3DPoint[maxPts];
    polys_ = new DynPoly[maxPolys];

    //
    // Create points
    //
    offset = 0;
    while (1)
    {
	// read a polygon
	npts = vtkEarthData[offset++];
	if ((npts == 0) || (actualpolys >= maxPolys)) break;
    
	land  = vtkEarthData[offset++];
    
	base[0] = 0;
	base[1] = 0;
	base[2] = 0;
    
//	fprintf(stderr, "npts = %d\n", npts);
	for (i = 1; i <= npts; i++)
	{
	    base[0] += vtkEarthData[offset++] * scale;
	    base[1] += vtkEarthData[offset++] * scale;
	    base[2] += vtkEarthData[offset++] * scale;
      
	    x[0] = base[2] * earth_radius_;
	    x[1] = base[0] * earth_radius_;
	    x[2] = base[1] * earth_radius_;
      
	    if ((land == 1) && (npts > on_ratio * 3))
	    {
		// use only every on_ratioth point in the polygon
		if ((i % on_ratio) == 0)
		{
		    points_[points_indx++].setPoint(x);
		    actualpts++;
		}
	    }
	}
    
	if ((land == 1) && (npts > on_ratio * 3))
	{
	    //
	    // Generate mesh connectivity for this polygon
	    //
      
	    actualpolys++;

	    polys_[actualpolys - 1].setSize(npts/on_ratio); 

	    for (i = 0; i < (npts/on_ratio); i++)
	    {
		Pts[i] = (actualpts - npts/on_ratio) + i;
		polys_[actualpolys - 1].setId(i, Pts[i]);
	    }

	}
    }

    n_polys_ = actualpolys - 1;

//    fprintf(stderr, "n_polys_ = %d\n", n_polys_);
}

void GfxUtils::drawCoastContours(_3DPoint* eye, double min_eye_dist, double max_eye_dist)
{
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(1.0);

    const double raise_fact =
	1 + computeRaise(eye, min_eye_dist, max_eye_dist) / earth_radius_;

    // Loop over polygons ...
    for (int p = 0; p < n_polys_; p++)
    {
	glBegin(GL_LINE_LOOP);
	// Loop over vertices ...
    for (int k = 0; k < polys_[p].getSize(); ++k)
	{
        int id = polys_[p].getId(k);
	    double v[3];
	    int i;
	    for (i = 0; i < 3; i++)
		v[i] = raise_fact * points_[id].getPoint()[i];
	    glVertex3dv(v);
	}
	glEnd();
    }
}

void GfxUtils::drawSurfacePolygon(
        const PointVector &points, _3DPoint* eye, double min_eye_dist, double max_eye_dist,
        const QColor &color, float lineWidth)
{
    glColor3f(color.redF(), color.greenF(), color.blueF());
    glLineWidth(lineWidth);

    const double raise_fact = 1 + computeRaise(eye, min_eye_dist, max_eye_dist) / earth_radius_;
    const double scale = raise_fact * earth_radius_;

    Q_ASSERT(points);

    glBegin(GL_LINE_LOOP);
    // loop over base points
    for (int i = 0; i < points->size(); ++i) {
        double x, y, z;

        // for a smoother curve (and to prevent it from intersecting the earth surface!),
        // draw extra points between this base point and the previous base point
        const int prevIndex = (i - 1 + points->size()) % points->size();
        const double maxDist = 0.01 * 2 * M_PI; // for now
        const double dist = Math::distance(points->at(i), points->at(prevIndex));
        if (dist > maxDist) {
            const int nSegments = static_cast<int>(ceil(dist / maxDist));
            const QVector<_3DPoint> extraPoints = Math::getGreatCirclePoints(points->at(prevIndex), points->at(i), nSegments);
            for (int j = 1; j < (extraPoints.size() - 1); ++j)
                glVertex3d(scale * extraPoints.at(j).x(), scale * extraPoints.at(j).y(), scale * extraPoints.at(j).z());
        }

        // draw base point
        Math::sphericalToCartesian(scale, points->at(i).second, points->at(i).first, x, y, z);
        glVertex3d(x, y, z);
    }
    glEnd();
}

void GfxUtils::drawSphere(double x, double y, double z, double radius, float r, float g, float b, float amb, int phi_res, int theta_res, GLenum shade_model)
{
    GLfloat mat_diffuse[] = {r, g, b, 1.0};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

    GLfloat mat_ambient[] = {amb, amb, amb, 1.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT,  mat_ambient);

//    GLfloat mat_specular[] = {0.0, 0.0, 0.0, 1.0};
//    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);

//    GLfloat mat_shininess[] = {100.0};
//    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glPushMatrix();
    glTranslated(x, y, z);

    glEnable(GL_LIGHTING);

    glShadeModel(shade_model);
    glColor3f(r, g, b);

    glutSolidSphere(radius, theta_res, phi_res);

    glPopMatrix();

    glShadeModel(GL_SMOOTH); // Assume this is the default

    glDisable(GL_LIGHTING);
}

void GfxUtils::drawSurfaceBall(double lon, double lat, double ballSize, float r, float g, float b, float amb, int phiRes, int thetaRes, GLenum shadeModel)
{
    const double
            rad = earth_radius_,
            x = rad * cos(lat) * cos(lon),
            y = rad * cos(lat) * sin(lon),
            z = rad * sin(lat);
    drawSphere(x, y, z, ballSize, r, g, b, amb, phiRes, thetaRes, shadeModel);
}

void GfxUtils::drawLine(double x0, double y0, double z0, double x1, double y1, double z1, double scale_fact, float r, float g, float b, double width)
{
    glColor3f(r, g, b);
    glLineWidth(width);
    glBegin(GL_LINES);
    glVertex3d(x0, y0, z0);
    _4DPoint p(x1, y1, z1);
    p.scale(scale_fact);
    glVertex3d(x0 + p.get(0), y0 + p.get(1), z0 + p.get(2));
    glEnd();
}

void GfxUtils::drawCone(double x0, double y0, double z0, double x1, double y1, double z1, double base, double length, float r, float g, float b, float amb, bool reverse)
{
    GLfloat mat_diffuse[] = {r, g, b, 1.0};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);

    GLfloat mat_ambient[] = {amb, amb, amb, 1.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT,  mat_ambient);

    glPushMatrix();
    glTranslated(x0, y0, z0);

    double lat, lon;
	Math::computeLatLon(x1, y1, z1, lat, lon);
	glRotated((lon / M_PI) * 180, 0, 0, 1);
	glRotated(((M_PI_2 - lat) / M_PI) * 180, 0, 1, 0);
	if (reverse)
	{
	    glTranslated(0, 0, length);
	    glRotated(180, 1, 0, 0);
	}

    glEnable(GL_LIGHTING);

    glColor3f(r, g, b);

    if (reverse)
    {
	glDisable(GL_CULL_FACE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    }

    glutSolidCone(base, length, 16, 1);

    if (reverse)
    {
	glEnable(GL_CULL_FACE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	GLfloat mat_amb_diff[] = {r, g, b, 1};
	glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff);
    }

    glPopMatrix();

    glDisable(GL_LIGHTING);
}

void GfxUtils::drawCameraSphere(double x, double y, double z, double radius, float r, float g, float b)
{
    glPushMatrix();
    glTranslated(x, y, z);

    glEnable(GL_LIGHTING);

    glColor3f(r, g, b);
    glutWireSphere(radius, 7, 7);

    glDisable(GL_LIGHTING);

    glPopMatrix();
}

void GfxUtils::drawBaseCircle(double radius, float r, float g, float b, float lineWidth, double thetaBegin, double thetaEnd)
{
    const int res = 128;
    const double deltaTheta = (2 * M_PI) / res;
    double theta = thetaBegin;
    glColor3f(r, g, b);
    glLineWidth(lineWidth);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= res; i++, theta += deltaTheta) {
        glVertex3d(radius * cos(theta), radius * sin(theta), 0);
        if (theta > thetaEnd)
            break;
    }
    glEnd();
}

double GfxUtils::computeRaise(_3DPoint* eye, double min_eye_dist, double max_eye_dist)
{
    const double eye_dist = sqrt(
	eye->x() * eye->x() +
	eye->y() * eye->y() +
	eye->z() * eye->z()) - earth_radius_;

    const double
	min_raise =  20000, // Must be tuned!
	max_raise = 250000; // ------''------

    return
	min_raise +
	((eye_dist - min_eye_dist) / (max_eye_dist - min_eye_dist)) *
	(max_raise - min_raise);
}

void GfxUtils::drawLatLonCircles(_3DPoint* eye, double min_eye_dist, double max_eye_dist)
{
    int i;
    const double
            c1[3] = {1.0, 0.8, 0.8},
            c2[3] = {0.6, 0.6, 0.6};

    double raise = computeRaise(eye, min_eye_dist, max_eye_dist);

    // Draw horizontal (and parallel) circles of constant latitude ...
    // 1) Equator:
    drawBaseCircle(earth_radius_ + raise, c1[0], c1[1], c1[2]);
    // 2) Circles above and below equator ...
    const double
            delta_theta = (10.0 / 180.0) * M_PI;
    for (i = 1; i < 9; i++)
    {
        double theta = i * delta_theta;
        double radius = cos(theta) * (earth_radius_ + raise);
        double z = sin(theta) * (earth_radius_ + raise);

        glPushMatrix();
        glTranslated(0, 0, z);
        drawBaseCircle(radius, c2[0], c2[1], c2[2]);
        glPopMatrix();
        //
        glPushMatrix();
        glTranslated(0, 0, -z);
        drawBaseCircle(radius, c2[0], c2[1], c2[2]);
        glPopMatrix();
    }

    // Draw vertical circles of constant longitude (a.k.a. meridians) ...
    // 1) Greenwich meridian ...
    glPushMatrix();
    glRotated(90, 1, 0, 0);
    drawBaseCircle(earth_radius_ + raise, c1[0], c1[1], c1[2]);
    glPopMatrix();
    // 2) Pacific meridian ...
    glPushMatrix();
    glRotated(90, 0, 1, 0);
    drawBaseCircle(earth_radius_ + raise, c1[0], c1[1], c1[2]);
    glPopMatrix();

    // 3) Circles east and west of Greenwich ...
    for (i = 1; i < 9; i++)
    {
        glPushMatrix();
        glRotated(i * 10, 0, 0, 1);
        glRotated(90, 1, 0, 0);
        drawBaseCircle(earth_radius_ + raise, c2[0], c2[1], c2[2]);
        glPopMatrix();
        //
        glPushMatrix();
        glRotated(-i * 10, 0, 0, 1);
        glRotated(90, 1, 0, 0);
        drawBaseCircle(earth_radius_ + raise, c2[0], c2[1], c2[2]);
        glPopMatrix();
    }
}

void GfxUtils::drawLatCircle(
        _3DPoint* eye, double min_eye_dist, double max_eye_dist, double lat, const QColor &color, float lineWidth)
{
    const double raise = computeRaise(eye, min_eye_dist, max_eye_dist);
    const double theta = (lat / 90) * (M_PI / 2);
    const double radius = cos(theta) * (earth_radius_ + raise);
    const double z = sin(theta) * (earth_radius_ + raise);
    const float r = color.redF();
    const float g = color.greenF();
    const float b = color.blueF();

    glPushMatrix();
    glTranslated(0, 0, z);
    drawBaseCircle(radius, r, g, b, lineWidth);
    glPopMatrix();
}

void GfxUtils::drawLonCircle(
        _3DPoint* eye, double min_eye_dist, double max_eye_dist, double lon, const QColor &color, const float lineWidth)
{
    const double raise = computeRaise(eye, min_eye_dist, max_eye_dist);
    const float r = color.redF();
    const float g = color.greenF();
    const float b = color.blueF();

    glPushMatrix();
    glRotated(lon, 0, 0, 1);
    glRotated(90, 1, 0, 0);
    drawBaseCircle(earth_radius_ + raise, r, g, b, lineWidth, -M_PI / 2, M_PI / 2);
    glPopMatrix();
}

void GfxUtils::drawLonOrLatCircle(
        bool lon, _3DPoint* eye, double min_eye_dist, double max_eye_dist, double val, const QColor &color, float lineWidth)
{
    if (lon)
        drawLonCircle(eye, min_eye_dist, max_eye_dist, val, color, lineWidth);
    else
        drawLatCircle(eye, min_eye_dist, max_eye_dist, val, color, lineWidth);
}

// Draws the great circle between surface positions line.p1() and line.p2(). If segmentOnly is true, only the part of the circle between
// the two endpoints is drawn.
void GfxUtils::drawGreatCircle(
        _3DPoint* eye, double min_eye_dist, double max_eye_dist, const QLineF &line, const QColor &color, float lineWidth, bool segmentOnly)
{
    const double raise_fact = 1 + computeRaise(eye, min_eye_dist, max_eye_dist) / earth_radius_;
    const double scale = raise_fact * earth_radius_;
    const float r = color.redF();
    const float g = color.greenF();
    const float b = color.blueF();

    glColor3f(r, g, b);
    glLineWidth(lineWidth);
    const int res = 64;
    const double lon1 = DEG2RAD(line.p1().x());
    const double lat1 = DEG2RAD(line.p1().y());
    const double lon2 = DEG2RAD(line.p2().x());
    const double lat2 = DEG2RAD(line.p2().y());
    const QVector<_3DPoint> points = Math::getGreatCirclePoints(qMakePair(lon1, lat1), qMakePair(lon2, lat2), res + 1, segmentOnly);

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < points.size(); ++i)
        glVertex3d(scale * points.at(i).x(), scale * points.at(i).y(), scale * points.at(i).z());
    glEnd();
}

void GfxUtils::drawBottomString(const QString &s, int win_width, int win_height, int row, int col, const QColor &textColor, const QColor &bgColor, bool alignLeft)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); // Save projection matrix

    glLoadIdentity();
    gluOrtho2D(0, win_width, 0, win_height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); // Save model-view matrix

    glLoadIdentity();

    const int // (values in pixels)
            dx       =  8,
            dy       = 13,
            offset_x = 0,
            offset_y =  0,
            inner_pad_y =  0,
            outer_pad = 4;

    glDisable(GL_DEPTH_TEST);

    // draw background
    const int
            x0 = alignLeft ? (offset_x + col * dx) : (win_width - offset_x - 2 * outer_pad - (col + s.size()) * dx),
            y0 = offset_y + row * (dy + inner_pad_y),
            x1 = x0 + s.size() * dx + 2 * outer_pad,
            y1 = y0 + dy + 2 * outer_pad;
    glColor3f(bgColor.redF(), bgColor.greenF(), bgColor.blueF());
    glBegin(GL_QUADS);
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
    glEnd();

    // draw string
    glColor3f(textColor.redF(), textColor.greenF(), textColor.blueF()); // note: glColor*() must be called before glRasterPos*()!
    glRasterPos2f(
                alignLeft ? (offset_x + outer_pad + col * dx) : (win_width - offset_x - outer_pad - (col + s.size()) * dx),
                offset_y + outer_pad + row * (dy + inner_pad_y));
    for (int i = 0; i < s.size(); i++)
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, s.toLatin1()[i]);

    glEnable(GL_DEPTH_TEST);

    glPopMatrix(); // Restore model-view matrix

    glMatrixMode(GL_PROJECTION);
    glPopMatrix(); // Restore projection matrix
}
