// Example qclserver code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qc.h"

class ChatWindow : public QWidget
{
    Q_OBJECT
public:
    ChatWindow()
    {
        QVBoxLayout *layout = new QVBoxLayout;
        QLabel *label = new QLabel("DUMMY CHAT WINDOW");
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
        setLayout(layout);
        resize(500, 300);
    }

    void showChatMessage(const QString &msg)
    {
        qDebug() << QString("ChatWindow::showChatMessage(%1) ... (2 B IMPLEMENTED)").arg(msg).toLatin1().data();
    }

private:
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
        connect(cchannels, SIGNAL(clientConnected()), SLOT(clientConnected()));
        connect(cchannels, SIGNAL(chatWindowShown()), window, SLOT(show()));
        connect(cchannels, SIGNAL(chatWindowHidden()), window, SLOT(hide()));
        connect(
            cchannels, SIGNAL(notification(const QString &)),
            SLOT(localNotification(const QString &)));

        connect(schannel, SIGNAL(chatMessage(const QString &)), SLOT(centralChatMessage(const QString &)));
        connect(schannel, SIGNAL(notification(const QString &)), SLOT(centralNotification(const QString &)));

        connect(window, SIGNAL(windowShown()), SLOT(showChatWindow()));
        connect(window, SIGNAL(windowHidden()), SLOT(hideChatWindow()));
        connect(window, SIGNAL(chatMessage(const QString &)), SLOT(localChatMessage(const QString &)));
    }

private:
    QCClientChannels *cchannels; // qcapp channels
    QCServerChannel *schannel; // qccserver channel
    ChatWindow *window;

private slots:
    void clientConnected()
    {
        if (window->isVisible())
            cchannels->showChatWindow();
        else
            cchannels->hideChatWindow();
    }

    void localNotification(const QString &msg)
    {
        qDebug() << "notification (from a local qcapp):" << msg;
        schannel->sendNotification(msg);
    }

    void centralChatMessage(const QString &msg)
    {
        qDebug() << "chat message (from qccserver):" << msg;
        window->showChatMessage(msg);
    }

    void centralNotification(const QString &msg)
    {
        qDebug() << "notification (from qccserver):" << msg;
        cchannels->sendNotification(msg);
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
    const QString qcchost = qgetenv("QCCHOST").trimmed();
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
