#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include "common.h"
#include <QWidget>
#include <QHash>
#include <QList>
#include <QLineF>
#include <QVariant>
#include <QSharedPointer>
#include <QVector>
#include <QPair>
#include <QDialog>

#include "mgp.h"

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
class QTabWidget;
class QTextEdit;

class FilterControlBase : public QObject
{
    Q_OBJECT
    friend class ControlPanel;

protected:
    FilterControlBase(mgp::FilterBase *, QCheckBox *, QCheckBox *);

    mgp::Filter filter_;
    mgp::Filter filter() const { return filter_; }
    virtual void update() = 0;

    QCheckBox *enabledCheckBox_;
    QCheckBox *currCheckBox_;

    virtual QVariant value() const = 0;

    virtual bool startDragging(const mgp::Point &) = 0;
    virtual void updateDragging(const mgp::Point &) = 0;
    bool dragged_;
};

class WithinFilterControl : public FilterControlBase
{
    friend class ControlPanel;

    WithinFilterControl(mgp::WithinFilter *, QCheckBox *, QCheckBox *);
    static FilterControlBase *create(QGridLayout *, int, mgp::WithinFilter *);
    void update();

    virtual QVariant value() const;

    virtual bool startDragging(const mgp::Point &);
    virtual void updateDragging(const mgp::Point &);
};

class LonOrLatFilterControl : public FilterControlBase
{
    Q_OBJECT

    friend class ControlPanel;

    LonOrLatFilterControl(mgp::LonOrLatFilter *, QCheckBox *, QCheckBox *, QDoubleSpinBox *);
    static FilterControlBase *create(QGridLayout *, int, mgp::LonOrLatFilter *);
    void update();

    QSharedPointer<mgp::LonOrLatFilter> lonOrLatFilter_;
    QDoubleSpinBox *valSpinBox_;
    virtual QVariant value() const;

    virtual bool startDragging(const mgp::Point &);
    virtual void updateDragging(const mgp::Point &);

private slots:
    void handleSpinBoxValueChanged();
};

class FreeLineFilterControl : public FilterControlBase
{
Q_OBJECT

    friend class ControlPanel;

    FreeLineFilterControl(
            mgp::FreeLineFilter *, QCheckBox *, QCheckBox *, QDoubleSpinBox *, QDoubleSpinBox *, QDoubleSpinBox *, QDoubleSpinBox *);
    static FilterControlBase *create(QGridLayout *, int, mgp::FreeLineFilter *);
    void update();

    QSharedPointer<mgp::FreeLineFilter> freeLineFilter_;
    QDoubleSpinBox *lon1SpinBox_;
    QDoubleSpinBox *lat1SpinBox_;
    QDoubleSpinBox *lon2SpinBox_;
    QDoubleSpinBox *lat2SpinBox_;
    virtual QVariant value() const;

    virtual bool startDragging(const mgp::Point &);
    virtual void updateDragging(const mgp::Point &);
    bool firstEndpointDragged_;

private slots:
    void handleSpinBoxValueChanged();
};

class BasePolygon
{
    friend class ControlPanel;

public:
    enum Type { None, Custom, ENOR_FIR, XXXX_FIR, YYYY_FIR, ZZZZ_FIR };
    static QString typeName(Type);

protected:
    BasePolygon(Type, const mgp::Polygon & = mgp::Polygon());
    static BasePolygon *create(Type);
    Type type_;
    mgp::Polygon polygon_; // first component = longitude in radians, second component = latitude in radians
};

struct FilterTabInfo
{
    QWidget *page_;
    QString baseText_;
    QList<mgp::FilterBase::Type> filterTypes_;
    FilterTabInfo(QWidget *page, const QString &baseText, const QList<mgp::FilterBase::Type> &filterTypes) :
        page_(page), baseText_(baseText), filterTypes_(filterTypes) {}
};

class ResultPolygonsExportPanel : public QDialog
{
    Q_OBJECT
public:
    ResultPolygonsExportPanel();
    void setPolygons(const mgp::Polygons &);
private:
    QTextEdit *textEdit_;
};

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    static ControlPanel &instance();
    void initialize();
    void open();

    bool isEnabled(mgp::FilterBase::Type) const;
    bool isCurrent(mgp::FilterBase::Type) const;
    bool isValid(mgp::FilterBase::Type) const;

    mgp::Polygon WIFilterPolygon() const;
    QVariant value(mgp::FilterBase::Type) const;

    bool rejectedByAnyFilter(const mgp::Point &) const;
    QVector<mgp::Point> filterIntersections(const mgp::Polygon &) const;
    bool filtersEditableOnSphere() const;
    void toggleFiltersEditableOnSphere();
    bool filterLinesVisible() const;
    bool filterPointsVisible() const;
    bool startFilterDragging(const mgp::Point &) const;
    void updateFilterDragging(const mgp::Point &);

    BasePolygon::Type currentBasePolygonType() const;
    bool basePolygonLinesVisible() const;
    bool basePolygonPointsVisible() const;
    bool basePolygonIntersectionsVisible() const;
    mgp::Polygon currentBasePolygon() const;
    int currentWIFilterPoint(const mgp::Point &, double);
    int currentCustomBasePolygonPoint(const mgp::Point &, double);
    bool customBasePolygonEditableOnSphere() const;
    void updateWIFilterPointDragging(int, const mgp::Point &);
    void updateCustomBasePolygonPointDragging(int, const mgp::Point &);
    void addPointToWIFilter(int);
    void addPointToCustomBasePolygon(int);
    void removePointFromWIFilter(int);
    void removePointFromCustomBasePolygon(int);
    bool currentBasePolygonIsClockwise() const;
    bool withinCurrentBasePolygon(const mgp::Point &) const;

    bool resultPolygonsLinesVisible() const;
    bool resultPolygonsPointsVisible() const;
    mgp::Polygons resultPolygons() const;
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
    QCheckBox *filterLinesVisibleCheckBox_;
    QCheckBox *filterPointsVisibleCheckBox_;
    QHash<mgp::FilterBase::Type, FilterControlBase *> filterControls_;
    FilterControlBase *currentFilter() const;
    mgp::Filters enabledAndValidFilters() const;

    QSharedPointer<mgp::WithinFilter> wiFilter_;

    QGroupBox *resultPolygonsGroupBox_;
    QCheckBox *resultPolygonsLinesVisibleCheckBox_;
    QCheckBox *resultPolygonsPointsVisibleCheckBox_;

    TextEdit *xmetExprEdit_;
    QPushButton *setFiltersFromXmetExprButton_;
    QString setFiltersFromXmetExprButtonText_;
    QCheckBox *autoSetFiltersCheckBox_;

    void updatePolygonPointDragging(const mgp::Polygon &, int, const mgp::Point &);
    void addPointToPolygon(const mgp::Polygon &, int);
    void removePointFromPolygon(const mgp::Polygon &, int);

    QTabWidget *filterTabWidget_;
    QList<FilterTabInfo> filterTabInfos_;

    ResultPolygonsExportPanel *resPolysExportPanel_;

public slots:
    void updateGLWidget();

private slots:
    void close();
    void updateFilterTabTexts();
    void basePolygonTypeChanged();
    void exportResultPolygons();
    void setXmetExprFromFilters();
    void setFiltersFromXmetExpr();
    void handleXmetExprChanged();
    void customBasePolygonEditableOnSphereCheckBoxStateChanged();
    void filtersEditableOnSphereCheckBoxStateChanged();
};

#endif // CONTROLPANEL_H
