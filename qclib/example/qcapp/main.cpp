// Example qcapp code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qc.h"

class Window : public QWidget
{
    Q_OBJECT
public:
    Window(QCServerChannel *schannel)
        : schannel(schannel)
        , showButton(0)
        , hideButton(0)
        , notifyButton(0)
        , notifyEdit(0)
    {
        QVBoxLayout *layout = new QVBoxLayout;

        QLabel *label = new QLabel("CLIENT APPLICATION MOCKUP");
        label->setStyleSheet("QLabel { background-color : yellow; color : black; }");
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

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
        connect(schannel, SIGNAL(chatWindowShown()), SLOT(showChatWindow()));
        connect(schannel, SIGNAL(chatWindowHidden()), SLOT(hideChatWindow()));
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

    void showChatWindow()
    {
        showButton->setEnabled(false);
        hideButton->setEnabled(true);
    }

    void hideChatWindow()
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

    // extract information environment
    bool ok;
    const quint16 port = qgetenv("QCLPORT").toUInt(&ok);
    if (!ok) {
        qDebug("failed to extract int from environment variable QCLPORT");
        return 1;
    }

    // establish channel to qclserver
    QCServerChannel schannel;
    if (!schannel.connectToServer("localhost", port)) {
        qDebug("schannel.connectToServer() failed: %s", schannel.lastError().toLatin1().data());
        return 1;
    }

    // create a dummy app window
    Window window(&schannel);
    window.show();
    return app.exec();
}

#include "main.moc"
