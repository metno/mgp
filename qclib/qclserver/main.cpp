// qclserver

#include <QtGui> // ### TODO: include relevant headers only
#include "qcchat.h"
#include "qcglobal.h"
#include <csignal>

using namespace qclib;

static QSettings *settings = 0;

class HoverableTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    HoverableTextBrowser()
    {
        setMouseTracking(true);
    }
private:
    virtual void mouseMoveEvent(QMouseEvent *event)
    {
        emit hover(event);
    }
signals:
    void hover(QMouseEvent *);
};

class ChatWindow : public QWidget
{
    Q_OBJECT
public:
    ChatWindow()
        : geometrySaveEnabled_(true)
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

        if (!restoreGeometry())
            resize(900, 350); // default if unable to restore from config file for some reason
        splitter->setSizes(QList<int>() << 750 << 150);

        setWindowIcon(QIcon("/usr/share/pixmaps/metchat.png"));
    }

    void appendEvent(const QString &text, const QString &user, int channelId, int timestamp, int type)
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
    void setChannels(const QStringList &chatChannels)
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
    void setFullNames(const QStringList &fullNames)
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

    void setUser(const QString &user)
    {
        userLabel_->setText(user);
    }

    // Prepends history (i.e. in front of any individual events that may have arrived
    // in the meantime!)
    void prependHistory(const QStringList &h)
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

    // Sets connected users, their IP-addresses, and their current channels.
    void setUsers(const QStringList &u, const QStringList &ipaddrs, const QList<int> &channelIds)
    {
        Q_ASSERT(!u.isEmpty()); // the local user should be present at least
        Q_ASSERT(u.size() == ipaddrs.size());
        Q_ASSERT(u.size() == channelIds.size());

        foreach (int channelId, channelUsers_.keys())
            channelUsers_.value(channelId)->clear();
        for (int i = 0; i < u.size(); ++i) {
            const int channelId = channelIds.at(i);
            if (channelUsers_.contains(channelId))
                channelUsers_.value(channelId)->insert(qMakePair(u.at(i), ipaddrs.at(i)));
        }

        updateUserTree();
    }

    // Handles a user switching to a channel.
    void handleCentralChannelSwitch(const QString &user, const QString &ipaddr, int channelId)
    {
        if (!channelUsers_.contains(channelId)) {
            Logger::instance().logError(QString("handleCentralChannelSwitch(): channel ID not found: %1").arg(channelId));
            qApp->exit(1);
            return;
        }

        // remove user from its current channel (if any)
        // foreach ((QSet<QPair<QString, QString> > *users), channelUsers_.values())
        //     users->remove(qMakePair(user, ipaddr));
        for (int i = 0; i < channelUsers_.values().size(); ++i) {
            QSet<QPair<QString, QString> > *users = channelUsers_.values().at(i);
            users->remove(qMakePair(user, ipaddr));
        }


        // associate user with its current channel
        channelUsers_.value(channelId)->insert(qMakePair(user, ipaddr));

        updateUserTree();
    }

    // Handles a user changing full name.
    void handleCentralFullNameChange(const QString &user, const QString &fullName)
    {
        userFullName_.insert(user, fullName);
        updateUserTree();
    }

    // Handles an error message from the central server.
    void handleCentralErrorMessage(const QString &msg)
    {
        QMessageBox::warning(0, "MetChat ERROR", msg);
    }

    void scrollToBottom()
    {
        QTextBrowser *currTB = qobject_cast<QTextBrowser *>(logStack_.currentWidget());
        Q_ASSERT(currTB);
        currTB->verticalScrollBar()->setValue(currTB->verticalScrollBar()->maximum());
    }

    int currentChannelId() const
    {
        return channelId_.value(channelCBox_->currentText());
    }

    void setServerSysInfo(const QString &hostname, const QString &domainname, const QString &ipaddr)
    {
        serverSysInfo_.insert("hostname", hostname);
        serverSysInfo_.insert("domainname", domainname);
        serverSysInfo_.insert("ipaddr", ipaddr);
        updateWindowTitle();
    }

