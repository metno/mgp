#include "mainwindow.h"
#include <QApplication>
#include <GL/glut.h>
#include <QDebug>

static void printUsageAndExit(const QString &argv0)
{
    qDebug() << "usage:" << argv0 << "[--expr <xmet expression>]";
    exit(0);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    glutInit(&argc, argv);

    const QStringList args = qApp->arguments();

    if (args.contains("--help"))
        printUsageAndExit(argv[0]);

    QString expr;
    {
        const int indxExpr = args.indexOf("--expr");
        if (indxExpr >= 0) {
            if (indxExpr < (args.size() - 1))
                expr = args.at(indxExpr + 1);
            else
                printUsageAndExit(argv[0]);
        }
    }

    MainWindow::init(expr);
    MainWindow::instance().show();

    return app.exec();
}
