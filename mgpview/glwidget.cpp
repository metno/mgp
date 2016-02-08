#include "glwidget.h"
#include "util3d.h"
#include "gfxutils.h"
#include "mainwindow.h"
#include "common.h"
#include <GL/glut.h>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDoubleSpinBox>
#include <QDialog>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QColor>

#include <stdio.h> // 4 TESTING!

LonOrLatFilterInfo::LonOrLatFilterInfo(Filter::Type type_, bool isLon_)
    : type(type_)
    , isLon(isLon_)
{
}

FreeLineFilterInfo::FreeLineFilterInfo(Filter::Type type_)
    : type(type_)
{
}

const double
   GLWidget::minDolly_ =
      0.05 * GfxUtils::getEarthRadius(),
   GLWidget::maxDolly_ =
      5    * GfxUtils::getEarthRadius();

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent)
    , dolly_(0.7)
    , heading_(0.5)
    , incl_(1)
    , focusLon_(0.013) // London
    , focusLat_(0.893)
    , focus_alt_(0)     // Surface
    , mouseLon_(0)
    , mouseLat_(0)
    , mouseHitsEarth_(false)
    , draggingFilter_(false)
    , draggingCustomBasePolygonPoint_(false)
    , draggingFocus_(false)
    , minBallSize_(0.001 * GfxUtils::getEarthRadius())
    , maxBallSize_(0.02 * GfxUtils::getEarthRadius())
    , currCustomBasePolygonPoint_(-1)
{
    // --- BEGIN initialize filter infos -------------------

    // LonOrLat filters

    lonOrLatFilterInfos_.insert(0, new LonOrLatFilterInfo(Filter::E_OF, true));
    lonOrLatFilterInfos_.insert(1, new LonOrLatFilterInfo(Filter::W_OF, true));
    lonOrLatFilterInfos_.insert(2, new LonOrLatFilterInfo(Filter::N_OF, false));
    lonOrLatFilterInfos_.insert(3, new LonOrLatFilterInfo(Filter::S_OF, false));


    // FreeLine filters
    freeLineFilterInfos_.insert(0, new FreeLineFilterInfo(Filter::NE_OF));
    freeLineFilterInfos_.insert(1, new FreeLineFilterInfo(Filter::NW_OF));
    freeLineFilterInfos_.insert(2, new FreeLineFilterInfo(Filter::SE_OF));
    freeLineFilterInfos_.insert(3, new FreeLineFilterInfo(Filter::SW_OF));

    // --- END initialize filter infos -------------------


    setMouseTracking(true);

    setFocusPolicy(Qt::StrongFocus);
    focus_.setPoint(GfxUtils::getEarthRadius(), 0, 0);

    addCustomBasePolygonPointAction_ = new QAction("Add point", 0);
    connect(addCustomBasePolygonPointAction_, SIGNAL(triggered()), SLOT(addCustomBasePolygonPoint()));

    removeCustomBasePolygonPointAction_ = new QAction("Remove point", 0);
    connect(removeCustomBasePolygonPointAction_, SIGNAL(triggered()), SLOT(removeCustomBasePolygonPoint()));
}


