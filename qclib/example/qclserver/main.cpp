// Example qclserver code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qc.h"

// ### Define these in a central place:
#define CHATMESSAGE 0
#define NOTIFICATION 1

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

    void appendEvent(int type, int timestamp, const QString &text)
    {
        log_->addItem(formatEvent(type, timestamp, text));
        log_->scrollToBottom();
    }

    // Prepends history (i.e. in front of any individual chat messages that may have arrived
    // in the meantime!)
    void prependHistory(const QStringList &h)
    {
        QRegExp rx("^(\\d+)\\s+(\\d+)\\s+(.*)$");
        QStringList events;
        QListIterator<QString> it(h);
        while (it.hasNext()) {
            QString item = it.next();
            if (rx.indexIn(item) == -1)
                qFatal("regexp mismatch: %s", item.toLatin1().data());
            const int type = rx.cap(1).toInt();
            const int timestamp = rx.cap(2).toInt();
            const QString text = rx.cap(3);
            events << formatEvent(type, timestamp, text);
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

    QString formatEvent(int type, int timestamp, const QString &text) const
    {
        return QString("[%1] [%2]  %3").arg(type).arg(timestamp).arg(text);
    }

    void prependFormattedEvents(const QStringList &events)
    {
        log_->insertItems(0, events);
        log_->scrollToBottom();
    }

private slots:
    void sendChatMessage()
    {
        const QString msg = edit_->text().trimmed();
        edit_->clear();
        if (!msg.isEmpty())
            emit chatMessage(msg);
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
        : cchannels(cchannels)
        , schannel(schannel)
        , window(window)
    {
        connect(cchannels, SIGNAL(clientConnected(qint64)), SLOT(clientConnected(qint64)));
        connect(cchannels, SIGNAL(chatWindowShown()), window, SLOT(show()));
        connect(cchannels, SIGNAL(chatWindowHidden()), window, SLOT(hide()));
        connect(
            cchannels, SIGNAL(notification(const QString &, int)),
            SLOT(localNotification(const QString &)));

        connect(
            schannel, SIGNAL(chatMessage(const QString &, int)), SLOT(centralChatMessage(const QString &, int)));
        connect(
            schannel, SIGNAL(notification(const QString &, int)), SLOT(centralNotification(const QString &, int)));
        connect(schannel, SIGNAL(history(const QStringList &)), SLOT(history(const QStringList &)));

        connect(window, SIGNAL(windowShown()), SLOT(showChatWindow()));
        connect(window, SIGNAL(windowHidden()), SLOT(hideChatWindow()));
        connect(window, SIGNAL(chatMessage(const QString &)), SLOT(localChatMessage(const QString &)));

        schannel->sendHistoryRequest();
    }

private:
    QCClientChannels *cchannels; // qcapp channels
    QCServerChannel *schannel; // qccserver channel
    ChatWindow *window;

private slots:
    void clientConnected(qint64 id)
    {
        // inform about current chat window visibility
        if (window->isVisible())
            cchannels->showChatWindow(id);
        else
            cchannels->hideChatWindow(id);
    }

    void localNotification(const QString &msg)
    {
        //qDebug() << "notification (from a local qcapp):" << msg;
        schannel->sendNotification(msg);
    }

    void centralChatMessage(const QString &msg, int timestamp)
    {
        //qDebug() << "chat message (from qccserver) (timestamp:" << timestamp << "):" << msg;
        window->appendEvent(CHATMESSAGE, timestamp, msg);
        window->scrollToBottom();
    }

    void centralNotification(const QString &msg, int timestamp)
    {
        //qDebug() << "notification (from qccserver) (timestamp:" << timestamp << "):" << msg;
        cchannels->sendNotification(msg, timestamp);
        window->appendEvent(NOTIFICATION, timestamp, msg);
        window->scrollToBottom();
    }

    void history(const QStringList &h)
    {
        //qDebug() << "history (from qccserver):" << h;
        window->prependHistory(h);
    }

    void showChatWindow()
    {
        cchannels->showChatWindow();
    }

    void hideChatWindow()
    {
        cchannels->hideChatWindow();
    }

    void localChatMessage(const QString &msg)
    {
        //qDebug() << "chat message (from local window):" << msg;
        schannel->sendChatMessage(msg);
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
