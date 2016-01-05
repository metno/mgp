//#include "communicator.h" // ### implement later to handle communication with other clients via central server
#include "mainwindow.h"
#include "taskmanager.h"
#include "common.h"
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QApplication>
#include <QSettings>
#include <QSharedPointer>

QSharedPointer<QSettings> settings;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // create a global settings object
    settings = QSharedPointer<QSettings>(
                new QSettings(QString("%1/.config/metorg/client.conf").arg(qgetenv("HOME").constData()), QSettings::NativeFormat));
    if (settings->status() != QSettings::NoError) {
        qWarning() << "WARNING: error in settings file:" << settings->status() << "(settings will be ignored)";
        settings.clear();
    }

    QDate baseDate = QDate::currentDate();
    int dateSpan = 7;
    if (settings) {
        if (settings->value("baseDate").isValid()) {
            const QDate tmp = settings->value("baseDate").toDate();
            if (tmp.isValid())
                baseDate = tmp;
            else
                qWarning() << "WARNING: base date found in settings file, but invalid; using default value:" << baseDate;
        } else {
            qWarning() << "WARNING: no base date found in settings file; using default value:" << baseDate;
        }
        if (settings->value("dateSpan").isValid()) {
            bool ok = false;
            const int tmp = settings->value("dateSpan").toInt(&ok);
            if (ok)
                dateSpan = tmp;
            else
                qWarning() << "WARNING: date span found in settings file, but invalid; using default value:" << dateSpan;
        } else {
            qWarning() << "WARNING: no date span found in settings file; using default value:" << dateSpan;
        }
    } else {
        qWarning() << "WARNING: no valid settings file found";
    }
    MainWindow::init(baseDate, dateSpan);

    MainWindow::instance().show();

    TaskManager::instance().emitUpdated(); // ensure views are initialized
    return app.exec();
}
