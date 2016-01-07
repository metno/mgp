#ifndef ROLEPANEL_H
#define ROLEPANEL_H

#include <QWidget>

class Role;
class QFormLayout;
class QLabel;
class QTextBrowser;

class RolePanel : public QWidget
{
    Q_OBJECT

public:
    static RolePanel &instance();
    void setContents(const Role *);
    void clearContents();

private:
    RolePanel();
    QFormLayout *formLayout_;
    QLabel *nameLabel_;
    QLabel *loTimeLabel_;
    QLabel *hiTimeLabel_;
    QTextBrowser *descrTextBrowser_;
};

#endif // ROLEPANEL_H
