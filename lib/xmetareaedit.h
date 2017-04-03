#ifndef XMETAREAEDIT_H
#define XMETAREAEDIT_H

#include "mgp.h"
#include "xmetareaeditdialog.h"
#include <QTextEdit>
#include <QBitArray>

//! This class extends a QTextEdit with validation and highlighting of a SIGMET/AIRMET area expression.
// ### WARNING: For now, this class is defined outside the mgp namespace in order not to confuse the Qt meta object compiler.
class XMETAreaEdit : public QTextEdit
{
    Q_OBJECT
public:
    /**
     * @param wiExclusive       See documentation in filtersFromXmetExpr().
     * @param wiOnly            See documentation in filtersFromXmetExpr().
     * @param wiKeywordImplicit See documentation in filtersFromXmetExpr().
     */
    XMETAreaEdit(QWidget *parent = 0, bool wiExclusive = true, bool wiOnly = true, bool wiKeywordImplicit = false);
//    XMETAreaEdit(QWidget *parent = 0, bool wiExclusive = true, bool wiOnly = false, bool wiKeywordImplicit = false);

    /**
     * @param text              Initial text.
     * @param wiExclusive       See documentation in filtersFromXmetExpr().
     * @param wiOnly            See documentation in filtersFromXmetExpr().
     * @param wiKeywordImplicit See documentation in filtersFromXmetExpr().
     */
    XMETAreaEdit(const QString &text, QWidget *parent = 0, bool wiExclusive = true, bool wiOnly = true, bool wiKeywordImplicit = false);
//    XMETAreaEdit(const QString &text, QWidget *parent = 0, bool wiExclusive = true, bool wiOnly = false, bool wiKeywordImplicit = false);

    /**
     * Updates filters and highlighting.
     * \return True iff each non-whitespace character is part of a matched range.
     */
    bool update();

    /**
     * Extracts the complete filters found in the text
     * \return The sequence of complete filters ordered by occurrence. If a given filter type (like 'S OF ...') appears more than
     * once, only the first occurrence is returned.
     */
    mgp::Filters filters();

    /**
     * Extracts the first FIR found in the text
     * \return The code for the first supported FIR found in the text, otherwise the code for an unsupported FIR.
     */
    mgp::FIR::Code fir();

    /**
     * Enables or disables WI exclusive mode (whether the WI filter is allowed to occur together with other filter types).
     */
    void setWIExclusive(bool);

    /**
     * Gets whether WI exclusive mode is enabled.
     */
    bool wiExclusive() const;

    /**
     * Enables or disables WI only mode (whether the WI filter is the only filter allowed).
     */
    void setWIOnly(bool);

    /**
     * Gets whether WI only mode is enabled.
     */
    bool wiOnly() const;

    /**
     * Sets whether the 'WI' keyword is implicit. If set, the presence of the 'WI' keyword would be considered
     * a syntax error.
     */
    void setWIKeywordImplicit(bool);

    /**
     * Gets whether the 'WI' keyword is implicit.
     */
    bool wiKeywordImplicit() const;

private:
    void init();
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void contextMenuEvent(QContextMenuEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    void resetHighlighting();
    void addMatchedRange(const QPair<int, int> &);
    void addIncompleteRange(const QPair<int, int> &, const QString &);
    void showHighlighting();
    QBitArray matched_;
    QBitArray incomplete_;
    QHash<int, QString> reason_;
    mgp::Filters filters_;
    mgp::FIR::Code fir_;
    bool wiExclusive_;
    bool wiOnly_;
    bool wiKeywordImplicit_;
    QAction *dialogEditAction_;
    XMETAreaEditDialog *xmetAreaEditDialog_;

private slots:
    void editInDialog();
};

#endif // XMETAREAEDIT_H
