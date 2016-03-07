#include "mainwindow.h"
#include <QApplication>
#include <GL/glut.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    glutInit(&argc, argv);

    MainWindow::init();
    MainWindow::instance().show();

    return app.exec();
}
