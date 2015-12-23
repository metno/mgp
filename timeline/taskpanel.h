#ifndef TASKPANEL_H
#define TASKPANEL_H

#include <QWidget>

class Task;
class QFormLayout;
class QLabel;
class QTextBrowser;

class TaskPanel : public QWidget
{
    Q_OBJECT

public:
    static TaskPanel &instance();
    void setContents(const Task *);
    void clearContents();

private:
    TaskPanel();
    QFormLayout *formLayout_;
    QLabel *nameLabel_;
    QLabel *summaryLabel_;
    QTextBrowser *descrLabel_;
};

#endif // TASKPANEL_H
