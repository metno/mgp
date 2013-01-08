// Example server code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qc.h"

class Window : public QWidget
{
    Q_OBJECT
public:
    Window()
    {
        QVBoxLayout *layout = new QVBoxLayout;
        QLabel *label = new QLabel("DUMMY CHAT WINDOW");
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
        setLayout(layout);
        resize(500, 300);
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
};

class ClientInteractor : public QObject
{
    Q_OBJECT
public:
    ClientInteractor(QCClientChannels *cchannels, Window *window)
        : cchannels(cchannels)
        , window(window)
    {
        connect(window, SIGNAL(windowShown()), SLOT(windowShown()));
        connect(window, SIGNAL(windowHidden()), SLOT(windowHidden()));

        connect(cchannels, SIGNAL(clientConnected()), SLOT(clientConnected()));
        connect(cchannels, SIGNAL(showChatWindowRequested()), window, SLOT(show()));
        connect(cchannels, SIGNAL(hideChatWindowRequested()), window, SLOT(hide()));
        connect(
            cchannels, SIGNAL(notificationRequested(const QString &)),
            SLOT(notificationRequested(const QString &)));
    }

private:
    QCClientChannels *cchannels;
    Window *window;

private slots:
    void clientConnected()
    {
        if (window->isVisible())
            cchannels->notifyChatWindowShown();
        else
            cchannels->notifyChatWindowHidden();
    }

    void windowShown()
    {
        cchannels->notifyChatWindowShown();
    }

    void windowHidden()
    {
        cchannels->notifyChatWindowHidden();
    }

    void notificationRequested(const QString &msg)
    {
        qDebug() << "notification requested:" << msg;
        cchannels->sendNotification(msg);
    }
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // listen for incoming client connections
    QCClientChannels cchannels;
    if (!cchannels.listen(1104)) // hardcode port number for now
        qFatal("cchannels.listen() failed: %s", cchannels.lastError().toLatin1().data());

    // create a dummy chat window
    Window *window = new Window;
    //window->show();

    // create object to handle interaction between clients and chat window
    ClientInteractor interactor(&cchannels, window);

    return app.exec();
}

#include "main.moc"
