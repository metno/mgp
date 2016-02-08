#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include "common.h"
#include <QWidget>
#include <QHash>
#include <QLineF>
#include <QVariant>
#include <QSharedPointer>
#include <QVector>
#include <QPair>

class QCheckBox;
class QDoubleSpinBox;
class GLWidget;
class QGridLayout;
class QComboBox;
class QSlider;

class Filter : public QObject // ### does this need to be a QObject?
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
    virtual bool isValid() const = 0;
    virtual bool rejected(double, double) const = 0;
    bool rejected(const QPair<double, double> &) const;

    // Returns true and intersection point iff filter intersects great circle segment between given two points.
    virtual bool intersects(const QPair<double, double> &, const QPair<double, double> &, QPair<double, double> *) const = 0;

    PointVectors apply(const PointVector &) const;

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
    virtual bool isValid() const;
    virtual bool rejected(double, double) const;
    virtual bool intersects(const QPair<double, double> &, const QPair<double, double> &, QPair<double, double> *) const;

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
    virtual bool isValid() const;
    virtual bool rejected(double, double) const;
    virtual bool intersects(const QPair<double, double> &, const QPair<double, double> &, QPair<double, double> *) const;

    QDoubleSpinBox *lon1SpinBox_;
    QDoubleSpinBox *lat1SpinBox_;
    QDoubleSpinBox *lon2SpinBox_;
    QDoubleSpinBox *lat2SpinBox_;

    bool firstEndpointDragged_;
    bool validCombination(double, double, double, double) const;
};

class BasePolygon
{
    friend class ControlPanel;

public:
    enum Type { None, Custom, ENOR_FIR, XXXX_FIR, YYYY_FIR, ZZZZ_FIR };
    static QString typeName(Type);

protected:
    BasePolygon(Type, const PointVector & = PointVector());
    static BasePolygon *create(Type);
    Type type_;
    PointVector points_; // first component = longitude in radians, second component = latitude in radians
};

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    static ControlPanel &instance();
    void initialize();
    void open();

    bool isEnabled(Filter::Type) const;
    bool isCurrent(Filter::Type) const;
    bool isValid(Filter::Type) const;
    QVariant value(Filter::Type) const;
    bool rejectedByAnyFilter(double, double) const;
    QVector<QPair<double, double> > filterIntersections(const QPair<double, double> &, const QPair<double, double> &) const;
    bool filtersEditableOnSphere() const;
    void toggleFiltersEditableOnSphere();
    bool startFilterDragging(double, double) const;
    void updateFilterDragging(double, double);

    BasePolygon::Type currentBasePolygonType() const;
    bool basePolygonVisible() const;
    PointVector currentBasePolygonPoints() const;
    int currentCustomBasePolygonPoint(double, double, double);
    bool customBasePolygonEditableOnSphere() const;
    void updateCustomBasePolygonPointDragging(int, double, double);
    void addPointToCustomBasePolygon(int);
    void removePointFromCustomBasePolygon(int);

    bool resultPolygonsLinesVisible() const;
    bool resultPolygonsPointsVisible() const;
    PointVectors resultPolygons() const;

    float ballSizeFrac();

private:
    ControlPanel();
    virtual void keyPressEvent(QKeyEvent *);

    QSlider *bsSlider_;

    QComboBox *basePolygonComboBox_;
    QCheckBox *basePolygonVisibleCheckBox_;

    QHash<BasePolygon::Type, BasePolygon *> basePolygons_;
    QCheckBox *customBasePolygonEditableOnSphereCheckBox_;

    QCheckBox *filtersEditableOnSphereCheckBox_;
    QHash<Filter::Type, Filter *> filters_;
    Filter *currentFilter() const;

    QCheckBox *resultPolygonsLinesVisibleCheckBox_;
    QCheckBox *resultPolygonsPointsVisibleCheckBox_;

private slots:
    void close();
    void updateGLWidget();
    void basePolygonTypeChanged();
};

#endif // CONTROLPANEL_H
