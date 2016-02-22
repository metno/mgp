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

class QGroupBox;
class QCheckBox;
class QDoubleSpinBox;
class GLWidget;
class QGridLayout;
class QComboBox;
class QSlider;
class TextEdit;
class QPushButton;
class QMouseEvent;

class Filter : public QObject // ### does this need to be a QObject?
{
    Q_OBJECT
    friend class ControlPanel;

public:
    enum Type { None, WI, E_OF, W_OF, N_OF, S_OF, NE_OF_LINE, NW_OF_LINE, SE_OF_LINE, SW_OF_LINE };
    static QString typeName(Type);

protected:
    Filter(Type, QCheckBox *, QCheckBox *);
    virtual QVariant value() const = 0;
    virtual bool startDragging(const QPair<double, double> &) = 0;
    virtual void updateDragging(const QPair<double, double> &) = 0;
    virtual bool isValid() const = 0;
    virtual bool rejected(const QPair<double, double> &) const = 0;

    // Applies the filter to a polygon and returns the result as zero or more polygons within the original area.
    virtual PointVectors apply(const PointVector &) const = 0;

    // Returns all intersection points between the filter and the given great circle segment.
    virtual QVector<QPair<double, double> > intersections(const QPair<double, double> &, const QPair<double, double> &) const = 0;

    // Returns the SIGMET/AIRMET expression corresponding to the filter state.
    virtual QString xmetExpr() const = 0;

    // Sets the filter state from a SIGMET/AIRMET expression. Returns true iff the state was successfully set.
    virtual bool setFromXmetExpr(const QString &, QPair<int, int> *, QPair<int, int> *, QString *) = 0;

    Type type_;
    QCheckBox *enabledCheckBox_;
    QCheckBox *currCheckBox_;

    bool dragged_;
};

class WithinFilter : public Filter
{
    Q_OBJECT
    friend class ControlPanel;

    WithinFilter(Type, QCheckBox *, QCheckBox *, const PointVector &);
    static Filter *create(QGridLayout *, int, Type, const PointVector &);

    virtual QVariant value() const;
    virtual bool startDragging(const QPair<double, double> &);
    virtual void updateDragging(const QPair<double, double> &);
    virtual bool isValid() const;
    virtual bool rejected(const QPair<double, double> &) const;

    virtual PointVectors apply(const PointVector &) const;
    virtual QVector<QPair<double, double> > intersections(const QPair<double, double> &, const QPair<double, double> &) const;
    virtual QString xmetExpr() const;
    virtual bool setFromXmetExpr(const QString &, QPair<int, int> *, QPair<int, int> *, QString *);

    PointVector points_;
};

class LineFilter : public Filter
{
protected:
    LineFilter(Type, QCheckBox *, QCheckBox *);

    // Returns true and intersection point iff filter intersects great circle segment between given two points.
    virtual bool intersects(const QPair<double, double> &, const QPair<double, double> &, QPair<double, double> *) const = 0;

    virtual PointVectors apply(const PointVector &) const;
    virtual QVector<QPair<double, double> > intersections(const QPair<double, double> &, const QPair<double, double> &) const;
};

class LonOrLatFilter : public LineFilter
{
    friend class ControlPanel;

protected:

    LonOrLatFilter(Type, QCheckBox *, QCheckBox *, QDoubleSpinBox *, double);
    static Filter *create(QGridLayout *, int, Type, double);
    virtual QVariant value() const;
    virtual bool startDragging(const QPair<double, double> &);
    virtual void updateDragging(const QPair<double, double> &);
    virtual bool isValid() const;
    virtual bool rejected(const QPair<double, double> &) const;
    virtual bool intersects(const QPair<double, double> &, const QPair<double, double> &, QPair<double, double> *) const;
    virtual QString xmetExpr() const;
    virtual bool setFromXmetExpr(const QString &, QPair<int, int> *, QPair<int, int> *, QString *);

    QDoubleSpinBox *valSpinBox_;
};

class LonFilter : public LonOrLatFilter
{
    friend class ControlPanel;
    friend class LonOrLatFilter;

    LonFilter(Type, QCheckBox *, QCheckBox *, QDoubleSpinBox *, double);
};

class LatFilter : public LonOrLatFilter
{
    friend class ControlPanel;
    friend class LonOrLatFilter;