void GLWidget::initializeGL()
{
    glClearColor(0, 0, 0, 0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
}


void GLWidget::resizeGL(int, int)
{
    updateGL();
}


void GLWidget::paintGL()
{
    GfxUtils& gfx_util = GfxUtils::instance();

    // set viewportBasePolygon currentBasePolygon() const;

    glViewport(0, 0, width(), height());

    // set projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(
	30, (double)width() / height(), 0.01 * gfx_util.getEarthRadius(),
	10 * gfx_util.getEarthRadius());

    // set light and camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //
    CartesianKeyFrame ckf = computeCamera();
    _3DPoint
	* eye = ckf.getEye(),
	* tgt = ckf.getTgt(),
	* up  = ckf.getUp();
    //
    GLfloat light_pos[] = {0, 0, 1, 1};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    //
    gluLookAt(eye->x(),            eye->y(),            eye->z(),
              eye->x() + tgt->x(), eye->y() + tgt->y(), eye->z() + tgt->z(),
              up->x(),             up->y(),             up->z());

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw earth base sphere
    gfx_util.drawSphere(
    0, 0, 0, gfx_util.getEarthRadius(), 0.6, 0.6, 0.6, 0.7, 36, 72,
    GL_SMOOTH);

    // draw coast contours
    glShadeModel(GL_FLAT);
    gfx_util.drawCoastContours(eye, minDolly_, maxDolly_);
    glShadeModel(GL_SMOOTH);


    // draw lat/lon circles
    glShadeModel(GL_FLAT);
    gfx_util.drawLatLonCircles(eye, minDolly_, maxDolly_);
    glShadeModel(GL_SMOOTH);

    // draw mouse point
//    gfx_util.drawSurfaceBall(mouseLon_, mouseLat_, ballSize(), 0.7, 0.6, 0.4, 0.8);


    // --- BEGIN draw base polygon --------------------------------

    if (ControlPanel::instance().currentBasePolygonPoints()) {
        const PointVector points = ControlPanel::instance().currentBasePolygonPoints();

        if (ControlPanel::instance().basePolygonVisible()) {
            glShadeModel(GL_FLAT);
            gfx_util.drawSurfacePolygon(points, eye, minDolly_, maxDolly_, QColor::fromRgbF(0, 0, 1), 2);
            glShadeModel(GL_SMOOTH);
        }

        if ((ControlPanel::instance().currentBasePolygonType() == BasePolygon::Custom) && ControlPanel::instance().customBasePolygonEditableOnSphere()) {

            // draw points and any intersections
            for (int i = 0; i < points->size(); ++i) {

                double r, g, b;
                if (i == currCustomBasePolygonPoint_) {
                    r = 1.0;
                    g = 1.0;
                    b = 0.0;
                } else if (ControlPanel::instance().rejectedByAnyFilter(points->at(i).first, points->at(i).second)) {
                    r = 0.8;
                    g = 0.0;
                    b = 0.0;
                } else {
                    r = 0.0;
                    g = 0.8;
                    b = 0.8;
                }

                // draw point
                gfx_util.drawSurfaceBall(points->at(i).first, points->at(i).second, ballSize(), r, g, b, 1);

                // draw points where filters intersect the great circle segment between this point and the previous one
                const int prevIndex = (i - 1 + points->size()) % points->size();
                const QVector<QPair<double, double> > isctPoints = ControlPanel::instance().filterIntersections(points->at(prevIndex), points->at(i));
                for (int j = 0; j < isctPoints.size(); ++j)
                    gfx_util.drawSurfaceBall(isctPoints.at(j).first, isctPoints.at(j).second, ballSize(), 1, 0, 1, 1);
            }
        }
    }

    // --- END draw base polygon --------------------------------


    // --- BEGIN draw filters --------------------------------

    const QColor normalColor = QColor::fromRgbF(1, 1, 1);
    const QColor currColor = QColor::fromRgbF(1, 1, 1);

    const QColor normalValidColor = QColor::fromRgbF(1, 1, 1);
    const QColor normalInvalidColor = QColor::fromRgbF(0.9, 0, 0);
    const QColor currValidColor = QColor::fromRgbF(1, 1, 1);
    const QColor currInvalidColor = QColor::fromRgbF(1, 0, 0);

    const float normalLineWidth = 1.5;
    const float currLineWidth = 5;

    // LonOrLat filters
    for (int i = 0; i < 4; ++i) {
        const LonOrLatFilterInfo *finfo = lonOrLatFilterInfos_.value(i);
        const bool curr = ControlPanel::instance().isCurrent(finfo->type);
        const QColor color = curr ? currColor : normalColor;
        const float lineWidth = curr ? currLineWidth : normalLineWidth;

        if (ControlPanel::instance().isEnabled(finfo->type)) {
            bool ok = false;
            gfx_util.drawLonOrLatCircle(
                        finfo->isLon, eye, minDolly_, maxDolly_, ControlPanel::instance().value(finfo->type).toDouble(&ok),
                        color, lineWidth);
            Q_ASSERT(ok);
        }
    }

    // FreeLine filters
    for (int i = 0; i < 4; ++i) {
        const FreeLineFilterInfo *finfo = freeLineFilterInfos_.value(i);
        const bool curr = ControlPanel::instance().isCurrent(finfo->type);
        const bool valid = ControlPanel::instance().isValid(finfo->type);
        const QColor color = curr ? (valid ? currValidColor : currInvalidColor) : (valid ? normalValidColor : normalInvalidColor);
        const float lineWidth = curr ? currLineWidth : normalLineWidth;

        if (ControlPanel::instance().isEnabled(finfo->type)) {
            const QLineF line = ControlPanel::instance().value(finfo->type).toLineF();
            gfx_util.drawGreatCircleSegment(eye, minDolly_, maxDolly_, line, color, lineWidth);
            if (ControlPanel::instance().isCurrent(finfo->type)) {
                gfx_util.drawSurfaceBall(DEG2RAD(line.p1().x()), DEG2RAD(line.p1().y()), ballSize(), 1.0, 1.0, 1.0, 0.8);
                gfx_util.drawSurfaceBall(DEG2RAD(line.p2().x()), DEG2RAD(line.p2().y()), ballSize(), 1.0, 1.0, 1.0, 0.8);
            }
        }
    }

    // --- END draw filters --------------------------------


    // --- BEGIN draw result polygons --------------------------------

    if (ControlPanel::instance().resultPolygonsLinesVisible() || ControlPanel::instance().resultPolygonsPointsVisible()) {
        const PointVectors polygons = ControlPanel::instance().resultPolygons();

        if (ControlPanel::instance().resultPolygonsLinesVisible()) {
            // draw lines
            glShadeModel(GL_FLAT);
            for (int i = 0; i < polygons->size(); ++i)
                if (polygons->at(i) && (!polygons->at(i)->isEmpty()))
                    gfx_util.drawSurfacePolygon(polygons->at(i), eye, minDolly_, maxDolly_ * 0.9, QColor::fromRgbF(1, 1, 0), 4);
            glShadeModel(GL_SMOOTH);
        }

        if (ControlPanel::instance().resultPolygonsPointsVisible()) {
            // draw points
            for (int i = 0; i < polygons->size(); ++i) {
                for (int j = 0; j < polygons->at(i)->size(); ++j) {
                    gfx_util.drawSurfaceBall(polygons->at(i)->at(j).first, polygons->at(i)->at(j).second, ballSize(), 1, 0.5, 0, 1);
                }
            }
        }
    }

    // --- END draw result polygons --------------------------------


    // show mouse position
    {
        if (mouseHitsEarth_) {
            QString s;
            s.sprintf("lon = %.3f, lat = %.3f", RAD2DEG(mouseLon_), RAD2DEG(mouseLat_));
            gfx_util.drawBottomString(s.toLatin1(), width(), height(), 0, 0, QColor::fromRgbF(1, 1, 0), QColor::fromRgbF(0, 0, 0));
        }
    }

    // show test label
    //gfx_util.drawBottomString("test...", width(), height(), 0, 0, QColor::fromRgbF(1, 1, 1), QColor::fromRgbF(0, 0.3, 0), false);

    glFlush();
}

