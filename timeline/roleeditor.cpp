#include "roleeditor.h"
#include "role.h"
#include <QLineEdit>
#include <QTimeEdit>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>

RoleEditor &RoleEditor::instance()
{
    static RoleEditor re;
    return re;
}

RoleEditor::RoleEditor()
{
    setWindowTitle("Edit Role Properties");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    formLayout->setLabelAlignment(Qt::AlignRight);
    //formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    nameEdit_ = new QLineEdit;
    formLayout->addRow("Name:", nameEdit_);

    loTimeEdit_ = new QTimeEdit;
    loTimeEdit_->setDisplayFormat("hh:mm");
    formLayout->addRow("Begins:", loTimeEdit_);

    hiTimeEdit_ = new QTimeEdit;
    hiTimeEdit_->setDisplayFormat("hh:mm");
    formLayout->addRow("Ends:", hiTimeEdit_);

    descrEdit_ = new QTextBrowser;
    descrEdit_->setReadOnly(false);
    formLayout->addRow("Description:", descrEdit_);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), SLOT(accept()));
    mainLayout->addWidget(buttonBox);

    resize(800, 300);
}

QHash<QString, QVariant> RoleEditor::edit(const Role *role)
{
    // initialize fields
    nameEdit_->setText(role->name());
    loTimeEdit_->setTime(role->loTime());
    hiTimeEdit_->setTime(role->hiTime());
    descrEdit_->setPlainText(role->description());

    // open dialog and return any edited values
    if (exec() == QDialog::Accepted) {
        const QTime loTime = loTimeEdit_->time();
        const QTime hiTime = hiTimeEdit_->time();
        if (loTime >= hiTime) {
            QMessageBox::warning(
                        0, "Warning",
                        QString("WARNING: %1 not earlier than %2; operation is canceled")
                        .arg(loTime.toString("hh:mm"))
                        .arg(hiTime.toString("hh:mm")));
            return QHash<QString, QVariant>();
        }

        QHash<QString, QVariant> values;
        values.insert("name", nameEdit_->text().trimmed());
        values.insert("loTime", loTimeEdit_->time());
        values.insert("hiTime", hiTimeEdit_->time());
        values.insert("description", descrEdit_->toPlainText().trimmed());
        return values;
    }
    return QHash<QString, QVariant>();
}
