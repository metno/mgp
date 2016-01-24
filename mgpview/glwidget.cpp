#include "glwidget.h"
#include "util3d.h"
#include "gfxutils.h"
#include "common.h"
//#include <qstring.h>
//#include <qcursor.h>
//#include <qmessagebox.h>
#include <GL/glut.h>
#include <QMenu>
#include <QAction>
#include <QDoubleSpinBox>
#include <QDialog>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QColor>

#include <stdio.h> // 4 TESTING!

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
    , camKfSlaveMode_(false)
    , currLon_(0.013) // London
    , currLat_(0.893)
    , focusLon_(currLon_)
    , focusLat_(currLat_)
    , focus_alt_(0)     // Surface
    , mouseLon_(0)
    , mouseLat_(0)
    , mouseHitsEarth_(false)
    , draggingFilter_(false)
    , draggingFocus_(false)
    , ballSize_(0.005 * GfxUtils::getEarthRadius())
    , minBallSize_(0.001 * GfxUtils::getEarthRadius())
    , maxBallSize_(0.05 * GfxUtils::getEarthRadius())
{
    setMouseTracking(true);

    setFocusPolicy(Qt::StrongFocus);
    focus_.setPoint(GfxUtils::getEarthRadius(), 0, 0);
    lastCam_ = CartesianKeyFrame(0, 0, 0, 1, 0, 0, 0, 0, 1);

    setCurrPosFromDialogAction_ = new QAction("Set current pos from dialog", 0);
    connect(setCurrPosFromDialogAction_, SIGNAL(triggered()), SLOT(setCurrPosFromDialog()));

    setCurrPosToThisPosAction_ = new QAction("Set current pos to this pos", 0);
    connect(setCurrPosToThisPosAction_, SIGNAL(triggered()), SLOT(setCurrPosToThisPos()));

    focusOnThisPosAction_ = new QAction("Focus on this pos", 0);
    connect(focusOnThisPosAction_, SIGNAL(triggered()), SLOT(focusOnThisPos()));

    focusOnCurrPosAction_ = new QAction("Focus on current pos", 0);
    connect(focusOnCurrPosAction_, SIGNAL(triggered()), SLOT(focusOnCurrPos()));

    focusOnCurrPos();

    // Create global popup menu ...
//    global_menu_ = new QPopupMenu(this);

    // Create and initialize 'focus' submenu ...
//    focus_menu_ = new QPopupMenu(this);
//    global_menu_->insertItem("Focus", focus_menu_);
    //
//    explicit_focus_item_ =
//	focus_menu_->insertItem("set focus to this surface point");
//    focus_menu_->connectItem(
//	explicit_focus_item_, this, SLOT(setFocusToSurfacePoint()));
//    //
//    gravity_focus_item_ = focus_menu_->insertItem(
//	"set focus to surface-projected center of gravity");
//    focus_menu_->connectItem(
//	gravity_focus_item_, this, SLOT(setFocusToCenterOfGravity()));
//    focus_menu_->setItemEnabled(gravity_focus_item_, false);
//    //
//    current_kf_focus_item_ =
//	focus_menu_->insertItem("lock focus to current key frame");
//    focus_menu_->connectItem(
//	current_kf_focus_item_, this, SLOT(lockFocusToCurrentKeyFrame()));
//    focus_menu_->setItemEnabled(current_kf_focus_item_, false);

//    // Create and initialize 'visibility' submenu ...
//    vis_menu_ = new QPopupMenu(this);
//    vis_menu_->setCheckable(true);
//    global_menu_->insertItem("Visibility", vis_menu_);
//    //
//    draw_kf_labels_item_ = vis_menu_->insertItem("key frame labels");
//    vis_menu_->connectItem(draw_kf_labels_item_, this, SLOT(toggleLabels()));
//    vis_menu_->setItemChecked(draw_kf_labels_item_, false);
//    vis_menu_->setItemEnabled(draw_kf_labels_item_, false);
//    //
//    draw_camera_item_ = vis_menu_->insertItem("camera indicator");
//    vis_menu_->connectItem(
//	draw_camera_item_, this, SLOT(toggleCameraIndicator()));
//    vis_menu_->setItemChecked(draw_camera_item_, false);
//    vis_menu_->setItemEnabled(draw_camera_item_, false);
//    //
//    draw_focus_point_item_ = vis_menu_->insertItem("focus point");
//    vis_menu_->connectItem(
//	draw_focus_point_item_, this, SLOT(toggleFocusPoint()));
//    //
//    draw_focus_point_label_item_ = vis_menu_->insertItem("focus point label");
//    vis_menu_->connectItem(
//	draw_focus_point_label_item_, this, SLOT(toggleFocusPointLabel()));

//    // Initialize key frame popup menu ...
//    kf_menu_ = new QPopupMenu(this);
//    //
//    insert_before_item_ = kf_menu_->insertItem("insert before");
//    kf_menu_->connectItem(
//	insert_before_item_, this, SLOT(insertBeforeKeyFrame()));
//    kf_menu_->setItemEnabled(insert_before_item_, false);
//    //
//    insert_after_item_ = kf_menu_->insertItem("insert after");
//    kf_menu_->connectItem(
//	insert_after_item_, this, SLOT(insertAfterKeyFrame()));
//    kf_menu_->setItemEnabled(insert_after_item_, false);
//    //
//    kf_menu_->insertSeparator();
//    //
//    remove_item_ = kf_menu_->insertItem("remove");
//    kf_menu_->connectItem(remove_item_, this, SLOT(removeKeyFrame()));
//    kf_menu_->setItemEnabled(remove_item_, false);
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

    // set viewport
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
    0, 0, 0, gfx_util.getEarthRadius(), 0.7, 0.7, 0.7, 0.7, 36, 72,
    GL_SMOOTH);

    // draw coast contours
    glShadeModel(GL_FLAT);
    gfx_util.drawCoastContours(eye, minDolly_, maxDolly_);
    glShadeModel(GL_SMOOTH);

    // draw ENOR FIR area
    glShadeModel(GL_FLAT);
    gfx_util.drawENORFIR(eye, minDolly_, maxDolly_);
    glShadeModel(GL_SMOOTH);

    // draw lat/lon circles
    glShadeModel(GL_FLAT);
    gfx_util.drawLatLonCircles(eye, minDolly_, maxDolly_);
    glShadeModel(GL_SMOOTH);

    // draw current surface point
    {
        const double
                r = GfxUtils::instance().getEarthRadius(),
                x = r * cos(currLat_) * cos(currLon_),
                y = r * cos(currLat_) * sin(currLon_),
                z = r * sin(currLat_);
        gfx_util.drawSphere(x, y, z, ballSize_, 0, 0.8, 0, 0.8, 18, 36, GL_SMOOTH);
    }

    // draw mouse point
    {
        const double
                r = GfxUtils::instance().getEarthRadius(),
                x = r * cos(mouseLat_) * cos(mouseLon_),
                y = r * cos(mouseLat_) * sin(mouseLon_),
                z = r * sin(mouseLat_);
        gfx_util.drawSphere(x, y, z, ballSize_, 0.7, 0.6, 0.4, 0.8, 18, 36, GL_SMOOTH);
    }

    // --- BEGIN draw filters --------------------------------
    // LonOrLat filters
    if (ControlPanel::instance().enabled(Filter::E_OF)) {
        bool ok = false;
        gfx_util.drawLonCircle(
                    eye, minDolly_, maxDolly_, ControlPanel::instance().value(Filter::E_OF).toDouble(&ok),
                    QColor::fromRgbF(0, 1, 1));
        Q_ASSERT(ok);
    }

    if (ControlPanel::instance().enabled(Filter::W_OF)) {
        bool ok = false;
        gfx_util.drawLonCircle(
                    eye, minDolly_, maxDolly_, ControlPanel::instance().value(Filter::W_OF).toDouble(&ok),
                    QColor::fromRgbF(1, 0, 0));
        Q_ASSERT(ok);
    }

    if (ControlPanel::instance().enabled(Filter::N_OF)) {
        bool ok = false;
        gfx_util.drawLatCircle(
                    eye, minDolly_, maxDolly_, ControlPanel::instance().value(Filter::N_OF).toDouble(&ok),
                    QColor::fromRgbF(0, 1, 1));
        Q_ASSERT(ok);
    }

    if (ControlPanel::instance().enabled(Filter::S_OF)) {
        bool ok = false;
        gfx_util.drawLatCircle(
                    eye, minDolly_, maxDolly_, ControlPanel::instance().value(Filter::S_OF).toDouble(&ok),
                    QColor::fromRgbF(1, 0, 0));
        Q_ASSERT(ok);
    }

    // FreeLine filters

    if (ControlPanel::instance().enabled(Filter::NE_OF)) {
        gfx_util.drawGreatCircleSegment(
                    eye, minDolly_, maxDolly_, ControlPanel::instance().value(Filter::NE_OF).toLineF(),
                    QColor::fromRgbF(0.8, 0.5, 0));
    }

    if (ControlPanel::instance().enabled(Filter::NW_OF)) {
        gfx_util.drawGreatCircleSegment(
                    eye, minDolly_, maxDolly_, ControlPanel::instance().value(Filter::NW_OF).toLineF(),
                    QColor::fromRgbF(0.8, 0, 1));
    }

    if (ControlPanel::instance().enabled(Filter::SE_OF)) {
        gfx_util.drawGreatCircleSegment(
                    eye, minDolly_, maxDolly_, ControlPanel::instance().value(Filter::SE_OF).toLineF(),
                    QColor::fromRgbF(0, 0.8, 0.4));
    }

    if (ControlPanel::instance().enabled(Filter::SW_OF)) {
        gfx_util.drawGreatCircleSegment(
                    eye, minDolly_, maxDolly_, ControlPanel::instance().value(Filter::SW_OF).toLineF(),
                    QColor::fromRgbF(0, 0.4, 0.8));
    }

    // --- END draw filters --------------------------------

    // show mouse position
    {
        QString s;
        s.sprintf("mouse: lon = %.3f, lat = %.3f", (mouseLon_ / M_PI) * 180, (mouseLat_ / M_PI) * 180);
        gfx_util.drawBottomString(s.toLatin1(), width(), height(), 0, 0, QColor::fromRgbF(1, 1, 0), QColor::fromRgbF(0, 0, 0));
    }

    // show current position
    {
        QString s;
        s.sprintf("curr pos: lon = %.3f, lat = %.3f", (currLon_ / M_PI) * 180, (currLat_ / M_PI) * 180);
        gfx_util.drawBottomString(s.toLatin1(), width(), height(), 0, 0, QColor::fromRgbF(1, 1, 1), QColor::fromRgbF(0, 0.3, 0), false);
    }

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


