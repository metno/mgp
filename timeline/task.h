#ifndef TASK_H
#define TASK_H

#include <QString>

class Task
{
    friend class TaskManager;
public:
    Task(const QString &);
    QString name() const;
    void setName(const QString &);
private:
    QString name_;
    qint64 roleId_; // role to which this task is assigned (< 0 if task is unnasigned)
};

#endif // TASK_H
