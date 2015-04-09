#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDateTime>

class Task
{
    friend class TaskManager;
public:
    Task(const QString &, const QDateTime &, const QDateTime &);
    QString name() const;
    void setName(const QString &);
    qint64 roleId() const;
    QDateTime loDateTime() const;
    QDateTime hiDateTime() const;
private:
    QString name_;
    qint64 roleId_; // role to which this task is assigned (< 0 if task is unnasigned)
    QDateTime loDateTime_;
    QDateTime hiDateTime_;
};

#endif // TASK_H