void GLWidget::computeRay(int x, int y, _4DPoint &eye, _4DPoint &ray)
{
    // Transform window coordinates into world coordinates ...
    GLint viewport[4];
    GLdouble mvmatrix[16], projmatrix[16];
    GLint real_y;
    GLdouble wx, wy, wz;
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
    real_y = viewport[3] - (GLint)y;
    gluUnProject((GLdouble)x, (GLdouble)real_y, 0.0, mvmatrix, projmatrix, viewport, &wx, &wy, &wz);

    // Compute ray from eye through the pixel ...
    CartesianKeyFrame ckf = computeCamera();
    _3DPoint *eye2 = ckf.getEye();
    ray.set(wx - eye2->x(), wy - eye2->y(), wz - eye2->z());
    ray.normalize();
    eye.set(eye2->x(), eye2->y(), eye2->z());
}

bool GLWidget::intersectsEarth(QMouseEvent *event, double &lon, double &lat)
{
    _4DPoint eye, ray;
    computeRay(event->x(), event->y(), eye, ray);

    double wx, wy, wz;
    if (Math::raySphereIntersect(
                eye.x(), eye.y(), eye.z(),
                ray.x(), ray.y(), ray.z(),
                0, 0, 0,
                GfxUtils::instance().getEarthRadius(),
                wx, wy, wz)) {
        Math::computeLatLon(wx, wy, wz, lat, lon);
        lon = fmod(lon + M_PI, 2 * M_PI) - M_PI; // [0, 2PI] -> [-PI, PI]
        return true;
    }

    return false;
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QMenu contextMenu;
        if ((ControlPanel::instance().currentBasePolygonType() == BasePolygon::Custom)
                && ControlPanel::instance().customBasePolygonEditableOnSphere()
                && (currCustomBasePolygonPoint_ >= 0)) {
            contextMenu.addAction(addCustomBasePolygonPointAction_);
            contextMenu.addAction(removeCustomBasePolygonPointAction_);
            removeCustomBasePolygonPointAction_->setEnabled(ControlPanel::instance().currentBasePolygonPoints()->size() > 3);
        }

        contextMenu.exec(QCursor::pos());

    } else if (mouseHitsEarth_) {

        // start dragging ...
        dragBaseX_ = event->x();
        dragBaseY_ = event->y();
        if ((event->button() == Qt::LeftButton)
                && ControlPanel::instance().filtersEditableOnSphere()
                && ControlPanel::instance().startFilterDragging(mouseLon_, mouseLat_)) {
            // ... filter
            draggingFilter_ = true;
            dragBaseLon_ = mouseLon_;
            dragBaseLat_ = mouseLat_;
        } else if ((event->button() == Qt::LeftButton)
                   && ControlPanel::instance().customBasePolygonEditableOnSphere()
                   && (currCustomBasePolygonPoint_ >= 0)) {
            // ... custom base polygon
            draggingCustomBasePolygonPoint_ = true;
            dragBaseLon_ = mouseLon_;
            dragBaseLat_ = mouseLat_;
        } else if ((event->button() == Qt::LeftButton) || (event->button() == Qt::MiddleButton)) {
            // ... camera (i.e. lat/lon focus)
            draggingFocus_ = true;
            dragBaseLon_ = focusLon_;
            dragBaseLat_ = focusLat_;
        }

        updateGL();
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *)
{
    draggingFilter_ = draggingCustomBasePolygonPoint_ = draggingFocus_ = false;
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(mouseHitsEarth_ = intersectsEarth(event, mouseLon_, mouseLat_))) {
        updateGL();
        return;
    }

    updateCurrCustomBasePolygonPoint();

    if (draggingFilter_) {
        ControlPanel::instance().updateFilterDragging(mouseLon_, mouseLat_);

    } else if (draggingCustomBasePolygonPoint_) {
        ControlPanel::instance().updateCustomBasePolygonPointDragging(currCustomBasePolygonPoint_, mouseLon_, mouseLat_);

    } else if (draggingFocus_) {
        const int dx = event->x() - dragBaseX_;
        const int dy = event->y() - dragBaseY_;
        const double minScale = 1.0 / 50000.0; // suitable for max zoom in (dolly_ == 0.0)
        const double maxScale = 1.0 / 1000.0; // suitable for max zoom out (dolly_ == 1.0)

        //const double scale = minScale + dolly_ * (maxScale - minScale); // linear scaling
        //const double scale = minScale + dolly_ * dolly_ * (maxScale - minScale); // non-linear scaling 1
        const double scale = minScale + dolly_ * (2 - dolly_) * 0.5 * (maxScale - minScale); // non-linear scaling 2

        const double deltaLon = -dx * scale * M_PI;
        const double deltaLat = dy * scale * M_PI;
        double newLon = dragBaseLon_ + deltaLon;
        if (newLon < -M_PI)
            newLon += 2 * M_PI;
        else if (newLon > M_PI)
            newLon -= 2 * M_PI;
        const double newLat = dragBaseLat_ + deltaLat;

        setCurrentFocusPos(newLon, newLat);
        emit focusPosChanged();
    }

    updateGL();
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    MainWindow::instance().handleKeyPressEvent(event);
    if (currCustomBasePolygonPoint_ >= 0) {
        if (event->key() == Qt::Key_Insert)
            addCustomBasePolygonPoint();
        else if (event->key() == Qt::Key_Delete)
            removeCustomBasePolygonPoint();
    }
}