private:
    QComboBox *channelCBox_;
    QMap<QString, int> channelId_;
    QMap<int, QString> channelName_;
    QTreeWidget *userTree_;
    QMap<int, QTextBrowser *> log_;
    QStackedLayout logStack_;
    QLabel *userLabel_;
    QLineEdit *edit_;
    QMap<int, QString> html_;
    // (user, IP-address)-combinations associated with each chat channel:
    QMap<int, QSet<QPair<QString, QString> > *> channelUsers_;
    bool geometrySaveEnabled_;
    QMap<QString, QString> serverSysInfo_;
    QMap<QString, QString> userFullName_;

    // Saves window geometry to config file.
    void saveGeometry()
    {
        if ((!geometrySaveEnabled_) || (settings->status() != QSettings::NoError))
            return;
        settings->setValue("geometry", geometry());
        settings->sync();
    }

    // Restores window geometry from config file. Returns true iff the operation succeeded.
    bool restoreGeometry()
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

    void closeEvent(QCloseEvent *e)
    {
        // prevent the close event from terminating the application
        hide();
        e->ignore();
    }

    void showEvent(QShowEvent *)
    {
        restoreGeometry();
        scrollToBottom();
        emit windowShown();
    }

    void hideEvent(QHideEvent *)
    {
        emit windowHidden();
    }

    void moveEvent(QMoveEvent *)
    {
        saveGeometry();
    }

    void resizeEvent(QResizeEvent *)
    {
        saveGeometry();
    }

    virtual void mouseMoveEvent(QMouseEvent *)
    {
    }

    bool eventFilter(QObject *obj, QEvent *event)
    {
        // double-clicking the user label allows for changing the full name of this user
        if (obj == userLabel_) {
            if (event->type() == QEvent::MouseButtonDblClick) {
                bool ok;
                const QString fullName = QInputDialog::getText(
                    this, "MetChat - Change full name", "New full name:", QLineEdit::Normal,
                    userFullName_.value(userLabel_->text()), &ok);
                if (ok)
                    emit fullNameChange(fullName.trimmed());
                return true;
            } else if (event->type() == QEvent::Enter) {
                QToolTip::showText(
                    QCursor::pos(),
                    QString("Full name: %1\nDouble-click to change").arg(userFullName_.value(userLabel_->text())));
            }
        }

        return false;
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

    void deleteTree(QTreeWidgetItem *root)
    {
        QList<QTreeWidgetItem *> children = root->takeChildren();
        foreach (QTreeWidgetItem *child, children)
            deleteTree(child);
        delete root;
    }

    void updateUserTree()
    {
        // foreach (int channelId, channelUsers_.keys())
        //     qDebug() << "users in channel" << channelId << ":" << *(channelUsers_.value(channelId));
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
//            foreach (QPair<QString, QString> userInfo, channelUsers) {
            for (int i = 0; i < channelUsers.size(); ++i) {
                const QPair<QString, QString> userInfo = channelUsers.at(i);
                // insert user in this channel branch
                const QString user = userInfo.first;
                const QString ipaddr = userInfo.second;
                QTreeWidgetItem *userItem = new QTreeWidgetItem;
                //
                userItem->setData(0, Qt::DisplayRole, user);
                userItem->setForeground(0, QColor("#000"));
                userItem->setToolTip(0, QString("%1\n%2").arg(userFullName_.value(user)).arg(ipaddr));
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
        }

        // restore 'expanded' state
        for (int i = 0; i < root->childCount(); ++i)
            root->child(i)->setExpanded(
                channelExpanded.value(root->child(i)->data(0, Qt::DisplayRole).toString().split(" ").first()));
    }

    void updateWindowTitle()
    {
        setWindowTitle(
            QString("MetChat - channel: %1; user: %2; central server: %3.%4 (%5)")
            .arg(channelName_.value(currentChannelId()))
            .arg(userLabel_->text())
            .arg(serverSysInfo_.value("hostname"))
            .arg(serverSysInfo_.value("domainname"))
            .arg(serverSysInfo_.value("ipaddr")));
    }

private slots:
    void sendChatMessage()
    {
        const QString text = edit_->text().trimmed();
        edit_->clear();
        if (!text.isEmpty())
            emit chatMessage(text, currentChannelId());
    }

    void handleChannelSwitch()
    {
        const int channelId = currentChannelId();
        logStack_.setCurrentWidget(log_.value(channelId));
        updateWindowTitle();
        emit channelSwitch(channelId);
    }

    void enableGeometrySave()
    {
        geometrySaveEnabled_ = true;
    }

    void hover(QMouseEvent *)
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

    void resizeUserTreeColumns()
    {
        userTree_->resizeColumnToContents(0);
        userTree_->resizeColumnToContents(1);
        userTree_->resizeColumnToContents(2);
    }

signals:
    void windowShown();
    void windowHidden();
    void chatMessage(const QString &, int);
    void channelSwitch(int);
    void fullNameChange(const QString &);
};

