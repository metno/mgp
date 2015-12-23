#include "roleeditor.h"
#include "role.h"
#include <QLineEdit>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>

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
    descrEdit_ = new QTextBrowser;
    descrEdit_->setReadOnly(false);
    formLayout->addRow("Description:", descrEdit_);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), SLOT(accept()));
    mainLayout->addWidget(buttonBox);

    resize(800, 300);
}

QHash<QString, QString> RoleEditor::edit(const Role *role)
{
    // initialize fields
    nameEdit_->setText(role->name());
    descrEdit_->setPlainText(role->description());

    // open dialog and return any edited values
    if (exec() == QDialog::Accepted) {
        QHash<QString, QString> values;
        values.insert("name", nameEdit_->text().trimmed());
        values.insert("description", descrEdit_->toPlainText().trimmed());
        return values;
    }
    return QHash<QString, QString>();
}
