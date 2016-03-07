#include "glwidget.h"
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

CartesianKeyFrame::CartesianKeyFrame(const CartesianKeyFrame& src)
    : eye_(src.eye_)
    , tgt_(src.tgt_)
    , up_(src.up_)
{
}

CartesianKeyFrame::CartesianKeyFrame(
    double ex, double ey, double ez,
    double tx, double ty, double tz,
    double ux, double uy, double uz)
{
    setEye(ex, ey, ez);
    setTgt(tx, ty, tz);
    setUp (ux, uy, uz);
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
    , draggingWIFilter_(false)
    , draggingOtherFilter_(false)
    , draggingCustomBasePolygon_(false)
    , draggingFocus_(false)
    , minBallSize_(0.001 * GfxUtils::getEarthRadius())
    , maxBallSize_(0.02 * GfxUtils::getEarthRadius())
    , currWIFilterPoint_(-1)
    , currCustomBasePolygonPoint_(-1)
{
    // --- BEGIN register filter types -------------------

    // LonOrLat filters
    lonOrLatFilterTypes_.append(mgp::FilterBase::E_OF);
    lonOrLatFilterTypes_.append(mgp::FilterBase::W_OF);
    lonOrLatFilterTypes_.append(mgp::FilterBase::N_OF);
    lonOrLatFilterTypes_.append(mgp::FilterBase::S_OF);

    // FreeLine filters
    freeLineFilterTypes_.append(mgp::FilterBase::E_OF_LINE);
    freeLineFilterTypes_.append(mgp::FilterBase::W_OF_LINE);
    freeLineFilterTypes_.append(mgp::FilterBase::N_OF_LINE);
    freeLineFilterTypes_.append(mgp::FilterBase::S_OF_LINE);
    freeLineFilterTypes_.append(mgp::FilterBase::NE_OF_LINE);
    freeLineFilterTypes_.append(mgp::FilterBase::NW_OF_LINE);
    freeLineFilterTypes_.append(mgp::FilterBase::SE_OF_LINE);
    freeLineFilterTypes_.append(mgp::FilterBase::SW_OF_LINE);

    // --- END register filter types -------------------


    setMouseTracking(true);

    setFocusPolicy(Qt::StrongFocus);
    focus_.setPoint(GfxUtils::getEarthRadius(), 0, 0);

    addWIFilterPointAction_ = new QAction("Add point", 0);
    connect(addWIFilterPointAction_, SIGNAL(triggered()), SLOT(addWIFilterPoint()));

    removeWIFilterPointAction_ = new QAction("Remove point", 0);
    connect(removeWIFilterPointAction_, SIGNAL(triggered()), SLOT(removeWIFilterPoint()));

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
    mgp::math::_3DPoint
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

    // draw axes
    //GfxUtils::drawAxes();

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

    if (ControlPanel::instance().currentBasePolygon()) {
        const mgp::Polygon points = ControlPanel::instance().currentBasePolygon();

        if (ControlPanel::instance().basePolygonLinesVisible()) {
            glShadeModel(GL_FLAT);
            gfx_util.drawSurfacePolygon(points, eye, minDolly_, maxDolly_, QColor::fromRgbF(0, 0, 1), 2);
            glShadeModel(GL_SMOOTH);
        }

        if (ControlPanel::instance().basePolygonPointsVisible() ||
                ((ControlPanel::instance().currentBasePolygonType() == BasePolygon::Custom)
                && ControlPanel::instance().customBasePolygonEditableOnSphere())) {

            // draw points
            for (int i = 0; i < points->size(); ++i) {

                float r, g, b;
                if (i == currCustomBasePolygonPoint_) {
                    r = 1.0;
                    g = 1.0;
                    b = 0.0;
                } else if (ControlPanel::instance().rejectedByAnyFilter(points->at(i))) {
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
            }
        }

        if (ControlPanel::instance().basePolygonIntersectionsVisible()) {
            // draw intersections
            const QVector<mgp::Point> isctPoints = ControlPanel::instance().filterIntersections(points);
            for (int i = 0; i < isctPoints.size(); ++i)
                gfx_util.drawSurfaceBall(isctPoints.at(i).first, isctPoints.at(i).second, ballSize(), 1, 0, 1, 1);
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

    // Within filter
    if (ControlPanel::instance().isEnabled(mgp::FilterBase::WI))
    {
        const mgp::Polygon points = ControlPanel::instance().WIFilterPolygon();

        const bool curr = ControlPanel::instance().isCurrent(mgp::FilterBase::WI);
        const bool valid = ControlPanel::instance().isValid(mgp::FilterBase::WI);
        const QColor color = curr ? (valid ? currValidColor : currInvalidColor) : (valid ? normalValidColor : normalInvalidColor);
        const float lineWidth = curr ? currLineWidth : normalLineWidth;

        if (ControlPanel::instance().filterLinesVisible()) {
            // draw polygon
            glShadeModel(GL_FLAT);
            gfx_util.drawSurfacePolygon(points, eye, minDolly_, maxDolly_, color, lineWidth);
            glShadeModel(GL_SMOOTH);
        }

        // draw points
        if (ControlPanel::instance().filterPointsVisible() && ControlPanel::instance().isCurrent(mgp::FilterBase::WI)) {
            for (int i = 0; i < points->size(); ++i) {

                float r, g, b;
                if (i == currWIFilterPoint_) {
                    r = 1.0;
                    g = 1.0;
                    b = 0.0;
                } else {
                    r = 1.0;
                    g = 1.0;
                    b = 1.0;
                }
                gfx_util.drawSurfaceBall(points->at(i).first, points->at(i).second, ballSize(), r, g, b, 0.8);
            }
        }
    }

    // LonOrLat filters
    if (ControlPanel::instance().filterLinesVisible()) {
        for (int i = 0; i < 4; ++i) {
            const mgp::FilterBase::Type type = lonOrLatFilterTypes_.at(i);
            const bool curr = ControlPanel::instance().isCurrent(type);
            const QColor color = curr ? currColor : normalColor;
            const float lineWidth = curr ? currLineWidth : normalLineWidth;

            if (ControlPanel::instance().isEnabled(type)) {
                // draw circle
                bool ok = false;
                const bool isLon = (type == mgp::FilterBase::E_OF) || (type == mgp::FilterBase::W_OF);
                gfx_util.drawLonOrLatCircle(
                            isLon, eye, minDolly_, maxDolly_, ControlPanel::instance().value(type).toDouble(&ok),
                            color, lineWidth);
                Q_ASSERT(ok);
            }
        }
    }

    // FreeLine filters
    for (int i = 0; i < 8; ++i) {
        const mgp::FilterBase::Type type = freeLineFilterTypes_.at(i);
        const bool curr = ControlPanel::instance().isCurrent(type);
        const bool valid = ControlPanel::instance().isValid(type);
        const QColor color = curr ? (valid ? currValidColor : currInvalidColor) : (valid ? normalValidColor : normalInvalidColor);
        const float lineWidth = curr ? currLineWidth : normalLineWidth;

        if (ControlPanel::instance().isEnabled(type)) {
            const QLineF line = ControlPanel::instance().value(type).toLineF();

            if (ControlPanel::instance().filterLinesVisible()) {
                // draw line
                gfx_util.drawGreatCircle(eye, minDolly_, maxDolly_, line, color, lineWidth);

                // draw the complete great circle passing through the endpoints
                gfx_util.drawGreatCircle(eye, minDolly_, maxDolly_, line, valid ? normalValidColor : normalInvalidColor, normalLineWidth, false);
            }

            if (ControlPanel::instance().filterPointsVisible()) {
                // draw endpoints
                if (ControlPanel::instance().isCurrent(type)) {
                    gfx_util.drawSurfaceBall(DEG2RAD(line.p1().x()), DEG2RAD(line.p1().y()), ballSize(), 1.0, 1.0, 1.0, 0.8);
                    gfx_util.drawSurfaceBall(DEG2RAD(line.p2().x()), DEG2RAD(line.p2().y()), ballSize(), 1.0, 1.0, 1.0, 0.8);
                }
            }
        }
    }

    // --- END draw filters --------------------------------


    // --- BEGIN draw result polygons --------------------------------

    if (ControlPanel::instance().resultPolygonsLinesVisible() || ControlPanel::instance().resultPolygonsPointsVisible()) {
        const mgp::Polygons polygons = ControlPanel::instance().resultPolygons();
        ControlPanel::instance().updateResultPolygonsGroupBoxTitle(polygons->size());

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
                if (polygons->at(i)) {
                    for (int j = 0; j < polygons->at(i)->size(); ++j) {
                        gfx_util.drawSurfaceBall(polygons->at(i)->at(j).first, polygons->at(i)->at(j).second, ballSize(), 1, 0.5, 0, 1);
                    }
                }
            }
        }
    } else {
        ControlPanel::instance().updateResultPolygonsGroupBoxTitle(-1);
    }

    // --- END draw result polygons --------------------------------


    if (mouseHitsEarth_) {
        // show mouse position
        QString s;
        s.sprintf("lon = %.3f, lat = %.3f", RAD2DEG(mouseLon_), RAD2DEG(mouseLat_));
        gfx_util.drawBottomString(s.toLatin1(), width(), height(), 0, 0, QColor::fromRgbF(1, 1, 0), QColor::fromRgbF(0, 0, 0));

        // for the current base polygon, show whether it is oriented clockwise, and whether the mouse position is inside it
        s.sprintf("current base polygon; clockwise: %d; mouse inside: %d",
                  ControlPanel::instance().currentBasePolygonIsClockwise(),
                  ControlPanel::instance().withinCurrentBasePolygon(qMakePair(mouseLon_, mouseLat_)));
        gfx_util.drawBottomString(s, width(), height(), 0, 0, QColor::fromRgbF(1, 1, 1), QColor::fromRgbF(0, 0.3, 0), false);
    }


    glFlush();
}

void GLWidget::computeRay(int x, int y, mgp::math::_4DPoint &eye, mgp::math::_4DPoint &ray)
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
    mgp::math::_3DPoint *eye2 = ckf.getEye();
    ray.set(wx - eye2->x(), wy - eye2->y(), wz - eye2->z());
    ray.normalize();
    eye.set(eye2->x(), eye2->y(), eye2->z());
}

bool GLWidget::intersectsEarth(QMouseEvent *event, double &lon, double &lat)
{
    mgp::math::_4DPoint eye, ray;
    computeRay(event->x(), event->y(), eye, ray);

    double wx, wy, wz;
    if (mgp::math::Math::raySphereIntersect(
                eye.x(), eye.y(), eye.z(),
                ray.x(), ray.y(), ray.z(),
                0, 0, 0,
                GfxUtils::instance().getEarthRadius(),
                wx, wy, wz)) {
        mgp::math::Math::computeLatLon(wx, wy, wz, lat, lon);
        lon = fmod(lon + M_PI, 2 * M_PI) - M_PI; // [0, 2PI] -> [-PI, PI]
        return true;
    }

    return false;
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        QMenu contextMenu;
        if (ControlPanel::instance().filtersEditableOnSphere()
                && (currWIFilterPoint_ >= 0)) {
            contextMenu.addAction(addWIFilterPointAction_);
            contextMenu.addAction(removeWIFilterPointAction_);
            removeWIFilterPointAction_->setEnabled(ControlPanel::instance().WIFilterPolygon()->size() > 3);
        } else if ((ControlPanel::instance().currentBasePolygonType() == BasePolygon::Custom)
                && ControlPanel::instance().customBasePolygonEditableOnSphere()
                && (currCustomBasePolygonPoint_ >= 0)) {
            contextMenu.addAction(addCustomBasePolygonPointAction_);
            contextMenu.addAction(removeCustomBasePolygonPointAction_);
            removeCustomBasePolygonPointAction_->setEnabled(ControlPanel::instance().currentBasePolygon()->size() > 3);
        }

        contextMenu.exec(QCursor::pos());

    } else if (mouseHitsEarth_) {

        // start dragging ...
        dragBaseX_ = event->x();
        dragBaseY_ = event->y();
        if ((event->button() == Qt::LeftButton)
                && ControlPanel::instance().filtersEditableOnSphere()
                && ControlPanel::instance().isCurrent(mgp::FilterBase::WI)
                && (currWIFilterPoint_ >= 0)) {
            // ... WI filter
            draggingWIFilter_ = true;
            dragBaseLon_ = mouseLon_;
            dragBaseLat_ = mouseLat_;
        } else if ((event->button() == Qt::LeftButton)
                && ControlPanel::instance().filtersEditableOnSphere()
                && ControlPanel::instance().startFilterDragging(qMakePair(mouseLon_, mouseLat_))) {
            // ... other filter
            draggingOtherFilter_ = true;
            dragBaseLon_ = mouseLon_;
            dragBaseLat_ = mouseLat_;
        } else if ((event->button() == Qt::LeftButton)
                   && ControlPanel::instance().customBasePolygonEditableOnSphere()
                   && (currCustomBasePolygonPoint_ >= 0)) {
            // ... custom base polygon
            draggingCustomBasePolygon_ = true;
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
    draggingWIFilter_ = draggingOtherFilter_ = draggingCustomBasePolygon_ = draggingFocus_ = false;
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(mouseHitsEarth_ = intersectsEarth(event, mouseLon_, mouseLat_))) {
        updateGL();
        return;
    }

    updateWIFilterPoint();
    updateCurrCustomBasePolygonPoint();

    if (draggingWIFilter_) {
        ControlPanel::instance().updateWIFilterPointDragging(currWIFilterPoint_, qMakePair(mouseLon_, mouseLat_));

    } else if (draggingOtherFilter_) {
        ControlPanel::instance().updateFilterDragging(qMakePair(mouseLon_, mouseLat_));

    } else if (draggingCustomBasePolygon_) {
        ControlPanel::instance().updateCustomBasePolygonPointDragging(currCustomBasePolygonPoint_, qMakePair(mouseLon_, mouseLat_));

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

    if (currWIFilterPoint_ >= 0) {
        if (event->key() == Qt::Key_Insert)
            addWIFilterPoint();
        else if (event->key() == Qt::Key_Delete)
            removeWIFilterPoint();
    } else if (currCustomBasePolygonPoint_ >= 0) {
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

void GLWidget::addWIFilterPoint()
{
    Q_ASSERT(currWIFilterPoint_ >= 0);
    ControlPanel::instance().addPointToWIFilter(currWIFilterPoint_);
}

void GLWidget::removeWIFilterPoint()
{
    Q_ASSERT(currWIFilterPoint_ >= 0);
    ControlPanel::instance().removePointFromWIFilter(currWIFilterPoint_);
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
    mgp::math::_4x4Matrix m_rot1, m_tmp;
    m_rot1.loadRotateY((1 - incl_) * M_PI_2);
    m_tmp.loadRotateX(fmod((heading_ + 0.5) * 2 * M_PI, 2 * M_PI));
    m_rot1.mulMatLeft(m_tmp);

    // Compute matrix for rotating the x unit vector into the focus vector ...
    double lat, lon;
    mgp::math::Math::computeLatLon(focus_.x(), focus_.y(), focus_.z(), lat, lon);
    //
    mgp::math::_4x4Matrix m_rot2;
    m_rot2.loadRotateY(-lat);
    m_tmp.loadRotateZ(lon);
    m_rot2.mulMatLeft(m_tmp);

    // Set matrix for translating the origin into the focus point ...
    mgp::math::_4x4Matrix m_tsl;
    m_tsl.loadTranslate(mgp::math::Math::norm(focus_.x(), focus_.y(), focus_.z()), 0, 0);

    // Compute eye-point ...
    mgp::math::_4x4Matrix m_eye = m_rot1;
    m_eye.mulMatLeft(m_tsl);
    m_eye.mulMatLeft(m_rot2);
    mgp::math::_4DPoint eye(minDolly_ + pow(dolly_, 2) * (maxDolly_ - minDolly_), 0, 0);
    eye.mulMatPoint(m_eye);

    // Compute up-vector ...
    mgp::math::_4x4Matrix m_up = m_rot1;
    m_up.mulMatLeft(m_rot2);
    mgp::math::_4DPoint up(0, 0, 1);
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

mgp::Point GLWidget::currentFocusPos() const
{
    return qMakePair(focusLon_, focusLat_);
}

void GLWidget::updateWIFilterPoint()
{
    if (!draggingWIFilter_)
        currWIFilterPoint_ = ControlPanel::instance().currentWIFilterPoint(
                    qMakePair(mouseLon_, mouseLat_), ballSize() / GfxUtils::getEarthRadius());
}

void GLWidget::updateCurrCustomBasePolygonPoint()
{
    if (!draggingCustomBasePolygon_)
        currCustomBasePolygonPoint_ = ControlPanel::instance().currentCustomBasePolygonPoint(
                    qMakePair(mouseLon_, mouseLat_), ballSize() / GfxUtils::getEarthRadius());
}

double GLWidget::ballSize() const
{
    return minBallSize_ + ControlPanel::instance().ballSizeFrac() * (maxBallSize_ - minBallSize_);
}
