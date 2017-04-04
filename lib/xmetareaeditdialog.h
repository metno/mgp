#ifndef XMETAREAEDITDIALOG_H
#define XMETAREAEDITDIALOG_H

#include <QLabel>
#include <QScrollArea>
#include <QDialog>
#include <QMetaType>

class XMETAreaEdit;
class QToolButton;
class QComboBox;
class QSpinBox;
class QVBoxLayout;

class SelLabel : public QLabel
{
    Q_OBJECT
public:
    SelLabel();
private:
    void mousePressEvent(QMouseEvent *);
signals:
    void mouseClicked(QMouseEvent *);
};

class PointEdit : public QWidget
{
    Q_OBJECT
public:
    enum LonDir { E, W };
    enum LatDir { N, S };

    PointEdit(int = E, int = 0, int = 0, int = N, int = 0, int = 0, QWidget * = 0);
    PointEdit(const PointEdit &);

    int lonDir() const;
    int lonDeg() const;
    int lonSec() const;

    int latDir() const;
    int latDeg() const;
    int latSec() const;

    void setSelected(bool);
    bool isSelected() const { return selected_; }

    void setLabelText(const QString &);

private:
    void init(int, int, int, int, int, int);

    SelLabel *selLabel_;
    bool selected_;

    QLabel *spacingLabel1_;
    QLabel *spacingLabel2_;

    QComboBox *lonDirEdit_;
    QSpinBox *lonDegEdit_;
    QSpinBox *lonSecEdit_;
    QComboBox *latDirEdit_;
    QSpinBox *latDegEdit_;
    QSpinBox *latSecEdit_;

private slots:
    void handleLonSecValueChanged(int);
    void handleLatSecValueChanged(int);

signals:
    void mouseClicked(QMouseEvent *);
};

Q_DECLARE_METATYPE(PointEdit::LonDir)
Q_DECLARE_METATYPE(PointEdit::LatDir)

class ScrollArea : public QScrollArea
{
public:
    ScrollArea(QWidget * = 0);
private:
    void keyPressEvent(QKeyEvent *);
};

class XMETAreaEditDialog : public QDialog
{
    Q_OBJECT
public:
    XMETAreaEditDialog(XMETAreaEdit *, QWidget *parent = 0);

public slots:
    void edit(bool);

private:
    XMETAreaEdit *xmetAreaEdit_;
    QVBoxLayout *pointsLayout_; // ### renamed from layersLayout_
    ScrollArea *scrollArea_;
    QToolButton *addNewButton_; // ### renamed from addEmptyButton_
    QToolButton *duplicateSelectedButton_;
    QToolButton *removeSelectedButton_;
    QToolButton *moveSelectedUpButton_;
    QToolButton *moveSelectedDownButton_;
    QToolButton *createToolButton(const QIcon &, const QString &, const char *) const;
    void initialize(PointEdit *); // ### PointEdit renamed from Layer
    int selectedIndex() const;
    PointEdit *selected();
    void selectIndex(int);
    void select(PointEdit *);
    PointEdit *atIndex(int);
    void ensureVisible(PointEdit *);
    void ensureSelectedVisible();
    void duplicate(PointEdit *);
    void remove(PointEdit *);
    void remove(int);
    void moveUp(PointEdit *);
    void moveUp(int);
    void moveDown(PointEdit *);
    void moveDown(int);

private slots:
    void mouseClicked(QMouseEvent *);
    void ensureSelectedVisibleTimeout();
    void addNew();
    void duplicateSelected();
    void removeSelected();
    void moveSelectedUp();
    void moveSelectedDown();
    void updateButtonsAndLabels();
};

#endif // XMETAREAEDITDIALOG_H