void GLWidget::computePixel(
    double wx, double wy, double wz, int &x, int &y)
{
    // Transform world coordinates into window coordinates ...
    GLint viewport[4];
    GLdouble mvmatrix[16], projmatrix[16];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
    GLdouble dx, dy, dz;
    gluProject(
	(GLdouble)wx, (GLdouble)wy, (GLdouble)wz, mvmatrix, projmatrix,
	viewport, &dx, &dy, &dz);

    x = (int)dx;
    y = (int)dy;
}

void GLWidget::drawLabel(
    const char s[], int x, int y, float r, float g, float b)
{
    // Assuming matrix mode = model-view and depth-test enabled!

    glPushMatrix(); // Save model-view matrix
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); // Save projection matrix
    glLoadIdentity();
    gluOrtho2D(0, width(), 0, height());

    glDisable(GL_DEPTH_TEST);

    // Draw background quad (to ensure visibility of text) ...
    glColor3f(1 - r, 1 - g, 1 - b);
    const int
	pad = 2,
	offset_x = 5,
	offset_y = 5,
	xx = (x - pad) + offset_x,
	yy = (y - pad) + offset_y,
	w  =  9 + 2 * pad,
	h  = 15 + 2 * pad;
    glBegin(GL_QUADS);
    glVertex2d(xx    , yy);
    glVertex2d(xx + w, yy);
    glVertex2d(xx + w, yy + h);
    glVertex2d(xx    , yy + h);
    glEnd();

    // Draw text ...
    glColor3f(r, g, b);
    glRasterPos2f(x + offset_x, y + offset_y);
    for (int i = 0; s[i]; i++)
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, s[i]);

    glEnable(GL_DEPTH_TEST);

    glPopMatrix(); // Restore projection matrix

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix(); // Restore model-view matrix
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
        contextMenu.addAction(setCurrPosFromDialogAction_);
        contextMenu.addAction(setCurrPosToThisPosAction_);
        contextMenu.addAction(focusOnThisPosAction_);
        contextMenu.addAction(focusOnCurrPosAction_);
        contextMenu.exec(QCursor::pos());

    } else if (mouseHitsEarth_) {

        if ((event->modifiers() & Qt::ControlModifier) && (event->button() == Qt::LeftButton)) {
            // update current surface position
            currLon_ = mouseLon_;
            currLat_ = mouseLat_;
        } else {
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
            } else if ((event->button() == Qt::LeftButton) || (event->button() == Qt::MiddleButton)) {
                // ... camera (i.e. lat/lon focus)
                draggingFocus_ = true;
                dragBaseLon_ = focusLon_;
                dragBaseLat_ = focusLat_;
            }
        }

        updateGL();
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *)
{
    draggingFilter_ = draggingFocus_ = false;
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!(mouseHitsEarth_ = intersectsEarth(event, mouseLon_, mouseLat_)))
        return;

    if (draggingFilter_) {
        ControlPanel::instance().updateFilterDragging(mouseLon_, mouseLat_);

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

void GLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    double lon, lat;
    if (intersectsEarth(event, lon, lat) && (event->button() == Qt::LeftButton)) {
        // update current surface position
        currLat_ = lat;
        currLon_ = lon;
        updateGL();
    }
}

