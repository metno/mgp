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
    {

        QVBoxLayout *layout = new QVBoxLayout;

        QLabel *label = new QLabel("CHAT WINDOW MOCKUP");
        label->setStyleSheet("QLabel { background-color : yellow; color : black; }");
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        log_ = new QListWidget;
        log_->show(); // to ensure scrollToBottom() has effect initially
        layout->addWidget(log_);

        edit_ = new QLineEdit;
        connect(edit_, SIGNAL(returnPressed()), SLOT(sendChatMessage()));

        layout->addWidget(edit_);

        setLayout(layout);
        resize(500, 300);
    }

    void appendEvent(const QString &text, const QString &user, int timestamp, int type)
    {
        log_->addItem(formatEvent(text, user, timestamp, type));
        log_->scrollToBottom();
    }

    // Prepends history (i.e. in front of any individual chat messages that may have arrived
    // in the meantime!)
    void prependHistory(const QStringList &h)
    {
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

        prependFormattedEvents(events);
    }

    void scrollToBottom()
    {
        log_->scrollToBottom();
    }

private:
    QListWidget *log_;
    QLineEdit *edit_;

    void closeEvent(QCloseEvent *event)
    {
        // prevent the close event from terminating the application
        hide();
        event->ignore();
    }

    void showEvent(QShowEvent *)
    {
        emit windowShown();
    }

    void hideEvent(QHideEvent *)
    {
        emit windowHidden();
    }

    QString formatEvent(const QString &text, const QString &user, int timestamp, int type) const
    {
        return QString("[%1] [%2] [%3]  %4").arg(type).arg(timestamp).arg(user).arg(text);
    }

    void prependFormattedEvents(const QStringList &events)
    {
        log_->insertItems(0, events);
        log_->scrollToBottom();
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
