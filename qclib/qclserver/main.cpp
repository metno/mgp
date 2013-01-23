// Example qclserver code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qcchat.h"

class ChatWindow : public QWidget
{
    Q_OBJECT
public:
    ChatWindow()
        : edit_(0)
    {
        QHBoxLayout *layout1 = new QHBoxLayout;

        QVBoxLayout *layout2 = new QVBoxLayout;
        layout1->addLayout(layout2);

        QHBoxLayout *layout2_1 = new QHBoxLayout;
        layout2->addLayout(layout2_1);
        channelCBox_ = new QComboBox;
        channelCBox_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        layout2_1->addWidget(channelCBox_);
        QLabel *label1 = new QLabel;
        // QLabel *label1 = new QLabel("CHAT WINDOW MOCKUP");
        // label1->setStyleSheet("QLabel { background-color : yellow; color : black; }");
        label1->setAlignment(Qt::AlignCenter);
        layout2_1->addWidget(label1);

        layout2->addLayout(&logStack_);
        connect(channelCBox_, SIGNAL(activated(int)), SLOT(handleChannelSwitch()));

        edit_ = new QLineEdit;
        connect(edit_, SIGNAL(returnPressed()), SLOT(sendChatMessage()));
        edit_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
        layout2->addWidget(edit_);

        usersLayout_ = new QVBoxLayout;
        layout1->addLayout(usersLayout_);
        QLabel *label2 = new QLabel("Users");
        label2->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        //label2->setStyleSheet("QLabel { background-color : cyan; color : black; }");
        label2->setAlignment(Qt::AlignLeft);
        usersLayout_->addWidget(label2);
        userTree_ = new QTreeWidget;
        userTree_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        userTree_->setFixedWidth(120);
        userTree_->header()->close();
        usersLayout_->addWidget(userTree_);

        setLayout(layout1);
        resize(1000, 400);
    }

    void appendEvent(const QString &text, const QString &user, int channelId, int timestamp, int type)
    {
        if (channelId < 0) {
            qWarning("WARNING: appendEvent(): ignoring event with channel id < 0 for now");
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
            if (rx.indexIn(item) == -1)
                qFatal("setChannels(): regexp mismatch: %s", item.toLatin1().data());
            const int id = rx.cap(1).toInt();
            const QString name = rx.cap(2);
            //const QString descr = rx.cap(3); unused for now
            channelCBox_->addItem(name);
            channelId_.insert(name, id);
            channelName_.insert(id, name);

            html_.insert(id, "<table></table>");

            QTextBrowser *tb = new QTextBrowser;
            tb->setHtml(html_.value(id));
            tb->setReadOnly(true);
            tb->setOpenExternalLinks(true);
            tb->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
            log_.insert(id, tb);
            logStack_.addWidget(tb);

            channelUsers_.insert(id, new QSet<QString>);
        }

        emit channelSwitch(currentChannelId());
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
            if (rx.indexIn(item) == -1)
                qFatal("prependHistory(): regexp mismatch: %s", item.toLatin1().data());
            const QString user = rx.cap(1);
            const int channelId = rx.cap(2).toInt();
            const int timestamp = rx.cap(3).toInt();
            const int type = rx.cap(4).toInt();
            const QString text = rx.cap(5);
            events[channelId] << formatEvent(text, user, timestamp, type);
        }

