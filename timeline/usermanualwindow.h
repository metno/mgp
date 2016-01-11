#ifndef USERMANUALWINDOW_H
#define USERMANUALWINDOW_H

#include <QWidget>

class UserManualWindow : public QWidget
{
public:
    static UserManualWindow &instance();

private:
    UserManualWindow();
    void keyPressEvent(QKeyEvent *);
};

#endif // USERMANUALWINDOW_H
