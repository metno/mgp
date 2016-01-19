#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    static ControlPanel &instance();
    void open();

private:
    ControlPanel();

private slots:
    void apply();
    void close();
};

#endif // CONTROLPANEL_H
