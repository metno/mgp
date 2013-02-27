#include "qcchat.h"
#include "qcglobal.h"
#include "window.h"

using namespace qclib;

extern QSettings *settings;

HoverableTextBrowser::HoverableTextBrowser()
{
    setMouseTracking(true);
}

void HoverableTextBrowser::mouseMoveEvent(QMouseEvent *event)
{
    emit hover(event);
}

// Returns a version of \a s where hyperlinks are embedded in HTML <a> tags.
static QString toAnchorTagged(const QString &s)
{
    return QString(s).replace(
        QRegExp("(http://\\S*)"), "<a href=\"\\1\" style=\"text-decoration: none;\">\\1</a>");
}

static QString toTimeString(int timestamp)
{
    return QDateTime::fromTime_t(timestamp).toString("yyyy MMM dd hh:mm:ss");
}

static QString formatEvent(const QString &text, const QString &user, int timestamp, int type)
{
    QString userTdStyle("style=\"padding-left:40px\"");
    QString textTdStyle("style=\"padding-left:2px\"");
    QString s("<tr>");
    s += QString("<td><span style=\"color:%1\">[%2]</span></td>").arg("#888").arg(toTimeString(timestamp));
    s += QString("<td align=\"right\" %1><span style=\"color:%2\">&lt;%3&gt;</span></td>")
        .arg(userTdStyle).arg("#000").arg(user);
    if (type == CHATMESSAGE) {
        s += QString("<td %1><span style=\"color:black\">%2</span></td>")
            .arg(textTdStyle).arg(toAnchorTagged(text));
    } else if (type == NOTIFICATION) {
        s += QString("<td %1><span style=\"color:%2\">%3</span></td>")
            .arg(textTdStyle).arg("#916409").arg(toAnchorTagged(text));
    } else {
        Logger::instance().logError(QString("invalid type: %1").arg(type));
        qApp->exit(1);
        return QString();
    }
    s += "</tr>";

    return s;
}

static void deleteTree(QTreeWidgetItem *root)
{
    QList<QTreeWidgetItem *> children = root->takeChildren();
    foreach (QTreeWidgetItem *child, children)
        deleteTree(child);
    delete root;
}

ChatWindow::ChatWindow()
{
    // --- BEGIN main layout and splitter ------------------------------------
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QSplitter *splitter = new QSplitter;
    QWidget *leftWidget = new QWidget;
    QWidget *rightWidget = new QWidget;
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    mainLayout->addWidget(splitter);
    // --- END main layout and splitter ------------------------------------

    // --- BEGIN left part of splitter ------------------------------------
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(11, 11, 0, 11);
    leftWidget->setLayout(leftLayout);

    QHBoxLayout *leftLayout_sub1 = new QHBoxLayout;
    leftLayout->addLayout(leftLayout_sub1);
    channelCBox_ = new QComboBox;
    channelCBox_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    leftLayout_sub1->addWidget(channelCBox_);
    leftLayout_sub1->setAlignment(channelCBox_, Qt::AlignLeft);

    leftLayout->addLayout(&logStack_);
    connect(channelCBox_, SIGNAL(activated(int)), SLOT(handleChannelSwitch()));

    QHBoxLayout *leftLayout_sub2 = new QHBoxLayout;
    leftLayout->addLayout(leftLayout_sub2);
    userLabel_ = new QLabel("(USER NOT SET)");
    userLabel_->installEventFilter(this);
    leftLayout_sub2->addWidget(userLabel_);
    edit_ = new QLineEdit;
    connect(edit_, SIGNAL(returnPressed()), SLOT(sendChatMessage()));
    edit_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    leftLayout_sub2->addWidget(edit_);
    // --- END left part of splitter ------------------------------------

    // --- BEGIN right part of splitter ------------------------------------
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setContentsMargins(0, 11, 11, 11);
    rightWidget->setLayout(rightLayout);

    QLabel *usersLabel = new QLabel("Users");
    usersLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    //usersLabel->setStyleSheet("QLabel { background-color : cyan; color : black; }");
    usersLabel->setAlignment(Qt::AlignLeft);
    rightLayout->addWidget(usersLabel);
    userTree_ = new QTreeWidget;
    userTree_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    userTree_->setColumnCount(3);
    userTree_->header()->close();
    // NOTE: for efficiency reasons, QTreeView::resizeColumnToContents() only applies to expanded items, so we
    // need to resize only when items get expanded:
    connect(userTree_, SIGNAL(itemExpanded(QTreeWidgetItem *)), SLOT(resizeUserTreeColumns()));
    rightLayout->addWidget(userTree_);
    // --- END right part of splitter ------------------------------------

    setLayout(mainLayout);

    splitter->setSizes(QList<int>() << 750 << 150);
}

