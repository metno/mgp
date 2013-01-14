// Example qclserver code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qc.h"

class ChatWindow : public QWidget
{
    Q_OBJECT
public:
    ChatWindow()
        : log(0)
        , edit(0)
    {
        QVBoxLayout *layout = new QVBoxLayout;

        QLabel *label = new QLabel("CHAT WINDOW MOCKUP");
        label->setStyleSheet("QLabel { background-color : yellow; color : black; }");
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        log = new QListWidget;
        layout->addWidget(log);

        edit = new QLineEdit;
        connect(edit, SIGNAL(returnPressed()), SLOT(sendChatMessage()));

        layout->addWidget(edit);

        setLayout(layout);
        resize(500, 300);
    }

    void appendChatMessage(const QString &msg)
    {
        qDebug() << QString("ChatWindow::appendChatMessage(%1) ... (2 B IMPLEMENTED)").arg(msg).toLatin1().data();
    }

    void prependHistory(const QStringList &h)
    {
        qDebug() << QString("ChatWindow::prependHistory(...) ... (2 B IMPLEMENTED)");
    }

private:
    QListWidget *log;
    QLineEdit *edit;

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

private slots:
    void sendChatMessage()
    {
        const QString msg = edit->text().trimmed();
        edit->clear();
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
            cchannels, SIGNAL(notification(const QString &)),
            SLOT(localNotification(const QString &)));

        connect(schannel, SIGNAL(chatMessage(const QString &)), SLOT(centralChatMessage(const QString &)));
        connect(schannel, SIGNAL(notification(const QString &)), SLOT(centralNotification(const QString &)));
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
        qDebug() << "notification (from a local qcapp):" << msg;
        schannel->sendNotification(msg);
    }

    void centralChatMessage(const QString &msg)
    {
        qDebug() << "chat message (from qccserver):" << msg;
        window->appendChatMessage(msg);
    }

    void centralNotification(const QString &msg)
    {
        qDebug() << "notification (from qccserver):" << msg;
        cchannels->sendNotification(msg);
    }

    void history(const QStringList &h)
    {
        qDebug() << "history (from qccserver):" << h;
        window->prependHistory(h);
        // *prepend* in chat window (i.e. in front of any individual chat messages that may have arrived
        // in the meantime!)
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
        qDebug() << "chat message (from local window):" << msg;
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