        foreach (int channelId, events.keys()) {
            if (channelId >= 0) {
                html_[channelId].insert(
                    html_.value(channelId).indexOf("<table>") + QString("<table>").size(),
                    events.value(channelId).join(""));
                log_[channelId]->setHtml(html_.value(channelId));
            } else {
                qWarning("WARNING: prependHistory(): ignoring events with channelId < 0 for now");
            }
        }
    }

    // Sets connected users and their current channels.
    void setUsers(const QStringList &u, const QList<int> &channelIds)
    {
        Q_ASSERT(!u.isEmpty()); // the local user should be present at least
        Q_ASSERT(u.size() == channelIds.size());

        foreach (int channelId, channelUsers_.keys())
            channelUsers_.value(channelId)->clear();
        for (int i = 0; i < u.size(); ++i) {
            const int channelId = channelIds.at(i);
            if (channelUsers_.contains(channelId))
                channelUsers_.value(channelId)->insert(u.at(i));
            // else
            //     qDebug() << "ignoring user" << u.at(i) << "with invalid channel id:" << channelId;
        }

        updateUserTree();
    }

    // Handles a user switching to a channel.
    void handleCentralChannelSwitch(const QString &user, int channelId)
    {
        Q_ASSERT(channelUsers_.contains(channelId));

        // remove user from its current channel (if any)
        foreach (QSet<QString> *users, channelUsers_.values())
            users->remove(user);

        // associate user with its current channel
        channelUsers_.value(channelId)->insert(user);

        updateUserTree();
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

private:
    QVBoxLayout *usersLayout_;
    QComboBox *channelCBox_;
    QMap<QString, int> channelId_;
    QMap<int, QString> channelName_;
    QTreeWidget *userTree_;
    QMap<int, QTextBrowser *>log_;
    QStackedLayout logStack_;
    QLineEdit *edit_;
    QMap<int, QString> html_;
    QMap<int, QSet<QString> *> channelUsers_;

    void closeEvent(QCloseEvent *event)
    {
        // prevent the close event from terminating the application
        hide();
        event->ignore();
    }

    void showEvent(QShowEvent *)
    {
        scrollToBottom();
        emit windowShown();
    }

    void hideEvent(QHideEvent *)
    {
        emit windowHidden();
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
        QString userTdStyle("style=\"padding-left:20px\"");
        QString textTdStyle("style=\"padding-left:2px\"");
        QString s("<tr>");
        s += QString("<td><span style=\"color:%1\">[%2]</span></td>").arg("#888").arg(toTimeString(timestamp));
        s += QString("<td align=\"right\" %1><span style=\"color:%2\">&lt;%3&gt;</span></td>")
            .arg(userTdStyle).arg("#000").arg(user);
        if (type == CHATMESSAGE)
            s += QString("<td %1><span style=\"color:black\">%2</span></td>")
                .arg(textTdStyle).arg(toAnchorTagged(text));
        else if (type == NOTIFICATION)
            s += QString("<td %1><span style=\"color:%2\">%3</span></td>")
                .arg(textTdStyle).arg("#916409").arg(toAnchorTagged(text));
        else
            qFatal("invalid type: %d", type);
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
            QStringList channelUsers = channelUsers_.value(channelId)->values();
            channelItem->setData(0, Qt::DisplayRole, QString("%1 (%2)").arg(channelName).arg(channelUsers.size()));
            foreach (QString user, channelUsers) {
                // insert user in this channel branch
                QTreeWidgetItem *userItem = new QTreeWidgetItem;
                userItem->setData(0, Qt::DisplayRole, user);
                userItem->setForeground(0, QColor("#888"));
                channelItem->addChild(userItem);
            }
        }

        // save 'expanded' state
        for (int i = 0; i < root->childCount(); ++i)
            root->child(i)->setExpanded(
                channelExpanded.value(root->child(i)->data(0, Qt::DisplayRole).toString().split(" ").first()));
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
        emit channelSwitch(channelId);
    }

signals:
    void windowShown();
    void windowHidden();
    void chatMessage(const QString &, int);
    void channelSwitch(int);
};

