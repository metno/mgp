#include "rolepanel.h"
#include "role.h"
#include <QLabel>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFormLayout>

RolePanel &RolePanel::instance()
{
    static RolePanel rp;
    return rp;
}

RolePanel::RolePanel()
    : formLayout_(new QFormLayout)
    , nameLabel_(new QLabel)
    , loTimeLabel_(new QLabel)
    , hiTimeLabel_(new QLabel)
    , descrTextBrowser_(new QTextBrowser)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    formLayout_->setLabelAlignment(Qt::AlignRight);
    formLayout_->addRow("Name:", nameLabel_);
    formLayout_->addRow("Begins:", loTimeLabel_);
    formLayout_->addRow("Ends:", hiTimeLabel_);
    descrTextBrowser_->setReadOnly(true);
    descrTextBrowser_->setOpenExternalLinks(true);
    formLayout_->addRow("Description:", descrTextBrowser_);

    QGroupBox *groupBox = new QGroupBox("Role Properties");
    groupBox->setLayout(formLayout_);
    mainLayout->addWidget(groupBox);

    clearContents();
}

static QString formatTime(const QTime &t)
{
    return t.toString("hh:mm");
}

void RolePanel::setContents(const Role *role)
{
    if (role) {
        nameLabel_->setText(role->name());
        loTimeLabel_->setText(formatTime(role->loTime()));
        hiTimeLabel_->setText(formatTime(role->hiTime()));
        descrTextBrowser_->setHtml(role->description());
    } else {
        nameLabel_->setText("");
        loTimeLabel_->setText("");
        hiTimeLabel_->setText("");
        descrTextBrowser_->setHtml("");
    }
}

void RolePanel::clearContents()
{
    setContents(0);
}
