#ifndef ROLEEDITOR_H
#define ROLEEDITOR_H

#include <QDialog>
#include <QHash>
#include <QVariant>

class Role;
class QLineEdit;
class QTimeEdit;
class QTextBrowser;

class RoleEditor : public QDialog
{
    Q_OBJECT

public:
    static RoleEditor &instance();
    QHash<QString, QVariant> edit(const Role *);
    QLineEdit *nameEdit_;
    QTimeEdit *loTimeEdit_;
    QTimeEdit *hiTimeEdit_;
    QTextBrowser *descrEdit_;

private:
    RoleEditor();
};

#endif // ROLEEDITOR_H
