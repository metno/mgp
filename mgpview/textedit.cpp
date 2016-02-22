#include "textedit.h"
#include <QMouseEvent>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>
#include <QToolTip>
#include <QDebug>

TextEdit::TextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    setMouseTracking(true);
}

void TextEdit::resetHighlighting()
{
    setPlainText(toPlainText().trimmed());
    matched_ = QBitArray(toPlainText().size(), false);
    incomplete_ = QBitArray(toPlainText().size(), false);
    reason_.clear();
}

void TextEdit::addMatchedRange(const QPair<int, int> &range)
{
    matched_.fill(true, range.first, range.second + 1);
}

void TextEdit::addIncompleteRange(const QPair<int, int> &range, const QString &reason)
{
    incomplete_.fill(true, range.first, range.second + 1);
    for (int i = range.first; i <= range.second; ++i)
        reason_.insert(i, reason);
}

void TextEdit::showHighlighting()
{
    const QString defaultColor("#888");
    const QString defaultBGColor("#fff");
    const QString defaultWeight("regular");
    const QString matchColor("#088");
    const QString matchBGColor("#fff");
    const QString matchWeight("bold");
    const QString incomColor("#800");
    const QString incomBGColor("#ff0");
    const QString incomWeight("bold");

    QString html;
    const QString text(toPlainText().trimmed());
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
        html += text[i]; // add character itself
    }

    setHtml(html);
}

void TextEdit::mouseMoveEvent(QMouseEvent *event)
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

void TextEdit::mousePressEvent(QMouseEvent *event)
{
    QTextEdit::mousePressEvent(event);
}
