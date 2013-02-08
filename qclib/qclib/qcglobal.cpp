#include "qcglobal.h"
#include <QDir>
#include <QFileInfo>
#include <QRegExp>

namespace qclib {

// Returns a map of (option, value) pairs extracted from a flat list of command-line arguments.
QMap<QString, QString> getOptions(const QStringList &args)
{
    QMap <QString, QString> options;
    for (int i = 0; i < args.size(); ++i)
        if ((args.at(i).indexOf("--") == 0) && (args.at(i).size() > 2))
            options.insert(args.at(i).mid(2), (i < (args.size() - 1)) ? args.at(i + 1) : QString());
    return options;
}

// Returns true iff a local server file of the form '/tmp/qclserver_USER_PID' exists.
// If such a file exists, \a path \a and pid (if non-null) are set to the absolute path and the PID
// component respectively.
bool localServerFileExists(QString *path, qint64 *pid)
{
    QDir dir("/tmp");
    QFileInfoList fileInfos = dir.entryInfoList(QStringList() << "qclserver_*", QDir::System);

    QRegExp rx(QString("^/tmp/qclserver_%1_(\\d+)$").arg(qgetenv("USER").data()));
    foreach (QFileInfo finfo, fileInfos) {
        if (rx.indexIn(finfo.absoluteFilePath()) >= 0) {
            *path = finfo.absoluteFilePath();
            if (pid) *pid = rx.cap(1).toInt();
            return true;
        }
    }
    
    *path = QString();
    if (pid) *pid = 0;
    return false;
}

} // namespace qclib
