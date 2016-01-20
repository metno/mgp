#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>
#include <QHash>
#include <QLineF>
#include <QVariant>

class QCheckBox;
class QDoubleSpinBox;
class GLWidget;
class QGridLayout;

class Filter : public QObject
{
    Q_OBJECT
    friend class ControlPanel;

public:
    enum Type { E_OF, W_OF, N_OF, S_OF, NE_OF, NW_OF, SE_OF, SW_OF };
    static QString typeName(Type);

protected:
    Filter(Type, QCheckBox *, QCheckBox *);
    virtual QVariant value() const = 0;
    Type type_;
    QCheckBox *enabledCheckBox_;
    QCheckBox *currCheckBox_;

protected slots:
    void updateGLWidget();
};

class LonOrLatFilter : public Filter
{
    Q_OBJECT
    friend class ControlPanel;

    LonOrLatFilter(Type, QCheckBox *, QCheckBox *, QDoubleSpinBox *, double);
    static Filter *create(QGridLayout *, int, Type, double);
    virtual QVariant value() const;
    QDoubleSpinBox *valSpinBox_;
};

class FreeLineFilter : public Filter {
    Q_OBJECT
    friend class ControlPanel;

    FreeLineFilter(
            Type, QCheckBox *, QCheckBox *, QDoubleSpinBox *, QDoubleSpinBox *, QDoubleSpinBox *, QDoubleSpinBox *, const QLineF &);
    static Filter *create(QGridLayout *, int, Type, const QLineF &);
    virtual QVariant value() const;
    QDoubleSpinBox *lon1SpinBox_;
    QDoubleSpinBox *lat1SpinBox_;
    QDoubleSpinBox *lon2SpinBox_;
    QDoubleSpinBox *lat2SpinBox_;
};

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    static ControlPanel &instance();
    void open();

private:
    ControlPanel();
    virtual void keyPressEvent(QKeyEvent *);

    QHash<Filter::Type, Filter *> filters_;

private slots:
    void apply();
    void close();
};

#endif // CONTROLPANEL_H