void GLWidget::enterEvent(QEvent *)
{
    updateGL();
}

void GLWidget::leaveEvent(QEvent *)
{
    mouseHitsEarth_ = false;
    updateGL();
}

void GLWidget::addCustomBasePolygonPoint()
{
    Q_ASSERT(currCustomBasePolygonPoint_ >= 0);
    ControlPanel::instance().addPointToCustomBasePolygon(currCustomBasePolygonPoint_);
}

void GLWidget::removeCustomBasePolygonPoint()
{
    Q_ASSERT(currCustomBasePolygonPoint_ >= 0);
    ControlPanel::instance().removePointFromCustomBasePolygon(currCustomBasePolygonPoint_);
}

CartesianKeyFrame GLWidget::computeCamera()
{
    const double
            radius = focus_alt_ + GfxUtils::instance().getEarthRadius(),
            focus_x = radius * cos(focusLat_) * cos(focusLon_),
            focus_y = radius * cos(focusLat_) * sin(focusLon_),
            focus_z = radius * sin(focusLat_);
    setCameraFocus(focus_x, focus_y, focus_z, false);

    // Compute matrix for rotating x-axis according to inclination and
    // heading ...
    _4x4Matrix m_rot1, m_tmp;
    m_rot1.loadRotateY((1 - incl_) * M_PI_2);
    m_tmp.loadRotateX(fmod((heading_ + 0.5) * 2 * M_PI, 2 * M_PI));
    m_rot1.mulMatLeft(m_tmp);

    // Compute matrix for rotating the x unit vector into the focus vector ...
    double lat, lon;
    Math::computeLatLon(focus_.x(), focus_.y(), focus_.z(), lat, lon);
    //
    _4x4Matrix m_rot2;
    m_rot2.loadRotateY(-lat);
    m_tmp.loadRotateZ(lon);
    m_rot2.mulMatLeft(m_tmp);

    // Set matrix for translating the origin into the focus point ...
    _4x4Matrix m_tsl;
    m_tsl.loadTranslate(
    Math::norm(focus_.x(), focus_.y(), focus_.z()), 0, 0);

    // Compute eye-point ...
    _4x4Matrix m_eye = m_rot1;
    m_eye.mulMatLeft(m_tsl);
    m_eye.mulMatLeft(m_rot2);
    _4DPoint eye(
    minDolly_ + pow(dolly_, 2) * (maxDolly_ - minDolly_), 0, 0);
    eye.mulMatPoint(m_eye);

    // Compute up-vector ...
    _4x4Matrix m_up = m_rot1;
    m_up.mulMatLeft(m_rot2);
    _4DPoint up(0, 0, 1);
    up.mulMatPoint(m_up);


    // Insert into key frame ...
    CartesianKeyFrame ckf;
    ckf.setEye(eye.get(0), eye.get(1), eye.get(2));
    ckf.setTgt(focus_.x() - eye.get(0),
	       focus_.y() - eye.get(1),
	       focus_.z() - eye.get(2));
    ckf.setUp(up.get(0), up.get(1), up.get(2));
    return ckf;
}

void GLWidget::setCameraFocus(double x, double y, double z, bool update_gl)
{
    focus_.setPoint(x, y, z);
    if (update_gl) updateGL();
}

void GLWidget::setDolly(double dolly)
{
    dolly_ = qMin(qMax(dolly, 0.0), 1.0);
    updateGL();
}

double GLWidget::dolly() const
{
    return dolly_;
}

void GLWidget::setCurrentFocusPos(double lon, double lat)
{
    focusLon_ = qMin(qMax(lon, -M_PI), M_PI);
    focusLat_ = qMin(qMax(lat, -M_PI / 2), M_PI / 2);
    updateGL();
}

QPair<double, double> GLWidget::currentFocusPos() const
{
    return qMakePair(focusLon_, focusLat_);
}

void GLWidget::updateCurrCustomBasePolygonPoint()
{
    if (!draggingCustomBasePolygonPoint_)
        currCustomBasePolygonPoint_ = ControlPanel::instance().currentCustomBasePolygonPoint(mouseLon_, mouseLat_, ballSize() / GfxUtils::getEarthRadius());
}

double GLWidget::ballSize() const
{
    return minBallSize_ + ControlPanel::instance().ballSizeFrac() * (maxBallSize_ - minBallSize_);
}
