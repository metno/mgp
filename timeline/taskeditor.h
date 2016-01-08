#ifndef TASKEDITOR_H
#define TASKEDITOR_H

#include <QDialog>
#include <QHash>
#include <QVariant>
#include <QString>
#include <QColor>

class Task;
class QLineEdit;
class QDateTimeEdit;
class QTextBrowser;
class QPushButton;

class TaskEditor : public QDialog
{
    Q_OBJECT

public:
    static TaskEditor &instance();
    QHash<QString, QVariant> edit(const Task *);
    QLineEdit *nameEdit_;
    QLineEdit *summaryEdit_;
    QDateTimeEdit *loDateTimeEdit_;
    QDateTimeEdit *hiDateTimeEdit_;
    QTextBrowser *descrEdit_;
    QPushButton *colorEdit_;
    QColor color_;

private:
    TaskEditor();

private slots:
    void editColor();
};

#endif // TASKEDITOR_H