    LatFilter(Type, QCheckBox *, QCheckBox *, QDoubleSpinBox *, double);
    virtual PointVectors apply(const PointVector &) const;
};

class FreeLineFilter : public LineFilter {
    friend class ControlPanel;

    FreeLineFilter(
            Type, QCheckBox *, QCheckBox *, QDoubleSpinBox *, QDoubleSpinBox *, QDoubleSpinBox *, QDoubleSpinBox *, const QLineF &);
    static Filter *create(QGridLayout *, int, Type, const QLineF &);
    virtual QVariant value() const;
    virtual bool startDragging(const QPair<double, double> &);
    virtual void updateDragging(const QPair<double, double> &);
    virtual bool isValid() const;
    virtual bool rejected(const QPair<double, double> &) const;
    virtual bool intersects(const QPair<double, double> &, const QPair<double, double> &, QPair<double, double> *) const;
    virtual QString xmetExpr() const;
    virtual bool setFromXmetExpr(const QString &, QPair<int, int> *, QPair<int, int> *, QString *);

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

    PointVector WIFilterPoints() const;
    QVariant value(Filter::Type) const;

    bool rejectedByAnyFilter(const QPair<double, double> &) const;
    QVector<QPair<double, double> > filterIntersections(const QPair<double, double> &, const QPair<double, double> &) const;
    bool filtersEditableOnSphere() const;
    void toggleFiltersEditableOnSphere();
    bool startFilterDragging(const QPair<double, double> &) const;
    void updateFilterDragging(const QPair<double, double> &);

    BasePolygon::Type currentBasePolygonType() const;
    bool basePolygonLinesVisible() const;
    bool basePolygonPointsVisible() const;
    bool basePolygonIntersectionsVisible() const;
    PointVector currentBasePolygonPoints() const;
    int currentWIFilterPoint(const QPair<double, double> &, double);
    int currentCustomBasePolygonPoint(const QPair<double, double> &, double);
    bool customBasePolygonEditableOnSphere() const;
    void updateWIFilterPointDragging(int, const QPair<double, double> &);
    void updateCustomBasePolygonPointDragging(int, const QPair<double, double> &);
    void addPointToWIFilter(int);
    void addPointToCustomBasePolygon(int);
    void removePointFromWIFilter(int);
    void removePointFromCustomBasePolygon(int);
    bool currentBasePolygonIsClockwise() const;
    bool withinCurrentBasePolygon(const QPair<double, double> &) const;

    bool resultPolygonsLinesVisible() const;
    bool resultPolygonsPointsVisible() const;
    PointVectors resultPolygons() const;
    void updateResultPolygonsGroupBoxTitle(int);

    float ballSizeFrac();

private:
    ControlPanel();
    virtual void keyPressEvent(QKeyEvent *);

    QSlider *bsSlider_;

    QComboBox *basePolygonComboBox_;
    QCheckBox *basePolygonLinesVisibleCheckBox_;
    QCheckBox *basePolygonPointsVisibleCheckBox_;
    QCheckBox *basePolygonIntersectionsVisibleCheckBox_;

    QHash<BasePolygon::Type, BasePolygon *> basePolygons_;
    QCheckBox *customBasePolygonEditableOnSphereCheckBox_;

    QCheckBox *filtersEditableOnSphereCheckBox_;
    QHash<Filter::Type, Filter *> filters_;
    Filter *currentFilter() const;

    QGroupBox *resultPolygonsGroupBox_;
    QCheckBox *resultPolygonsLinesVisibleCheckBox_;
    QCheckBox *resultPolygonsPointsVisibleCheckBox_;

    TextEdit *xmetExprEdit_;
    QPushButton *setFiltersFromXmetExprButton_;
    QString setFiltersFromXmetExprButtonText_;

    void updatePolygonPointDragging(PointVector &, int, const QPair<double, double> &);
    void addPointToPolygon(PointVector &, int);
    void removePointFromPolygon(PointVector &, int);

private slots:
    void close();
    void updateGLWidget();
    void basePolygonTypeChanged();
    void setXmetExprFromFilters();
    void setFiltersFromXmetExpr();
    void handleXmetExprChanged();
};

#endif // CONTROLPANEL_H