class Interactor : public QObject
{
    Q_OBJECT
public:
    Interactor(const QString &user, const QString &chost, quint16 cport)
        : user_(user)
        , chost_(chost)
        , cport_(cport)
        , showingWindow_(false)
    {
        cchannels_.reset(new QCLocalClientChannels);
        schannel_.reset(new QCTcpServerChannel);
        window_.reset(new ChatWindow);
    }

    bool initialize()
    {
        // initialize interaction with chat window
        connect(window_.data(), SIGNAL(windowShown()), SLOT(windowShown()));
        connect(window_.data(), SIGNAL(windowHidden()), SLOT(windowHidden()));
        connect(window_.data(), SIGNAL(chatMessage(const QString &, int)), SLOT(localChatMessage(const QString &, int)));
        connect(window_.data(), SIGNAL(channelSwitch(int)), SLOT(handleChannelSwitch(int)));
        connect(window_.data(), SIGNAL(fullNameChange(const QString &)), SLOT(handleFullNameChange(const QString &)));
        window_->setUser(user_);

        // initialize interaction with qcapps
        connect(cchannels_.data(), SIGNAL(serverFileChanged(const QString &)), SLOT(serverFileChanged(const QString &)));
        connect(cchannels_.data(), SIGNAL(clientConnected(qint64)), SLOT(clientConnected(qint64)));
        connect(cchannels_.data(), SIGNAL(showChatWindow()), SLOT(showChatWindow()));
        connect(cchannels_.data(), SIGNAL(hideChatWindow()), SLOT(hideChatWindow()));
        connect(
            cchannels_.data(), SIGNAL(notification(const QString &, const QString &, int, int)),
            SLOT(localNotification(const QString &, const QString &, int)));
        if (!cchannels_->listen()) {
            Logger::instance().logError(
                QString("failed to create listen path: %1").arg(cchannels_->lastError().toLatin1().data()));
            return false;
        }

        // initialize interaction with qccserver
        connect(schannel_.data(), SIGNAL(init(const QVariantMap &, qint64)), SLOT(init(const QVariantMap &)));
        connect(schannel_.data(), SIGNAL(serverDisconnected()), SLOT(serverDisconnected()));
        connect(
            schannel_.data(), SIGNAL(chatMessage(const QString &, const QString &, int, int)),
            SLOT(centralChatMessage(const QString &, const QString &, int, int)));
        connect(
            schannel_.data(), SIGNAL(notification(const QString &, const QString &, int, int)),
            SLOT(centralNotification(const QString &, const QString &, int, int)));
        connect(
            schannel_.data(), SIGNAL(channelSwitch(int, const QString &, const QString &, qint64)),
            SLOT(centralChannelSwitch(int, const QString &, const QString &)));
        connect(
            schannel_.data(), SIGNAL(fullNameChange(const QString &, const QString &, qint64)),
            SLOT(centralFullNameChange(const QString &, const QString &)));
        connect(
            schannel_.data(), SIGNAL(errorMessage(const QString &, qint64)),
            SLOT(centralErrorMessage(const QString &)));
        connect(
            schannel_.data(), SIGNAL(users(const QStringList &, const QStringList &, const QList<int> &)),
            SLOT(users(const QStringList &, const QStringList &, const QList<int> &)));
        if (!schannel_->connectToServer(chost_, cport_)) {
            Logger::instance().logError(
                QString("failed to connect to qccserver: connectToServer() failed: %1")
                .arg(schannel_->lastError().toLatin1().data()));
            return false;
        }
        QVariantMap msg;
        msg.insert("user", user_);
        bool ok;
        const QString ipaddr = getLocalIPAddress(&ok);
        if (!ok) {
            Logger::instance().logError(QString("failed to get local IP address: %1").arg(ipaddr.toLatin1().data()));
            return false;
        }
        msg.insert("ipaddr", ipaddr);
        schannel_->sendInit(msg);

        return true;
    }

private:
    QString user_;
    QString chost_;
    quint16 cport_;
    QScopedPointer<QCLocalClientChannels> cchannels_; // qcapp channels
    QScopedPointer<QCTcpServerChannel> schannel_; // qccserver channel
    QScopedPointer<ChatWindow> window_;
    QStringList chatChannels_; //  a.k.a. chat rooms
    bool showingWindow_;

private slots:
    void init(const QVariantMap &msg)
    {
        window_->setServerSysInfo(
            msg.value("hostname").toString(), msg.value("domainname").toString(), msg.value("ipaddr").toString());
        chatChannels_ = msg.value("channels").toStringList();
        window_->setChannels(chatChannels_);
        window_->setFullNames(msg.value("fullnames").toStringList());
        window_->prependHistory(msg.value("history").toStringList());
    }

