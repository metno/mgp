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
    enum Type { None, E_OF, W_OF, N_OF, S_OF, NE_OF, NW_OF, SE_OF, SW_OF };
    static QString typeName(Type);

protected:
    Filter(Type, QCheckBox *, QCheckBox *);
    virtual QVariant value() const = 0;
    virtual bool startDragging(double, double) = 0;
    virtual void updateDragging(double, double) = 0;
    Type type_;
    QCheckBox *enabledCheckBox_;
    QCheckBox *currCheckBox_;

    bool dragged_;
};

class LonOrLatFilter : public Filter
{
    Q_OBJECT
    friend class ControlPanel;

    LonOrLatFilter(Type, QCheckBox *, QCheckBox *, QDoubleSpinBox *, double);
    static Filter *create(QGridLayout *, int, Type, double);
    virtual QVariant value() const;
    virtual bool startDragging(double, double);
    virtual void updateDragging(double, double);
    QDoubleSpinBox *valSpinBox_;
};

class FreeLineFilter : public Filter {
    Q_OBJECT
    friend class ControlPanel;

    FreeLineFilter(
            Type, QCheckBox *, QCheckBox *, QDoubleSpinBox *, QDoubleSpinBox *, QDoubleSpinBox *, QDoubleSpinBox *, const QLineF &);
    static Filter *create(QGridLayout *, int, Type, const QLineF &);
    virtual QVariant value() const;
    virtual bool startDragging(double, double);
    virtual void updateDragging(double, double);
    QDoubleSpinBox *lon1SpinBox_;
    QDoubleSpinBox *lat1SpinBox_;
    QDoubleSpinBox *lon2SpinBox_;
    QDoubleSpinBox *lat2SpinBox_;

    bool firstEndpointDragged_;
};

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    static ControlPanel &instance();
    void initialize();
    void open();
    bool enabled(Filter::Type) const;
    QVariant value(Filter::Type) const;
    bool filtersEditableOnSphere() const;
    void toggleFiltersEditableOnSphere();
    bool startFilterDragging(double, double) const;
    void updateFilterDragging(double, double);

private:
    ControlPanel();
    virtual void keyPressEvent(QKeyEvent *);

    QCheckBox *filtersEditableOnSphereCheckBox_;
    QHash<Filter::Type, Filter *> filters_;

private slots:
    void apply();
    void close();
    void updateGLWidget();
};

#endif // CONTROLPANEL_H
