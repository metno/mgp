#include "misc.h"
#include <QString>
#include <QMessageBox>

bool confirm(const QString &text)
{
    QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    return (msgBox.exec() == QMessageBox::Yes);
}