void GLWidget::enterEvent(QEvent *)
{
    updateGL();
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
    dolly_ = min(max(dolly, 0), 1);
    updateGL();
}

double GLWidget::dolly() const
{
    return dolly_;
}

void GLWidget::setCurrentFocusPos(double lon, double lat)
{
    focusLon_ = min(max(lon, -M_PI), M_PI);
    focusLat_ = min(max(lat, -M_PI / 2), M_PI / 2);
    updateGL();
}

QPair<double, double> GLWidget::currentFocusPos() const
{
    return qMakePair(focusLon_, focusLat_);
}


void GLWidget::setHeading(double heading)
{
    heading_ = min(max(heading, 0), 1);
    updateGL();
}


void GLWidget::setInclination(double incl)
{
    incl_ = min(max(incl, 0), 1);
    updateGL();
}


void GLWidget::drawCalled(QObject *ip_ckf)
{
/* FOR SOME REASON THE NEXT LINE CRASHES!
    CartesianKeyFrame *last_cam =
    dynamic_cast<CartesianKeyFrame *>(ip_ckf);
    if (!last_cam)
    throw ProgrammingError(
	    __FILE__, __LINE__, "failed to cast to CartesianKeyFrame");
    last_cam_ = *last_cam;
*/

    // Use this line for now:
    lastCam_ = *((CartesianKeyFrame *)ip_ckf);

    updateGL();
}

