#ifndef XMETAREAEDITDIALOG_H
#define XMETAREAEDITDIALOG_H

#include <QLabel>
#include <QScrollArea>
#include <QDialog>
#include <QMetaType>

class QToolButton;
class QComboBox;
class QSpinBox;
class QTextEdit;
class QVBoxLayout;

class SelLabel : public QLabel
{
    Q_OBJECT
public:
    SelLabel(int);
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

    PointEdit(LonDir = E, int = 0, int = 0, LatDir = N, int = 0, int = 0, QWidget * = 0);
    PointEdit(const PointEdit &);

    void setLonDir(LonDir) { }
    LonDir lonDir() const { return E; }
    void setLonDeg(int) { }
    int lonDeg() const { return -1; }
    void setLonSec(int) { }
    int lonSec() const { return -1; }

    void setLatDir(LatDir) { }
    LatDir latDir() const { return S; }
    void setLatDeg(int) { }
    int latDeg() const { return -1; }
    void setLatSec(int) { }
    int latSec() const { return -1; }

    void setSelected(bool);
    bool isSelected() const { return selected_; }

    void setLabelText(const QString &);

private:
    void init(LonDir, int, int, LatDir, int, int);

    SelLabel *selLabel_;
    bool selected_;

    QComboBox *lonDirEdit_;
    QSpinBox *lonDegEdit_;
    QSpinBox *lonSecEdit_;
    QComboBox *latDirEdit_;
    QSpinBox *latDegEdit_;
    QSpinBox *latSecEdit_;

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
    XMETAreaEditDialog(QTextEdit *, QWidget *parent = 0);

public slots:
    void edit();

private:
    QTextEdit *xmetAreaEdit_;
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
