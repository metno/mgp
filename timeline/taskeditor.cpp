#include "taskeditor.h"
#include "task.h"
#include "common.h"
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QColorDialog>

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

    colorEdit_ = new QPushButton("Edit");
    connect(colorEdit_, SIGNAL(clicked()), SLOT(editColor()));
    formLayout->addRow("Color:", colorEdit_);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), SLOT(accept()));
    mainLayout->addWidget(buttonBox);

    resize(800, 300);
}

static QString formatDateTime(const QDateTime &dt)
{
    return dt.toString("yyyy-MM-dd hh:mm");
}

QHash<QString, QVariant> TaskEditor::edit(const Task *task)
{
    // initialize fields
    nameEdit_->setText(task->name());
    summaryEdit_->setText(task->summary());
    loDateTimeEdit_->setDateTime(task->loDateTime());
    hiDateTimeEdit_->setDateTime(task->hiDateTime());
    descrEdit_->setPlainText(task->description());
    if (task->color().isValid())
        colorEdit_->setStyleSheet(QString("background-color: %1").arg(task->color().name()));

    // open dialog and return any edited values
    if (exec() == QDialog::Accepted) {
        const QDateTime loDateTime = loDateTimeEdit_->dateTime();
        const QDateTime hiDateTime = hiDateTimeEdit_->dateTime();
        const QDateTime minDateTime(QDate(1970, 1, 1), QTime(0, 0), Qt::UTC); // see https://en.wikipedia.org/wiki/Unix_time
        const QDateTime maxDateTime(QDate(2038, 1, 19), QTime(03, 14), Qt::UTC); // see https://en.wikipedia.org/wiki/Year_2038_problem

        if (loDateTime < minDateTime) {
            QMessageBox::warning(
                        0, "Warning",
                        QString("WARNING: %1 earlier than %2; operation is canceled")
                        .arg(formatDateTime(loDateTime))
                        .arg(formatDateTime(minDateTime)));
            return QHash<QString, QVariant>();
        }

        if (hiDateTime > maxDateTime) {
            QMessageBox::warning(
                        0, "Warning",
                        QString("WARNING: %1 later than %2; operation is canceled")
                        .arg(formatDateTime(hiDateTime))
                        .arg(formatDateTime(maxDateTime)));
            return QHash<QString, QVariant>();
        }

        if (loDateTime >= hiDateTime) {
            QMessageBox::warning(
                        0, "Warning",
                        QString("WARNING: %1 not earlier than %2; operation is canceled")
                        .arg(formatDateTime(loDateTime))
                        .arg(formatDateTime(hiDateTime)));
            return QHash<QString, QVariant>();
        }

        QHash<QString, QVariant> values;
        values.insert("name", nameEdit_->text().trimmed());
        values.insert("summary", summaryEdit_->text().trimmed());
        values.insert("loDateTime", loDateTimeEdit_->dateTime());
        values.insert("hiDateTime", hiDateTimeEdit_->dateTime());
        values.insert("description", descrEdit_->toPlainText().trimmed());
        values.insert("color", color_);
        return values;
    }
    return QHash<QString, QVariant>();
}

void TaskEditor::editColor()
{
    color_ = QColorDialog::getColor();
    if (color_.isValid())
        colorEdit_->setStyleSheet(QString("background-color: %1").arg(color_.name()));
}