    void serverDisconnected()
    {
        Logger::instance().logWarning("central server disconnected");
        qApp->exit(1);
    }

    void serverFileChanged(const QString &serverPath)
    {
        Logger::instance().logError(
            QString("local server file modified or (re)moved: %1").arg(serverPath.toLatin1().data()));
        qApp->exit(1);
    }

    void clientConnected(qint64 qcapp)
    {
        if (!schannel_->isConnected()) {
            Logger::instance().logWarning("central server not connected; disconnecting client");
            cchannels_->close(qcapp);
            return;
        }

        // send init message to this qcapp only
        QVariantMap msg;
        msg.insert("chost", chost_);
        msg.insert("cport", cport_);
        msg.insert("channels", chatChannels_);
        msg.insert("windowshown", window_->isVisible());
        cchannels_->sendInit(msg, qcapp);
    }

    void localNotification(const QString &text, const QString &, int channelId)
    {
        schannel_->sendNotification(text, user_, channelId);
    }

    void centralChatMessage(const QString &text, const QString &user, int channelId, int timestamp)
    {
        window_->appendEvent(text, user, channelId, timestamp, CHATMESSAGE);
        window_->scrollToBottom();
    }

    void centralNotification(const QString &text, const QString &user, int channelId, int timestamp)
    {
        cchannels_->sendNotification(text, user, channelId, timestamp);
        window_->appendEvent(text, user, channelId, timestamp, NOTIFICATION);
        window_->scrollToBottom();
    }

    void centralChannelSwitch(int channelId, const QString &user, const QString &ipaddr)
    {
        window_->handleCentralChannelSwitch(user, ipaddr, channelId);
    }

    void centralFullNameChange(const QString &fullName, const QString &user)
    {
        window_->handleCentralFullNameChange(user, fullName);
    }

    void centralErrorMessage(const QString &msg)
    {
        window_->handleCentralErrorMessage(msg);
    }

