#include "taskpanel.h"
#include "task.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>

TaskPanel &TaskPanel::instance()
{
    static TaskPanel tp;
    return tp;
}

TaskPanel::TaskPanel()
    : formLayout_(new QFormLayout)
    , nameLabel_(new QLabel)
    , summaryLabel_(new QLabel)
    , descrLabel_(new QLabel)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    formLayout_->setLabelAlignment(Qt::AlignRight);
    formLayout_->addRow("Name:", nameLabel_);
    formLayout_->addRow("Summary:", summaryLabel_);
    formLayout_->addRow("Description:", descrLabel_);

    QGroupBox *groupBox = new QGroupBox("Task Properties");
    groupBox->setLayout(formLayout_);
    mainLayout->addWidget(groupBox);

    clearContents();
}

void TaskPanel::setContents(const Task *task)
{
    if (task) {
        nameLabel_->setText(task->name());
        summaryLabel_->setText(task->summary());
        descrLabel_->setText(task->description());
    } else {
        nameLabel_->setText("");
        summaryLabel_->setText("");
        descrLabel_->setText("");
    }
}

void TaskPanel::clearContents()
{
    setContents(0);
}
