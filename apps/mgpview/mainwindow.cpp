#include "mainwindow.h"
#include "glwidget.h"
#include "controlpanel.h"
#include "common.h"
#include <QSlider>
#include <QApplication>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>

QString MainWindow::initExpr_ = QString();

void MainWindow::init(const QString &initExpr)
{
    initExpr_ = initExpr;
    isInit_ = true;
}

MainWindow &MainWindow::instance()
{
    if (!isInit_)
        qFatal("MainWindow not initialized");
    static MainWindow mw;
    return mw;
}

MainWindow::MainWindow()
{
    setWindowTitle("MGPView 1.1.16");

    QGridLayout *mainLayout = new QGridLayout;

    // add GL widget    
    glw_ = new GLWidget;
    mainLayout->addWidget(glw_, 1, 1);

    // add slider for changing zooming
    zoomSlider_ = new QSlider(Qt::Horizontal);
    zoomSlider_->setMinimum(0);
    zoomSlider_->setMaximum(500);
    syncZoomSlider();
    connect(zoomSlider_, SIGNAL(valueChanged(int)), SLOT(handleZoomValueChanged(int)));
    mainLayout->addWidget(zoomSlider_, 0, 1);

    // add slider for changing longitude focus
    lonSlider_ = new QSlider(Qt::Vertical);
    lonSlider_->setMinimum(0);
    lonSlider_->setMaximum(500);
    syncLonSlider();
    connect(lonSlider_, SIGNAL(valueChanged(int)), SLOT(handleLonValueChanged(int)));
    mainLayout->addWidget(lonSlider_, 1, 0);

    // add slider for changing latitude focus
    latSlider_ = new QSlider(Qt::Vertical);
    latSlider_->setMinimum(0);
    latSlider_->setMaximum(500);
    syncLatSlider();
    connect(latSlider_, SIGNAL(valueChanged(int)), SLOT(handleLatValueChanged(int)));
    mainLayout->addWidget(latSlider_, 1, 2);

    connect(glw_, SIGNAL(focusPosChanged()), SLOT(handleFocusPosChanged()));

    // add bottom panel
    QFrame *botPanel = new QFrame;
    botPanel->setLayout(new QHBoxLayout);

    QPushButton *ctrlPanelButton = new QPushButton("Control Panel");
    connect(ctrlPanelButton, SIGNAL(clicked()), SLOT(openControlPanel()));
    botPanel->layout()->addWidget(ctrlPanelButton);

    qobject_cast<QHBoxLayout *>(botPanel->layout())->addStretch(1);

    QPushButton *quitButton = new QPushButton("Quit");
    connect(quitButton, SIGNAL(clicked()), qApp, SLOT(quit()));
    botPanel->layout()->addWidget(quitButton);

    mainLayout->addWidget(botPanel, 2, 0, 1, 3);

    setLayout(mainLayout);
    resize(800, 800);

    ControlPanel::instance().initialize(initExpr_);
}

bool MainWindow::isInit_ = false;

GLWidget *MainWindow::glWidget() const
{
    return glw_;
}

void MainWindow::handleKeyPressEvent(QKeyEvent *event)
{
    keyPressEvent(event); // for now, propagate to private handler without filtering
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_Q)) {
        qApp->quit();
    } else if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_C)) {
        openControlPanel();
    } else if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_E)) {
        ControlPanel::instance().toggleFiltersEditableOnSphere();
    }
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    const qreal scale = 0.05;
    const qreal delta = -scale * (event->delta() / 360.0);
    glw_->setDolly(qMax(qMin(glw_->dolly() + delta, 1.0), 0.0));
    syncZoomSlider();
}

void MainWindow::syncZoomSlider()
{
    zoomSlider_->setValue(zoomSlider_->minimum() + glw_->dolly() * (zoomSlider_->maximum() - zoomSlider_->minimum()));
}

void MainWindow::syncLonSlider()
{
    const qreal lon = glw_->currentFocusPos().first;
    const qreal frac = (lon - -M_PI) / (2 * M_PI);
    lonSlider_->blockSignals(true);
    lonSlider_->setValue(lonSlider_->minimum() + frac * (lonSlider_->maximum() - lonSlider_->minimum()));
    lonSlider_->blockSignals(false);
}

void MainWindow::syncLatSlider()
{
    const qreal lat = glw_->currentFocusPos().second;
    const qreal frac = (lat - -M_PI / 2) / M_PI;
    latSlider_->blockSignals(true);
    latSlider_->setValue(latSlider_->minimum() + frac * (latSlider_->maximum() - latSlider_->minimum()));
    latSlider_->blockSignals(false);
}

void MainWindow::handleZoomValueChanged(int val)
{
    glw_->setDolly((qreal(val) - zoomSlider_->minimum()) / (zoomSlider_->maximum() - zoomSlider_->minimum()));
}

void MainWindow::handleLonValueChanged(int val)
{
    const qreal frac = (qreal(val) - lonSlider_->minimum()) / (lonSlider_->maximum() - lonSlider_->minimum());
    const qreal lon = -M_PI + frac * 2 * M_PI;
    glw_->setCurrentFocusPos(lon, glw_->currentFocusPos().second);
}

void MainWindow::handleLatValueChanged(int val)
{
    const qreal frac = (qreal(val) - latSlider_->minimum()) / (latSlider_->maximum() - latSlider_->minimum());
    const qreal lat = -M_PI / 2 + frac * M_PI;
    glw_->setCurrentFocusPos(glw_->currentFocusPos().first, lat);
}

void MainWindow::handleFocusPosChanged()
{
    syncLonSlider();
    syncLatSlider();
}

void MainWindow::openControlPanel()
{
    ControlPanel::instance().open();
}