    void users(const QStringList &u, const QStringList &ipaddrs, const QList<int> &channelIds)
    {
        window_->setUsers(u, ipaddrs, channelIds);
    }

    void sendShowChatWindow()
    {
        cchannels_->sendShowChatWindow();
    }

    void sendHideChatWindow()
    {
        cchannels_->sendHideChatWindow();
    }

    // invoked from window_->showEvent()
    void windowShown()
    {
        // inform clients right after other events have been processed
        QTimer::singleShot(0, this, SLOT(sendShowChatWindow()));
        showingWindow_ = false;
    }

    // invoked from client request
    void showChatWindow()
    {
        if (showingWindow_)
            return; // let the active operation complete
        showingWindow_ = true;

        if (window_->isVisible()) {
            window_->hide();
            qApp->processEvents();
            QTimer::singleShot(500, window_.data(), SLOT(show()));
        } else {
            window_->show();
        }
    }

    // invoked from window_->hideEvent()
    void windowHidden()
    {
        // inform clients right after other events have been processed
        QTimer::singleShot(0, this, SLOT(sendHideChatWindow()));
    }

    // invoked from client request
    void hideChatWindow()
    {
        window_->hide();
    }

    void localChatMessage(const QString &text, int channelId)
    {
        schannel_->sendChatMessage(text, user_, channelId);
    }

    void handleChannelSwitch(int channelId)
    {
        schannel_->sendChannelSwitch(channelId);
    }

    void handleFullNameChange(const QString &fullName)
    {
        schannel_->sendFullNameChange(fullName);
    }
};

static void printUsage(bool toLogger = true)
{
    const QString s = QString(
        "usage: %1 --help | (--chost <central server host> --cport <central server port>)")
        .arg(qApp->arguments().first().toLatin1().data());
    if (toLogger)
        Logger::instance().logError(s);
    else
        qDebug() << s.toLatin1().data();
}

struct CleanExit {
    CleanExit() {
        signal(SIGQUIT, &CleanExit::exitQt);
        signal(SIGINT, &CleanExit::exitQt);
        signal(SIGTERM, &CleanExit::exitQt);
        // signal(SIGBREAK, &CleanExit::exitQt);
        // ### TODO: Add more signals to cover all relevant reasons for termination
        // ...
    }

    static void exitQt(int sig) {
        Q_UNUSED(sig);
        QCoreApplication::exit(0);
    }
};

class ExitHandler : public QObject
{
    Q_OBJECT
public slots:
    void cleanup()
    {
        // do cleanup actions here
    }
};

int main(int argc, char *argv[])
{
    Logger::instance().initialize("/tmp/qclserver.log");
    CleanExit cleanExit;
    QApplication app(argc, argv);
    ExitHandler exitHandler;
    QObject::connect(&app, SIGNAL(aboutToQuit()), &exitHandler, SLOT(cleanup()));

    // extract command-line options
    const QMap<QString, QString> options = getOptions(app.arguments());
    if (options.contains("help")) {
        printUsage(false);
        return 0;
    }
    const QString chost = options.value("chost");
    if (chost.isEmpty()) {
        Logger::instance().logError("failed to extract central server host");
        printUsage();
        return 1;
    }
    bool ok;
    const quint16 cport = options.value("cport").toUInt(&ok);
    if (!ok) {
        Logger::instance().logError("failed to extract central server port");
        printUsage();
        return 1;
    }

    // create global settings object for this $USER (assuming 1-1 correspondence between $USER and $HOME)
    settings = new QSettings(
        QString("%1/.config/metchat/qclserver.conf").arg(qgetenv("HOME").constData()), QSettings::NativeFormat);

    // create object to handle interaction between qcapps, qccserver, and chat window
    Interactor interactor(qgetenv("USER").constData(), chost, cport);
    if (!interactor.initialize()) {
        Logger::instance().logError("failed to initialize interactor");
        return 1;
    }

    return app.exec();
}

#include "main.moc"
