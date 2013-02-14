#ifndef QCGLOBAL_H
#define QCGLOBAL_H

#include <QMap>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#define QCLIB_BEGIN_NAMESPACE namespace qclib {
#define QCLIB_END_NAMESPACE }

QCLIB_BEGIN_NAMESPACE

QMap<QString, QString> getOptions(const QStringList &);
QMap<QString, QString> getOptions(int argc, char *argv[]);
bool localServerFileExists(QString *, qint64 * = 0);

class Logger
{
public:
    static Logger& instance() // Meyer's singleton
    {
        static Logger logger;
        return logger;
    }
    void initialize(const QString &);
    void logInfo(const QString &) const;
    void logWarning(const QString &) const;
    void logError(const QString &) const;
private:
    Logger() {}
    QString fname_;
};

QCLIB_END_NAMESPACE

#endif // QCGLOBAL_H
