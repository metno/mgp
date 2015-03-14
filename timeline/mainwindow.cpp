#include "scene.h"
#include "view.h"
#include "mainwindow.h"
#include <QtGui>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;

    scene_ = new Scene;
    mainLayout->addWidget(new View(scene_));

    QGridLayout *controlsLayout = new QGridLayout;
    controlsLayout->addWidget(new QPushButton("Test"), 0, 0);
    mainLayout->addLayout(controlsLayout);

    setLayout(mainLayout);
    setWindowTitle("Timeline");

    resize(800, 500);
}
