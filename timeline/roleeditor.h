#ifndef ROLEEDITOR_H
#define ROLEEDITOR_H

#include <QDialog>
#include <QHash>

class Role;
class QLineEdit;
class QTextBrowser;

class RoleEditor : public QDialog
{
    Q_OBJECT

public:
    static RoleEditor &instance();
    QHash<QString, QString> edit(const Role *);
    QLineEdit *nameEdit_;
    QTextBrowser *descrEdit_;

private:
    RoleEditor();
};

#endif // ROLEEDITOR_H