void GLWidget::setCurrPosFromDialog()
{
    QDialog dialog;
    dialog.setWindowTitle("Set current position");
    QVBoxLayout mainLayout;

    dialog.setLayout(&mainLayout);

    QFormLayout formLayout;
    mainLayout.addLayout(&formLayout);

    QDoubleSpinBox lonSpinBox;
    lonSpinBox.setMinimum(-180);
    lonSpinBox.setMaximum(180);
    lonSpinBox.setValue((currLon_ / M_PI) * 180);
    formLayout.addRow("Longitude:", &lonSpinBox);

    QDoubleSpinBox latSpinBox;
    latSpinBox.setMinimum(-90);
    latSpinBox.setMaximum(90);
    latSpinBox.setValue((currLat_ / (M_PI / 2)) * 90);
    formLayout.addRow("Latitude:", &latSpinBox);

    QDialogButtonBox buttonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    connect(buttonBox.button(QDialogButtonBox::Cancel), SIGNAL(clicked()), &dialog, SLOT(reject()));
    connect(buttonBox.button(QDialogButtonBox::Ok), SIGNAL(clicked()), &dialog, SLOT(accept()));
    mainLayout.addWidget(&buttonBox);

    if (dialog.exec() == QDialog::Accepted) {
        currLon_ = (lonSpinBox.value() / 180) * M_PI;
        currLat_ = (latSpinBox.value() / 90) * (M_PI / 2);
        updateGL();
    }
}

void GLWidget::setCurrPosToThisPos()
{
    currLat_ = mouseLat_;
    currLon_ = mouseLon_;
    updateGL();
}

void GLWidget::focusOnThisPos()
{
    focusLat_ = mouseLat_;
    focusLon_ = mouseLon_;
    focus_alt_ = 0;
    updateGL();
    emit focusPosChanged();
}

void GLWidget::focusOnCurrPos()
{
    focusLat_ = currLat_;
    focusLon_ = currLon_;
    focus_alt_ = 0;
    updateGL();
    emit focusPosChanged();
}

void GLWidget::setBallSizeFrac(float frac)
{
    ballSize_ = minBallSize_ + min(max(frac, 0), 1) * (maxBallSize_ - minBallSize_);
    updateGL();
}

float GLWidget::ballSizeFrac() const
{
    return (ballSize_ - minBallSize_) / (maxBallSize_ - minBallSize_);
}

//void GLWidget::toggleVisMenuItem(int item)
//{
//    vis_menu_->setItemChecked(item, !vis_menu_->isItemChecked(item));
//    updateGL();
//}


//void GLWidget::toggleLabels()
//{
//    toggleVisMenuItem(draw_kf_labels_item_);
//}


//void GLWidget::toggleCameraIndicator()
//{
//    toggleVisMenuItem(draw_camera_item_);
//}


//void GLWidget::toggleFocusPoint()
//{
//    toggleVisMenuItem(draw_focus_point_item_);
//}


//void GLWidget::toggleFocusPointLabel()
//{
//    toggleVisMenuItem(draw_focus_point_label_item_);
//}
