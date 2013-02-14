#include "qcglobal.h"
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/PatternLayout.hh"
#include "log4cpp/Priority.hh"

QCLIB_BEGIN_NAMESPACE

// Returns a map of (option, value) pairs extracted from a flat list of command-line arguments.
QMap<QString, QString> getOptions(const QStringList &args)
{
    QMap <QString, QString> options;
    for (int i = 0; i < args.size(); ++i)
        if ((args.at(i).indexOf("--") == 0) && (args.at(i).size() > 2))
            options.insert(args.at(i).mid(2), (i < (args.size() - 1)) ? args.at(i + 1) : QString());
    return options;
}
QMap<QString, QString> getOptions(int argc, char *argv[]) // convenience wrapper
{
    QStringList args;
    for (int i = 1; i < argc; ++i)
        args.append(argv[i]);
    return getOptions(args);
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

static log4cpp::Category& root_ = log4cpp::Category::getRoot();

void Logger::initialize(const QString &fname)
{
    fname_ = fname;
    // ### validate fname ... 2 B DONE!

    // arrange for logging via syslog to /var/log/qccserver.log
    //log4cpp::Appender *appender = new log4cpp::SyslogAppender(...);
    // ### FOR NOW:
    log4cpp::Appender *appender = new log4cpp::FileAppender("default", fname_.toLatin1().data());

    log4cpp::PatternLayout *ptnLayout = new log4cpp::PatternLayout;
    ptnLayout->setConversionPattern("[%d{%Y %b %d %H:%M:%S}] %6p  %m%n");
    appender->setLayout(ptnLayout);
    root_.setPriority(log4cpp::Priority::INFO);
    root_.addAppender(appender);
}

void Logger::logInfo(const QString &msg) const
{
    if (fname_.isEmpty())
        qFatal("logger not initialized");
    root_.info( msg.toLatin1().data());
}

void Logger::logWarning(const QString &msg) const
{
    if (fname_.isEmpty())
        qFatal("logger not initialized");
    root_.warn( msg.toLatin1().data());
}

void Logger::logError(const QString &msg) const
{
    if (fname_.isEmpty())
        qFatal("logger not initialized");
    root_.error(msg.toLatin1().data());
}

QCLIB_END_NAMESPACE
