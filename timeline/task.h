#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDateTime>

class TaskProperties
{
    friend class Task;

public:
    TaskProperties() {}

    TaskProperties(
            const QString &name, const QString &summary, const QString description,
            const QDateTime &loDateTime, const QDateTime &hiDateTime)
        : name_(name)
        , summary_(summary)
        , description_(description)
        , loDateTime_(loDateTime)
        , hiDateTime_(hiDateTime)
    {
    }

    TaskProperties(
            const QString &name, const QString &summary, const QString description,
            int loTimestamp, int hiTimestamp)
        : name_(name)
        , summary_(summary)
        , description_(description)
    {
        loDateTime_.setTime_t(loTimestamp);
        hiDateTime_.setTime_t(hiTimestamp);
    }

private:
    QString name_;
    QString summary_;
    QString description_;
    QDateTime loDateTime_; // start time
    QDateTime hiDateTime_; // end time
};

class Task
{
    friend class TaskManager;

public:
    qint64 roleId() const;
    QString name() const;
    QString summary() const;
    QString description() const;
    QDateTime loDateTime() const;
    QDateTime hiDateTime() const;

private:
    Task();
    Task(const TaskProperties &);

    qint64 roleId_; // role to which this task is assigned (< 0 if task is unnasigned)
    TaskProperties props_;

    void setRoleId(qint64);
    void setName(const QString &);
    void setSummary(const QString &);
    void setDescription(const QString &);
    void setLoDateTime(const QDateTime &);
    void setLoUTCTimestamp(int);
    void setHiDateTime(const QDateTime &);
    void setHiUTCTimestamp(int);
};

#endif // TASK_H
