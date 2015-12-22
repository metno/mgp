#include "taskeditor.h"
#include "task.h"
#include <QLineEdit>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>

TaskEditor &TaskEditor::instance()
{
    static TaskEditor te;
    return te;
}

TaskEditor::TaskEditor()
{
    setWindowTitle("Edit Task Properties");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    formLayout->setLabelAlignment(Qt::AlignRight);

    nameEdit_ = new QLineEdit;
    formLayout->addRow("Name:", nameEdit_);
    summaryEdit_ = new QLineEdit;
    formLayout->addRow("Summary:", summaryEdit_);
    descrEdit_ = new QLineEdit;
    formLayout->addRow("Description:", descrEdit_);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), SLOT(accept()));
    mainLayout->addWidget(buttonBox);
}

QHash<QString, QString> TaskEditor::edit(const Task *task)
{
    // initialize fields
    nameEdit_->setText(task->name());
    summaryEdit_->setText("<dummy summary>" /*task->summary()*/);
    descrEdit_->setText("<dummy description>" /*task->description()*/);

    // open dialog and return any edited values
    if (exec() == QDialog::Accepted) {
        QHash<QString, QString> values;
        values.insert("name", nameEdit_->text().trimmed());
        values.insert("summary", summaryEdit_->text().trimmed());
        values.insert("description", descrEdit_->text().trimmed());
        return values;
    }
    return QHash<QString, QString>();
}