void ChatWindow::appendEvent(const QString &text, const QString &user, int channelId, int timestamp, int type)
{
    if (channelId < 0) {
        Logger::instance().logWarning("appendEvent(): ignoring event with channel id < 0 for now");
        return;
    }

    html_[channelId].insert(
        html_.value(channelId).lastIndexOf("</table>"), formatEvent(text, user, timestamp, type));
    log_[channelId]->setHtml(html_.value(channelId));
}

// Sets the list of available chat channels.
void ChatWindow::setChannels(const QStringList &chatChannels)
{
    Q_ASSERT(!chatChannels.isEmpty());

    QRegExp rx("^(\\d+)\\s+(\\S+)\\s+(.*)$");
    QListIterator<QString> it(chatChannels);
    while (it.hasNext()) {
        QString item = it.next();
        if (rx.indexIn(item) == -1) {
            Logger::instance().logError(QString("setChannels(): regexp mismatch: %1").arg(item.toLatin1().data()));
            qApp->exit(1);
            return;
        }
        const int id = rx.cap(1).toInt();
        const QString name = rx.cap(2);
        //const QString descr = rx.cap(3); unused for now
        channelCBox_->addItem(name);
        channelId_.insert(name, id);
        channelName_.insert(id, name);

        html_.insert(id, "<table></table>");

        //QTextBrowser *tb = new QTextBrowser;
        HoverableTextBrowser *tb = new HoverableTextBrowser;
        tb->setHtml(html_.value(id));
        tb->setReadOnly(true);
        tb->setOpenExternalLinks(true);
        tb->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        connect(tb, SIGNAL(hover(QMouseEvent *)), SLOT(hover(QMouseEvent *)));
        log_.insert(id, tb);
        logStack_.addWidget(tb);

        channelUsers_.insert(id, new QSet<QPair<QString, QString> >);
    }

    updateWindowTitle();
    emit channelSwitch(currentChannelId());
}

// Sets the full user names.
void ChatWindow::setFullNames(const QStringList &fullNames)
{
    userFullName_.clear();
    QRegExp rx("^(\\S+)\\s+(.*)$");
    QListIterator<QString> it(fullNames);
    while (it.hasNext()) {
        QString item = it.next();
        if (rx.indexIn(item) == -1) {
            Logger::instance().logError(QString("setFullNames(): regexp mismatch: %1").arg(item.toLatin1().data()));
            qApp->exit(1);
            return;
        }
        const QString user = rx.cap(1);
        const QString fullName = rx.cap(2);
        userFullName_.insert(user, fullName);
    }
}

void ChatWindow::setUser(const QString &user)
{
    userLabel_->setText(user);
}

// Prepends history (i.e. in front of any individual events that may have arrived
// in the meantime!)
void ChatWindow::prependHistory(const QStringList &h)
{
    if (h.isEmpty())
        return;

    QRegExp rx("^(\\S+)\\s+(-?\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(.*)$");
    QMap<int, QStringList> events;
    QListIterator<QString> it(h);
    while (it.hasNext()) {
        QString item = it.next();
        if (rx.indexIn(item) == -1) {
            Logger::instance().logError(QString("prependHistory(): regexp mismatch: %1").arg(item.toLatin1().data()));
            qApp->exit(1);
            return;
        }
        const QString user = rx.cap(1);
        const int channelId = rx.cap(2).toInt();
        const int timestamp = rx.cap(3).toInt();
        const int type = rx.cap(4).toInt();
        const QString text = rx.cap(5);
        if (channelName_.contains(channelId)) {
            events[channelId] << formatEvent(text, user, timestamp, type);
        } else {
            QString msg =
                QString("prependHistory(): skipping event with invalid channelId: %1 not in")
                .arg(channelId);
            foreach (int cid, channelName_.keys())
                msg.append(QString(" %1").arg(cid));
            Logger::instance().logWarning(msg);
        }
    }

    foreach (int channelId, events.keys()) {
        if (channelId >= 0) {
            html_[channelId].insert(
                html_.value(channelId).indexOf("<table>") + QString("<table>").size(),
                events.value(channelId).join(""));
            log_[channelId]->setHtml(html_.value(channelId));
        } else {
            Logger::instance().logWarning("prependHistory(): ignoring events with channelId < 0 for now");
        }
    }
}

