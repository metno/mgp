#include "externalview.h"
#include <QApplication>
#include <QDebug>
#include <GL/glut.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    glutInit(&argc, argv);

    ExternalView *view = new ExternalView;
    view->show();

    return app.exec();
}