class Interactor : public QObject
{
    Q_OBJECT
public:
    Interactor(QCClientChannels *cchannels, QCServerChannel *schannel, ChatWindow *window)
        : cchannels_(cchannels)
        , schannel_(schannel)
        , window_(window)
    {
        user_ = qgetenv("USER");
        if (user_.isEmpty()) {
            qWarning("failed to extract string from environment variable USER; using '<unknown>'");
            user_ = QString("<unknown>");
        }

        connect(cchannels_, SIGNAL(clientConnected(qint64)), SLOT(clientConnected(qint64)));
        connect(cchannels_, SIGNAL(chatWindowShown()), window_, SLOT(show()));
        connect(cchannels_, SIGNAL(chatWindowHidden()), window_, SLOT(hide()));
        connect(
            cchannels_, SIGNAL(notification(const QString &, const QString &, int, int)),
            SLOT(localNotification(const QString &, const QString &, int)));

        connect(schannel_, SIGNAL(serverDisconnected()), SLOT(serverDisconnected()));
        connect(
            schannel_, SIGNAL(chatMessage(const QString &, const QString &, int, int)),
            SLOT(centralChatMessage(const QString &, const QString &, int, int)));
        connect(
            schannel_, SIGNAL(notification(const QString &, const QString &, int, int)),
            SLOT(centralNotification(const QString &, const QString &, int, int)));
        connect(
            schannel_, SIGNAL(channelSwitch(qint64, int, const QString &)),
            SLOT(centralChannelSwitch(qint64, int, const QString &)));
        connect(schannel_, SIGNAL(channels(const QStringList &)), SLOT(channels(const QStringList &)));
        connect(schannel_, SIGNAL(history(const QStringList &)), SLOT(history(const QStringList &)));
        connect(
            schannel_, SIGNAL(users(const QStringList &, const QList<int> &)),
            SLOT(users(const QStringList &, const QList<int> &)));

        connect(window_, SIGNAL(windowShown()), SLOT(showChatWindow()));
        connect(window_, SIGNAL(windowHidden()), SLOT(hideChatWindow()));
        connect(window_, SIGNAL(chatMessage(const QString &, int)), SLOT(localChatMessage(const QString &, int)));
        connect(window_, SIGNAL(channelSwitch(int)), SLOT(handleChannelSwitch(int)));

        QVariantMap msg;
        msg.insert("user", user_);
        schannel_->initialize(msg);
    }

private:
    QCClientChannels *cchannels_; // qcapp channels
    QCServerChannel *schannel_; // qccserver channel
    ChatWindow *window_;
    QString user_;
    QStringList chatChannels_; //  a.k.a. chat rooms

private slots:
    void serverDisconnected()
    {
        qWarning("WARNING: central server disconnected");
        qApp->exit(1);
    }

    void clientConnected(qint64 qcapp)
    {
        if (!schannel_->isConnected()) {
            qWarning("WARNING: central server not connected; disconnecting");
            cchannels_->close(qcapp);
            return;
        }

        // inform about current chat window visibility
        if (window_->isVisible())
            cchannels_->showChatWindow(qcapp);
        else
            cchannels_->hideChatWindow(qcapp);

        // send available chat channels to this qcapp only
        cchannels_->sendChannels(chatChannels_, qcapp);
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

    void centralChannelSwitch(qint64, int channelId, const QString &user)
    {
        window_->handleCentralChannelSwitch(user, channelId);
    }

    void channels(const QStringList &chatChannels)
    {
        window_->setChannels(chatChannels);
        chatChannels_ = chatChannels;
    }

    void history(const QStringList &h)
    {
        window_->prependHistory(h);
    }

    void users(const QStringList &u, const QList<int> &channelIds)
    {
        window_->setUsers(u, channelIds);
    }

    void showChatWindow()
    {
        cchannels_->showChatWindow();
    }

    void hideChatWindow()
    {
        cchannels_->hideChatWindow();
    }

    void localChatMessage(const QString &text, int channelId)
    {
        schannel_->sendChatMessage(text, user_, channelId);
    }

    void handleChannelSwitch(int channelId)
    {
        schannel_->sendChannelSwitch(channelId);
    }
};

static void printUsage()
{
    qDebug() << QString(
        "usage: %1 --lport <local server port> --chost <central server host> --cport <central server port>")
        .arg(qApp->arguments().first()).toLatin1().data();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // extract command-line options
    const QMap<QString, QString> options = getOptions(app.arguments());
    bool ok;
    const quint16 lport = options.value("lport").toUInt(&ok);
    if (!ok) {
        qDebug("failed to extract local server port");
        printUsage();
        return 1;
    }
    const QString chost = options.value("chost");
    if (chost.isEmpty()) {
        qDebug("failed to extract central server host");
          return 1;
    }
    const quint16 cport = options.value("cport").toUInt(&ok);
    if (!ok) {
        qDebug("failed to extract central server port");
        return 1;
    }

    // listen for incoming qcapp connections
    QCClientChannels cchannels;
    if (!cchannels.listen(lport)) {
        qDebug(
            "failed to listen for incoming qcapp connections: cchannels.listen() failed: %s",
            cchannels.lastError().toLatin1().data());
        return 1;
    }

    // establish channel to qccserver
    QCServerChannel schannel;
    if (!schannel.connectToServer(chost, cport)) {
        qDebug(
            "failed to connect to qccserver: schannel.connectToServer() failed: %s",
            schannel.lastError().toLatin1().data());
        return 1;
    }

    // create a chat window
    ChatWindow *window = new ChatWindow;
    //window->show();

    // create object to handle interaction between qcapps, qccserver, and chat window
    Interactor interactor(&cchannels, &schannel, window);

    return app.exec();
}

#include "main.moc"
