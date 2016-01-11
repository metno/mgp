#include "usermanualwindow.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QString>
#include <QDialogButtonBox>
#include <QPushButton>

UserManualWindow::UserManualWindow()
{
    setWindowTitle("MetOrg Demo - User Manual");
    QVBoxLayout *layout = new QVBoxLayout;

    QTextBrowser *tb = new QTextBrowser;
    QString html;
    html += "<html><body>";

    html += "<h3>1 Lane View</h3>";

    html += "<h4>1.1 Mouse operations</h4>";
    html += "<ul>";
    html += "<li>&nbsp; Left-click on a task to make it current.</li>";
    html += "<li>&nbsp; Left-click outside any task to make no task current.</li>";
    html += "<li>&nbsp; Right-click on a task to make the task current and open the context menu.</li>";
    html += "<li>&nbsp; Right-click outside a task to open the context menu.</li>";
    html += "<li>&nbsp; Double-left-click on a task to edit it.</li>";
    html += "<li>&nbsp; Double-left-click outside any task to insert a new task at the current mouse position.</li>";
    html += "<li>&nbsp; Drag interior of task to move it vertically (thus changing <em>both</em> start <em>and</em> end time).</li>";
    html += "<li>&nbsp; Drag top or bottom edge of task to resize it vertically (thus changing <em>either</em> start <em>or</em> end time).</li>";
    html += "<li>&nbsp; Wheel pans vertically.</li>";
    html += "<li>&nbsp; Alt + wheel pans horizontally.</li>";
    html += "<li>&nbsp; Control + wheel scales (zooms) vertically.</li>";
    html += "<li>&nbsp; Shift + wheel scales horizontally.</li>";
    html += "</ul>";

    html += "<h4>1.2 Context menu actions</h4>";
    html += "<ul>";
    html += "<li>&nbsp; <em>Add new task</em>: Adds a new task at the current mouse position.</li>";
    html += "<li>&nbsp; <em>Paste task:</em> Pastes the last cut or copied task at the current mouse position.</li>";
    html += "</ul>";

    html += "In addition, when opening the context menu on a task (the one thus made current):";

    html += "<ul>";
    html += "<li>&nbsp; <emEdit task:</em> Opens the edit panel for the task.</li>";
    html += "<li>&nbsp; <em>Remove task:</em> Removes the task without making a pastable copy first.</li>";
    html += "<li>&nbsp; <em>Cut task:</em> Removes the task, making a pastable copy.</li>";
    html += "<li>&nbsp; <em>Copy task:</em> Makes a pastable copy of the task.</li>";
    html += "</ul>";

    html += "<h4>1.3 Keyboard shortcuts</h4>";

    html += "<ul>";
    html += "<li>&nbsp; <em>PageUp</em>/<em>PageDown</em>: Cycles the stacking order of tasks intersecting the task currently under the mouse"
            "(useful for getting to tasks completely hidden under other tasks!).</li>";
    html += "<li>&nbsp; <em>Insert</em>: Adds a new task at the current mouse position.</li>";
    html += "<li>&nbsp; <em>Delete</em>: Removes the current task.</li>";
    html += "<li>&nbsp; <em>Control</em>+<em>X</em>: Removes the current task, making a pastable copy.</li>";
    html += "<li>&nbsp; <em>Control</em>+<em>C</em>: Makes a pastable copy of the current task.</li>";
    html += "<li>&nbsp; <em>Control</em>+<em>V</em>: Pastes the last cut or copied task at the current mouse position.</li>";
    html += "</ul>";


    html += "<h3>2 Lane Header View</h3>";

    html += "<h4>2.1 Mouse operations</h4>";
    html += "<ul>";
    html += "<li>&nbsp; Left-click on a header to make the lane current.</li>";
    html += "<li>&nbsp; Left-click outside any header to make no lane current.</li>";
    html += "<li>&nbsp; Right-click on a header to make the lane current and open the context menu.</li>";
    html += "<li>&nbsp; Double-left-click on a header to edit the lane.</li>";
    html += "</ul>";

    html += "<h4>2.2 Context menu actions</h4>";
    html += "When opening the context menu on a header (the lane of which thus made current):";
    html += "<ul>";
    html += "<li>&nbsp; <emEdit lane:</em> Opens the edit panel for the lane.</li>";
    html += "<li>&nbsp; <em>Remove lane:</em> Removes the lane.</li>";
    html += "<li>&nbsp; <em>Move lane left:</em> Moves the lane one position to the left.</li>";
    html += "<li>&nbsp; <em>Move lane right:</em> Moves the lane one position to the right.</li>";
    html += "</ul>";

    html += "<h4>2.3 Keyboard shortcuts</h4>";

    html += "<ul>";
    html += "<li>&nbsp; <em>Delete</em>: Removes the current lane.</li>";
    html += "<li>&nbsp; <em>Left arrow</em>: Moves the lane one position to the left.</li>";
    html += "<li>&nbsp; <em>Right arrow</em>: Moves the lane one position to the right.</li>";
    html += "</ul>";

    html += "<h3>3 Timeline View</h3>";

    html += "<h4>3.1 Mouse operations</h4>";
    html += "<ul>";
    html += "<li>&nbsp; Drag to change the visible part of the timeline.</li>";
    html += "</ul>";

    html += "</body></html>";

    tb->setHtml(html);
    layout->addWidget(tb);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(hide()));
    layout->addWidget(buttonBox);

    setLayout(layout);
    resize(800, 800);
}

UserManualWindow &UserManualWindow::instance()
{
    static UserManualWindow uw;
    return uw;
}

void UserManualWindow::keyPressEvent(QKeyEvent *event)
{
    MainWindow::instance().handleKeyPressEvent(event); // to handle Control+Q for closing application etc.
}
