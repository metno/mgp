#include "taskeditor.h"
#include "task.h"
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QTextBrowser>
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
    //formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    nameEdit_ = new QLineEdit;
    formLayout->addRow("Name:", nameEdit_);

    summaryEdit_ = new QLineEdit;
    formLayout->addRow("Summary:", summaryEdit_);

    loDateTimeEdit_ = new QDateTimeEdit;
    loDateTimeEdit_->setDisplayFormat("yyyy-MM-dd hh:mm");
    formLayout->addRow("Begins:", loDateTimeEdit_);

    hiDateTimeEdit_ = new QDateTimeEdit;
    hiDateTimeEdit_->setDisplayFormat("yyyy-MM-dd hh:mm");
    formLayout->addRow("Ends:", hiDateTimeEdit_);

    descrEdit_ = new QTextBrowser;
    descrEdit_->setReadOnly(false);
    formLayout->addRow("Description:", descrEdit_);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), SLOT(accept()));
    mainLayout->addWidget(buttonBox);

    resize(800, 300);
}

QHash<QString, QVariant> TaskEditor::edit(const Task *task)
{
    // initialize fields
    nameEdit_->setText(task->name());
    summaryEdit_->setText(task->summary());
    loDateTimeEdit_->setDateTime(task->loDateTime());
    hiDateTimeEdit_->setDateTime(task->hiDateTime());
    descrEdit_->setPlainText(task->description());

    // open dialog and return any edited values
    if (exec() == QDialog::Accepted) {
        QHash<QString, QVariant> values;
        values.insert("name", nameEdit_->text().trimmed());
        values.insert("summary", summaryEdit_->text().trimmed());
        values.insert("loDateTime", loDateTimeEdit_->dateTime());
        values.insert("hiDateTime", hiDateTimeEdit_->dateTime());
        values.insert("description", descrEdit_->toPlainText().trimmed());
        return values;
    }
    return QHash<QString, QVariant>();
}
