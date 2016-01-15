#include "mainwindow.h"
#include "glwidget.h"
#include "common.h"
#include <QSlider>
#include <QApplication>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>

void MainWindow::init()
{
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
    setWindowTitle("MGPView 0.0.0");

    QGridLayout *mainLayout = new QGridLayout;

    // add GL widget
    glw_ = new GLWidget;
    mainLayout->addWidget(glw_, 1, 1);

    // add slider for changing zooming
    zoomSlider_ = new QSlider(Qt::Horizontal);
    syncZoomSlider();
    connect(zoomSlider_, SIGNAL(valueChanged(int)), SLOT(handleZoomValueChanged(int)));
    mainLayout->addWidget(zoomSlider_, 0, 1);

    // add slider for changing longitude focus
    lonSlider_ = new QSlider(Qt::Vertical);
    mainLayout->addWidget(lonSlider_, 1, 0);

    // add slider for changing latitude focus
    latSlider_ = new QSlider(Qt::Vertical);
    mainLayout->addWidget(latSlider_, 1, 2);

    // add bottom panel
    QFrame *botPanel = new QFrame;
    botPanel->setLayout(new QHBoxLayout);

    QPushButton *button1 = new QPushButton("1");
    botPanel->layout()->addWidget(button1);

    QPushButton *button2 = new QPushButton("2");
    botPanel->layout()->addWidget(button2);

    qobject_cast<QHBoxLayout *>(botPanel->layout())->addStretch(1);

    QPushButton *closeButton = new QPushButton("Close");
    connect(closeButton, SIGNAL(pressed()), qApp, SLOT(quit()));
    botPanel->layout()->addWidget(closeButton);

    mainLayout->addWidget(botPanel, 2, 0, 1, 3);

    setLayout(mainLayout);
    resize(800, 800);
}

bool MainWindow::isInit_ = false;

void MainWindow::handleKeyPressEvent(QKeyEvent *event)
{
    keyPressEvent(event); // for now, propagate to private handler without filtering
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Escape) || ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_Q))) {
        qApp->quit();
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

void MainWindow::handleZoomValueChanged(int val)
{
    glw_->setDolly((qreal(val) - zoomSlider_->minimum()) / (zoomSlider_->maximum() - zoomSlider_->minimum()));
}

void MainWindow::handleLonValueChanged(int val)
{
}

void MainWindow::handleLatValueChanged(int val)
{
}
