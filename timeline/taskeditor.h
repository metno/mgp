#ifndef TASKEDITOR_H
#define TASKEDITOR_H

#include <QDialog>
#include <QHash>

class Task;
class QLineEdit;
class QTextBrowser;

class TaskEditor : public QDialog
{
    Q_OBJECT

public:
    static TaskEditor &instance();
    QHash<QString, QString> edit(const Task *);
    QLineEdit *nameEdit_;
    QLineEdit *summaryEdit_;
    QTextBrowser *descrEdit_;

private:
    TaskEditor();
};

#endif // TASKEDITOR_H
