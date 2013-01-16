// Example qclserver code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qc.h"

class ChatWindow : public QWidget
{
    Q_OBJECT
public:
    ChatWindow()
        : log_(0)
        , edit_(0)
        , html_("<table></table>")
    {
        QVBoxLayout *layout = new QVBoxLayout;

        QLabel *label = new QLabel("CHAT WINDOW MOCKUP");
        label->setStyleSheet("QLabel { background-color : yellow; color : black; }");
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        log_ = new QTextBrowser;
        log_->setHtml(html_);
        log_->setReadOnly(true);
        log_->setOpenExternalLinks(true);
        layout->addWidget(log_);

        edit_ = new QLineEdit;
        connect(edit_, SIGNAL(returnPressed()), SLOT(sendChatMessage()));

        layout->addWidget(edit_);

        setLayout(layout);
        resize(1000, 400);
    }

    void appendEvent(const QString &text, const QString &user, int timestamp, int type)
    {
        html_.insert(html_.lastIndexOf("</table>"), formatEvent(text, user, timestamp, type));
        log_->setHtml(html_);
    }

    // Prepends history (i.e. in front of any individual events that may have arrived
    // in the meantime!)
    void prependHistory(const QStringList &h)
    {
        if (h.size() == 0)
            return;

        QRegExp rx("^(\\S+)\\s+(\\d+)\\s+(\\d+)\\s+(.*)$");
        QStringList events;
        QListIterator<QString> it(h);
        while (it.hasNext()) {
            QString item = it.next();
            if (rx.indexIn(item) == -1)
                qFatal("regexp mismatch: %s", item.toLatin1().data());
            const QString user = rx.cap(1);
            const int timestamp = rx.cap(2).toInt();
            const int type = rx.cap(3).toInt();
            const QString text = rx.cap(4);
            events << formatEvent(text, user, timestamp, type);
        }

        html_.insert(html_.indexOf("<table>") + QString("<table>").size(), events.join(""));
        log_->setHtml(html_);
    }

    void scrollToBottom()
    {
        log_->verticalScrollBar()->setValue(log_->verticalScrollBar()->maximum());
    }

private:
    QTextBrowser *log_;
    QLineEdit *edit_;
    QString html_;

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

private slots:
    void sendChatMessage()
    {
        const QString text = edit_->text().trimmed();
        edit_->clear();
        if (!text.isEmpty())
            emit chatMessage(text);
    }

signals:
    void windowShown();
    void windowHidden();
    void chatMessage(const QString &);
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
            cchannels_, SIGNAL(notification(const QString &, const QString &, int)),
            SLOT(localNotification(const QString &)));

        connect(
            schannel_, SIGNAL(chatMessage(const QString &, const QString &, int)),
            SLOT(centralChatMessage(const QString &, const QString &, int)));
        connect(
            schannel_, SIGNAL(notification(const QString &, const QString &, int)),
            SLOT(centralNotification(const QString &, const QString &, int)));
        connect(schannel_, SIGNAL(history(const QStringList &)), SLOT(history(const QStringList &)));

        connect(window_, SIGNAL(windowShown()), SLOT(showChatWindow()));
        connect(window_, SIGNAL(windowHidden()), SLOT(hideChatWindow()));
        connect(window_, SIGNAL(chatMessage(const QString &)), SLOT(localChatMessage(const QString &)));

        schannel_->sendHistoryRequest();
    }

private:
    QCClientChannels *cchannels_; // qcapp channels
    QCServerChannel *schannel_; // qccserver channel
    ChatWindow *window_;
    QString user_;

private slots:
    void clientConnected(qint64 id)
    {
        // inform about current chat window visibility
        if (window_->isVisible())
            cchannels_->showChatWindow(id);
        else
            cchannels_->hideChatWindow(id);
    }

    void localNotification(const QString &text)
    {
        //qDebug() << "notification (from a local qcapp):" << text;
        schannel_->sendNotification(text, user_);
    }

    void centralChatMessage(const QString &text, const QString &user, int timestamp)
    {
        //qDebug() << "chat message (from qccserver) (timestamp:" << timestamp << "):" << text;
        window_->appendEvent(text, user, timestamp, CHATMESSAGE);
        window_->scrollToBottom();
    }

    void centralNotification(const QString &text, const QString &user, int timestamp)
    {
        //qDebug() << "notification (from qccserver) (timestamp:" << timestamp << "):" << text;
        cchannels_->sendNotification(text, user, timestamp);
        window_->appendEvent(text, user, timestamp, NOTIFICATION);
        window_->scrollToBottom();
    }

    void history(const QStringList &h)
    {
        //qDebug() << "history (from qccserver):" << h;
        window_->prependHistory(h);
    }

    void showChatWindow()
    {
        cchannels_->showChatWindow();
    }

    void hideChatWindow()
    {
        cchannels_->hideChatWindow();
    }

    void localChatMessage(const QString &text)
    {
        //qDebug() << "chat message (from local window):" << text;
        schannel_->sendChatMessage(text, user_);
    }
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // extract information from environment
    bool ok;
    const quint16 qclport = qgetenv("QCLPORT").toUInt(&ok);
    if (!ok) {
        qDebug("failed to extract int from environment variable QCLPORT");
        return 1;
    }
    const QString qcchost = qgetenv("QCCHOST");
    if (qcchost.isEmpty()) {
        qDebug("failed to extract string from environment variable QCCHOST");
        return 1;
    }
    const quint16 qccport = qgetenv("QCCPORT").toUInt(&ok);
    if (!ok) {
        qDebug("failed to extract int from environment variable QCCPORT");
        return 1;
    }

    // listen for incoming qcapp connections
    QCClientChannels cchannels;
    if (!cchannels.listen(qclport)) {
        qDebug(
            "failed to listen for incoming qcapp connections: cchannels.listen() failed: %s",
            cchannels.lastError().toLatin1().data());
        return 1;
    }

    // establish channel to qccserver
    QCServerChannel schannel;
    if (!schannel.connectToServer(qcchost, qccport)) {
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
