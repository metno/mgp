#include <QtCore>
#include <QDebug>

class DateTimeFilesExtractor
{
    // Returns a datetime pattern where occurrences of 'MM' are replaced with 'mm' and vice versa.
    static QString convertDateTimePattern(const QString &s)
    {
        QString r(s);
        for (int i = 0; i < r.size(); ++i) {
            if (i > 0) {
                if ((r[i] == 'M') && (r[i - 1] == 'M'))
                    r[i - 1] = r[i] = 'm';
                else if ((r[i] == 'm') && (r[i - 1] == 'm'))
                    r[i - 1] = r[i] = 'M';
            }
        }
        return r;
    }

    static bool extractComponents(
        const QString &filePattern, QString &prefix, QString &dtPattern, QString &suffix, QDir &dir)
    {
        QRegExp rx("^([^\\[\\]]+)\\[([^\\[\\]]+)\\]([^\\[\\]]*)$");
        if (rx.indexIn(filePattern) < 0)
            return false;
        prefix = rx.cap(1);
        dtPattern = convertDateTimePattern(rx.cap(2));
        suffix = rx.cap(3);
        dir = QFileInfo(filePattern).dir();
        return true;
    }

    static void extractMatchingFiles(
        QString &prefix, QString &dtPattern, QString &suffix, const QDir &dir,
        QList<QPair<QFileInfo, QDateTime> > &result)
    {
        QDirIterator dit(dir);
        QRegExp rx(QString("^%1(.+)%2$").arg(prefix).arg(suffix));
        while (dit.hasNext()) {
            const QString fileName = dit.next();
            if (rx.indexIn(fileName) >= 0) {
                const QDateTime dateTime = QDateTime::fromString(rx.cap(1), dtPattern);
                if (dateTime.isValid())
                    result.append(qMakePair(QFileInfo(fileName), dateTime));
            }
        }
    }

    static bool lessThan(const QPair<QFileInfo, QDateTime> &p1, const QPair<QFileInfo, QDateTime> &p2)
    {
        return p1.second < p2.second;
    }

public:

    // Returns a list of (fileInfo, dateTime) pairs for files matching a file pattern
    // of the form <part1>[<part2>]<part3> where none of <part1-3> contain '[' or ']'
    // and <part2> is a format string similar to the second argument of
    // QDateTime::fromString(const QString &, const QString &), except that the minute
    // is 'MM' and the month number is 'mm'. Example:
    //
    //   "/home/joa/foo/gale_[yyyymmddtHHMM].kml"
    //
    static QList<QPair<QFileInfo, QDateTime> > getFiles(const QString &filePattern)
    {
        // extract prefix, converted datetime pattern, suffix, and directory
        QString prefix;
        QString dtPattern;
        QString suffix;
        QDir dir;
        if (!extractComponents(filePattern, prefix, dtPattern, suffix, dir)) {
            qWarning("getDateTimeFiles(): invalid file pattern");
            return QList<QPair<QFileInfo, QDateTime> >();
        }
    
        // extract matching files
        QList<QPair<QFileInfo, QDateTime> > result;
        extractMatchingFiles(prefix, dtPattern, suffix, dir, result);

        // sort chronologically so that the oldest dateTime appears first
        qSort(result.begin(), result.end(), lessThan);

        return result;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QString filePattern("/disk1/gale_warnings/gale_[yyyymmddtHHMM].kml");

    QList<QPair<QFileInfo, QDateTime> > dtFiles = DateTimeFilesExtractor::getFiles(filePattern);
    for (int i = 0; i < dtFiles.size(); ++i)
        qDebug() << dtFiles.at(i).first.filePath() << " / " << dtFiles.at(i).second;

    return 0;
}
