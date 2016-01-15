#include "glwidget.h"
#include <QApplication>
#include <QDebug>
#include <GL/glut.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    glutInit(&argc, argv);

    GLWidget *glw = new GLWidget;
    glw->show();

    return app.exec();
}
