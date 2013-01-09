// Example client code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qc.h"

class Window : public QWidget
{
    Q_OBJECT
public:
    Window(QCServerChannel *schannel)
        : schannel(schannel)
    {
        QVBoxLayout *layout = new QVBoxLayout;

        showButton = new QPushButton("show chat window");
        showButton->setEnabled(false);
        connect(showButton, SIGNAL(clicked()), schannel, SLOT(showChatWindow()));
        layout->addWidget(showButton);

        hideButton = new QPushButton("hide chat window");
        hideButton->setEnabled(false);
        connect(hideButton, SIGNAL(clicked()), schannel, SLOT(hideChatWindow()));
        layout->addWidget(hideButton);

        QHBoxLayout *layout2 = new QHBoxLayout;
        notifyButton = new QPushButton("send notification:");
        connect(notifyButton, SIGNAL(clicked()), SLOT(sendNotification()));
        layout2->addWidget(notifyButton);
        notifyEdit = new QLineEdit();
        connect(notifyEdit, SIGNAL(returnPressed()), SLOT(sendNotification()));
        layout2->addWidget(notifyEdit);

        layout->addLayout(layout2);

        QPushButton *quitButton = new QPushButton("quit");
        connect(quitButton, SIGNAL(clicked()), qApp, SLOT(quit()));
        layout->addWidget(quitButton);

        setLayout(layout);
        setMinimumWidth(600);

        connect(schannel, SIGNAL(serverDisconnected()), SLOT(serverDisconnected()));
        connect(schannel, SIGNAL(chatWindowShown()), SLOT(chatWindowShown()));
        connect(schannel, SIGNAL(chatWindowHidden()), SLOT(chatWindowHidden()));
        connect(schannel, SIGNAL(notification(const QString &)), SLOT(notification(const QString &)));
    }

private:
    QCServerChannel *schannel;
    QPushButton *showButton;
    QPushButton *hideButton;
    QPushButton *notifyButton;
    QLineEdit *notifyEdit;

private slots:
    void sendNotification()
    {
        schannel->sendNotification(notifyEdit->text());
    }

    void serverDisconnected()
    {
        showButton->setEnabled(false);
        hideButton->setEnabled(false);
        notifyButton->setEnabled(false);
        notifyEdit->setEnabled(false);
    }

    void chatWindowShown()
    {
        showButton->setEnabled(false);
        hideButton->setEnabled(true);
    }

    void chatWindowHidden()
    {
        showButton->setEnabled(true);
        hideButton->setEnabled(false);
    }

    void notification(const QString &msg)
    {
        qDebug() << "notification:" << msg;
    }
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // establish the server channel
    QCServerChannel schannel;
    if (!schannel.connectToServer("localhost", 1104)) // hardcode address and port number for now
        qFatal("schannel.connectToServer() failed: %s", schannel.lastError().toLatin1().data());

    // create a dummy app window
    Window window(&schannel);
    window.show();
    return app.exec();
}

#include "main.moc"