// Sets connected users, their IP-addresses, current window visibilities, and current channels.
void ChatWindow::setUsers(
    const QStringList &u, const QStringList &ipaddrs, const QList<bool> &winVis, const QList<int> &channelIds)
{
    Q_ASSERT(!u.isEmpty()); // the local user should be present at least
    Q_ASSERT(u.size() == ipaddrs.size());
    Q_ASSERT(u.size() == winVis.size());
    Q_ASSERT(u.size() == channelIds.size());

    foreach (int channelId, channelUsers_.keys())
        channelUsers_.value(channelId)->clear();
    winVisible_.clear();
    for (int i = 0; i < u.size(); ++i) {
        const int channelId = channelIds.at(i);
        const QPair<QString, QString> userKey = qMakePair(u.at(i), ipaddrs.at(i));
        if (channelUsers_.contains(channelId))
            channelUsers_.value(channelId)->insert(userKey);
        winVisible_.insert(userKey, winVis.at(i));
    }

    updateUserTree();
}

// Handles a user opening or closing the chat window.
void ChatWindow::handleCentralWindowVisibility(const QString &user, const QString &ipaddr, bool visible)
{
    winVisible_.insert(qMakePair(user, ipaddr), visible);
    updateUserTree();
}

// Handles a user switching to a channel.
void ChatWindow::handleCentralChannelSwitch(const QString &user, const QString &ipaddr, int channelId)
{
    if (!channelUsers_.contains(channelId)) {
        Logger::instance().logError(QString("handleCentralChannelSwitch(): channel ID not found: %1").arg(channelId));
        qApp->exit(1);
        return;
    }

    // remove user from its current channel (if any)
    for (int i = 0; i < channelUsers_.values().size(); ++i) {
        QSet<QPair<QString, QString> > *users = channelUsers_.values().at(i);
        users->remove(qMakePair(user, ipaddr));
    }

    // associate user with its current channel
    channelUsers_.value(channelId)->insert(qMakePair(user, ipaddr));

    updateUserTree();
}

// Handles a user changing full name.
void ChatWindow::handleCentralFullNameChange(const QString &user, const QString &fullName)
{
    userFullName_.insert(user, fullName);
    updateUserTree();
}

// Handles an error message from the central server.
void ChatWindow::handleCentralErrorMessage(const QString &msg)
{
    QMessageBox::warning(0, "MetChat ERROR", msg);
}

void ChatWindow::scrollToBottom()
{
    QTextBrowser *currTB = qobject_cast<QTextBrowser *>(logStack_.currentWidget());
    Q_ASSERT(currTB);
    currTB->verticalScrollBar()->setValue(currTB->verticalScrollBar()->maximum());
}

int ChatWindow::currentChannelId() const
{
    return channelId_.value(channelCBox_->currentText());
}

void ChatWindow::setServerSysInfo(const QString &hostname, const QString &domainname, const QString &ipaddr)
{
    serverSysInfo_.insert("hostname", hostname);
    serverSysInfo_.insert("domainname", domainname);
    serverSysInfo_.insert("ipaddr", ipaddr);
    updateWindowTitle();
}

void ChatWindow::openFullNameDialog()
{
    bool ok;
    const QString fullName = QInputDialog::getText(
        this, "MetChat - Change full name", "New full name:", QLineEdit::Normal,
        userFullName_.value(userLabel_->text()), &ok);
    if (ok)
        emit fullNameChange(fullName.trimmed());
}

bool ChatWindow::eventFilter(QObject *obj, QEvent *event)
{
    // double-clicking the user label allows for changing the full name of this user
    if (obj == userLabel_) {
        if (event->type() == QEvent::MouseButtonDblClick) {
            openFullNameDialog();
            return true;
        } else if (event->type() == QEvent::Enter) {
            QToolTip::showText(
                QCursor::pos(),
                QString("Full name: %1\nDouble-click to change").arg(userFullName_.value(userLabel_->text())));
        }
    }

    return false;
}

