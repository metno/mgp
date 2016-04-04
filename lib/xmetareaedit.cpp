#include "mgp.h"
#include <QMouseEvent>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>
#include <QToolTip>
#include <QDebug>

MGP_BEGIN_NAMESPACE

XMETAreaEdit::XMETAreaEdit(QWidget *parent)
    : QTextEdit(parent)
{
    init();
}

XMETAreaEdit::XMETAreaEdit(const QString &text, QWidget *parent)
    : QTextEdit(text, parent)
{
    init();
}

void XMETAreaEdit::init()
{
    setMouseTracking(true);
}

void XMETAreaEdit::resetHighlighting()
{
    setPlainText(toPlainText());
    matched_ = QBitArray(toPlainText().size(), false);
    incomplete_ = QBitArray(toPlainText().size(), false);
    reason_.clear();
}

void XMETAreaEdit::addMatchedRange(const QPair<int, int> &range)
{
    matched_.fill(true, range.first, range.second + 1);
}

void XMETAreaEdit::addIncompleteRange(const QPair<int, int> &range, const QString &reason)
{
    incomplete_.fill(true, range.first, range.second + 1);
    for (int i = range.first; i <= range.second; ++i)
        reason_.insert(i, reason);
}

void XMETAreaEdit::showHighlighting()
{
    const QString defaultColor("#f00");
    const QString defaultBGColor("#fff");
    const QString defaultWeight("regular");

    //const QString matchColor("#088");
    const QString matchColor("#000");
    const QString matchBGColor("#fff");
    //const QString matchWeight("bold");
    const QString matchWeight("regular");

    //const QString incomColor("#088");
    const QString incomColor("#000");
    const QString incomBGColor("#ff0");
    //const QString incomWeight("bold");
    const QString incomWeight("regular");

    QString html;
    const QString text(toPlainText());
    if (text.isEmpty()|| matched_.isEmpty())
        return;

    Q_ASSERT(!incomplete_.isEmpty());

    bool prevMatch = matched_.testBit(0);
    bool prevIncom = incomplete_.testBit(0);

    for (int i = 0; i < text.size(); ++i) {
        const int currMatch = matched_.testBit(i);
        const int currIncom = incomplete_.testBit(i);
        if ((i == 0) || (prevMatch != currMatch) || (prevIncom != currIncom)) {
            if (i > 0)
                html += "</span>"; // end current span

            // start new span, prioritizing incompleteness
            QString color;
            QString bgcolor;
            QString weight;
            if (prevIncom != currIncom) {
                color = currIncom ? incomColor : defaultColor;
                bgcolor = currIncom ? incomBGColor : defaultBGColor;
                weight = currIncom ? incomWeight : defaultWeight;
            } else if (prevMatch != currMatch) {
                color = currMatch ? matchColor : defaultColor;
                bgcolor = currMatch ? matchBGColor : defaultBGColor;
                weight = currMatch ? matchWeight : defaultWeight;
            } else {
                Q_ASSERT(i == 0);
                color = currIncom ? incomColor : (currMatch ? matchColor : defaultColor);
                bgcolor = currIncom ? incomBGColor : (currMatch ? matchBGColor : defaultBGColor);
                weight = currIncom ? incomWeight : (currMatch ? matchWeight : defaultWeight);
            }
            html += QString("<span style='color:%1; background-color:%2; font-weight:%3'>").arg(color).arg(bgcolor).arg(weight);
        }

        prevMatch = currMatch;
        prevIncom = currIncom;

        html += (QChar(text[i]).isSpace() ? QString("&nbsp;") : QString(text[i])); // add character itself
    }

    setHtml(html);
}

void XMETAreaEdit::mouseMoveEvent(QMouseEvent *event)
{
    QTextEdit::mouseMoveEvent(event);
    const QPoint scrollBarPos(
                horizontalScrollBar() ? horizontalScrollBar()->sliderPosition() : 0,
                verticalScrollBar() ? verticalScrollBar()->sliderPosition() : 0);
    const QPoint cursorPos = viewport()->mapFromGlobal(QCursor::pos()) + scrollBarPos;
    const int textPos = document()->documentLayout()->hitTest(cursorPos, Qt::ExactHit);
    if (reason_.contains(textPos))
        QToolTip::showText(QCursor::pos(), QString(reason_.value(textPos)));
    else
        QToolTip::showText(QCursor::pos(), "");
}

void XMETAreaEdit::mousePressEvent(QMouseEvent *event)
{
    QTextEdit::mousePressEvent(event);
}

bool XMETAreaEdit::update()
{
    // update filters from expression
    QList<QPair<int, int> > matchedRanges;
    QList<QPair<QPair<int, int>, QString> > incompleteRanges;
    const QString text = toPlainText();
    filters_ = mgp::filtersFromXmetExpr(text, &matchedRanges, &incompleteRanges);

    // update highlighting from matched and incomplete ranges

    QTextCursor cursor(textCursor());
    const int cursorPos = cursor.position();
    blockSignals(true);

    resetHighlighting();
    for (int i = 0; i < matchedRanges.size(); ++i)
        addMatchedRange(matchedRanges.at(i));
    for (int i = 0; i < incompleteRanges.size(); ++i)
        addIncompleteRange(incompleteRanges.at(i).first, incompleteRanges.at(i).second);
    showHighlighting();

    blockSignals(false);
    cursor.setPosition(cursorPos);
    setTextCursor(cursor);

    // return true iff at least one non-whitespace character is found outside any matched range
    for (int i = 0; i < matched_.size(); ++i)
        if ((!matched_.testBit(i)) && (!text.at(i).isSpace()))
            return false; // found at least one non-whitespace character in an unmatched range
    return true;
}

mgp::Filters XMETAreaEdit::filters()
{
    update();
    return filters_;
}

MGP_END_NAMESPACE