void ChatWindow::updateUserTree()
{
    QTreeWidgetItem *root = userTree_->invisibleRootItem();

    // save 'expanded' state
    QMap<QString, bool> channelExpanded;
    for (int i = 0; i < root->childCount(); ++i)
        channelExpanded.insert(
            root->child(i)->data(0, Qt::DisplayRole).toString().split(" ").first(), root->child(i)->isExpanded());

    // clear
    QList<QTreeWidgetItem *> channelItems = root->takeChildren();
    foreach (QTreeWidgetItem *channelItem, channelItems)
        deleteTree(channelItem);

    // rebuild
    foreach (int channelId, channelUsers_.keys()) {
        const QString channelName = channelName_.value(channelId);
        // create channel branch
        QTreeWidgetItem *channelItem = new QTreeWidgetItem;
        root->addChild(channelItem);
        //QStringList channelUsers = channelUsers_.value(channelId)->values();
        QList<QPair<QString, QString> > channelUsers = channelUsers_.value(channelId)->values();
        channelItem->setData(0, Qt::DisplayRole, QString("%1 (%2)").arg(channelName).arg(channelUsers.size()));
        for (int i = 0; i < channelUsers.size(); ++i) {
            const QPair<QString, QString> userInfo = channelUsers.at(i);
            // insert user in this channel branch
            const QString user = userInfo.first;
            const QString ipaddr = userInfo.second;
            const bool winOpen = winVisible_.value(qMakePair(user, ipaddr)); 
            QTreeWidgetItem *userItem = new QTreeWidgetItem;
            //
            userItem->setData(0, Qt::DisplayRole, user);
            userItem->setForeground(0, winOpen ? QColor("#000") : QColor("#888"));
            const QString toolTip = QString("%1\n%2\nchat window %3")
                .arg(userFullName_.value(user)).arg(ipaddr).arg(winOpen ? "open" : "closed");
            userItem->setToolTip(0, toolTip);
            userItem->setToolTip(1, toolTip);
            userItem->setToolTip(2, toolTip);
            //
            if (!userFullName_.value(user).isEmpty())
                userItem->setData(1, Qt::DisplayRole, QString("%1").arg(userFullName_.value(user)));
            userItem->setForeground(1, QColor("#888"));
            //
            userItem->setData(2, Qt::DisplayRole, ipaddr);
            userItem->setForeground(2, QColor("#888"));
            //
            channelItem->addChild(userItem);
        }
        channelItem->sortChildren(0, Qt::AscendingOrder);
    }

    // restore 'expanded' state
    for (int i = 0; i < root->childCount(); ++i)
        root->child(i)->setExpanded(
            channelExpanded.value(root->child(i)->data(0, Qt::DisplayRole).toString().split(" ").first()));
}

void ChatWindow::updateWindowTitle()
{
    window()->setWindowTitle(
        QString("MetChat - channel: %1; user: %2; central server: %3.%4 (%5)")
        .arg(channelName_.value(currentChannelId()))
        .arg(userLabel_->text())
        .arg(serverSysInfo_.value("hostname"))
        .arg(serverSysInfo_.value("domainname"))
        .arg(serverSysInfo_.value("ipaddr")));
}

void ChatWindow::sendChatMessage()
{
    const QString text = edit_->text().trimmed();
    edit_->clear();
    if (!text.isEmpty())
        emit chatMessage(text, currentChannelId());
}

void ChatWindow::handleChannelSwitch()
{
    const int channelId = currentChannelId();
    logStack_.setCurrentWidget(log_.value(channelId));
    updateWindowTitle();
    emit channelSwitch(channelId);
}

void ChatWindow::hover(QMouseEvent *)
{
    QTextBrowser *tb = qobject_cast<QTextBrowser *>(sender());
    Q_ASSERT(tb);

    const QPoint scrollBarPos(
        tb->horizontalScrollBar() ? tb->horizontalScrollBar()->sliderPosition() : 0,
        tb->verticalScrollBar() ? tb->verticalScrollBar()->sliderPosition() : 0);
    const QPoint cursorPos = tb->viewport()->mapFromGlobal(QCursor::pos()) + scrollBarPos;
    const int textPos =
        tb->document()->documentLayout()->hitTest(cursorPos, Qt::ExactHit);

    QString text = tb->document()->findBlock(textPos).text();
    QString candUser; // candidate user name
    if ((text.size() > 2) && (text.at(0) == '<') && (text.at(text.size() - 1) == '>'))
        candUser = text.mid(1, text.size() - 2); // strip away '<' and '>'
    else
        candUser = text;

    if (userFullName_.contains(candUser))
        QToolTip::showText(QCursor::pos(), userFullName_.value(candUser));
    else
        QToolTip::hideText();
}

void ChatWindow::resizeUserTreeColumns()
{
    userTree_->resizeColumnToContents(0);
    userTree_->resizeColumnToContents(1);
    userTree_->resizeColumnToContents(2);
}

ChatMainWindow::ChatMainWindow(ChatWindow *window)
    : geometrySaveEnabled_(true)
{
    setCentralWidget(window);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *quit = new QAction(tr("&Quit"), this);
    quit->setShortcut(QKeySequence(tr("Ctrl+Q")));
    fileMenu->addAction(quit);
    connect(quit, SIGNAL(triggered()), this, SLOT(close()));

    QMenu *optionsMenu = menuBar()->addMenu(tr("&Options"));
    QAction *setFullName = new QAction(tr("Set &full name"), this);
    setFullName->setShortcut(QKeySequence(tr("Ctrl+F")));
    optionsMenu->addAction(setFullName);
    connect(
        setFullName, SIGNAL(triggered()), qobject_cast<ChatWindow *>(centralWidget()), SLOT(openFullNameDialog()));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *about = new QAction(tr("About"), this);
    helpMenu->addAction(about);
    connect(about, SIGNAL(triggered()), this, SLOT(showAbout()));

    setWindowIcon(QIcon("/usr/share/pixmaps/metchat.png"));
    if (!restoreGeometry())
        resize(900, 350); // default if unable to restore from config file for some reason
}

// Saves main window geometry to config file.
void ChatMainWindow::saveGeometry()
{
    if ((!geometrySaveEnabled_) || (settings->status() != QSettings::NoError))
        return;
    settings->setValue("geometry", geometry());
    settings->sync();
}

// Restores main window geometry from config file. Returns true iff the operation succeeded.
bool ChatMainWindow::restoreGeometry()
{
    if ((!geometrySaveEnabled_) || (settings->status() != QSettings::NoError) ||
        (!settings->value("geometry").canConvert(QVariant::Rect)))
        return false;

    // temporarily disable geometry saves (in particular those that would otherwise result
    // from move and resize events generated for a visible window by the below setGeometry() call;
    // the value of geometry() at those points tends to be unexpected; probably due to window manager
    // peculiarities on X11)
    if (geometrySaveEnabled_) {
        geometrySaveEnabled_ = false;
        QTimer::singleShot(100, this, SLOT(enableGeometrySave()));
    }

    setGeometry(settings->value("geometry").toRect());
    return true;
}

void ChatMainWindow::closeEvent(QCloseEvent *e)
{
    // prevent the close event from terminating the application
    hide();
    e->ignore();
}

void ChatMainWindow::showEvent(QShowEvent *)
{
    restoreGeometry();
    qobject_cast<ChatWindow *>(centralWidget())->scrollToBottom();
    emit windowVisibility(true);
}

void ChatMainWindow::hideEvent(QHideEvent *)
{
    emit windowVisibility(false);
}

void ChatMainWindow::moveEvent(QMoveEvent *)
{
    saveGeometry();
}

void ChatMainWindow::resizeEvent(QResizeEvent *)
{
    saveGeometry();
}

void ChatMainWindow::enableGeometrySave()
{
    geometrySaveEnabled_ = true;
}

void ChatMainWindow::showAbout()
{
    QMessageBox mbox;
    mbox.setIconPixmap(windowIcon().pixmap(QSize(64, 64)));
    mbox.setTextFormat(Qt::RichText);
    mbox.setWindowTitle(QString("%1 MetChat").arg(tr("About")));
    mbox.setText(
        "MetChat: Chat for meteorologiske applikasjoner"
        "<br />Versjon: 0.0.0"
        "<br /><br />"
        "<a href=\"http://met.no\">Mer informasjon</a>");
    mbox.exec();
}
