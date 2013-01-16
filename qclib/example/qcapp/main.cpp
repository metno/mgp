// Example qcapp code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qc.h"

class Window : public QWidget
{
    Q_OBJECT
public:
    Window(QCServerChannel *schannel)
        : schannel_(schannel)
        , showButton_(0)
        , hideButton_(0)
        , notifyButton_(0)
        , notifyEdit_(0)
    {
        QVBoxLayout *layout = new QVBoxLayout;

        QLabel *label = new QLabel("CLIENT APPLICATION MOCKUP");
        label->setStyleSheet("QLabel { background-color : yellow; color : black; }");
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        showButton_ = new QPushButton("show chat window");
        showButton_->setEnabled(false);
        connect(showButton_, SIGNAL(clicked()), schannel_, SLOT(showChatWindow()));
        layout->addWidget(showButton_);

        hideButton_ = new QPushButton("hide chat window");
        hideButton_->setEnabled(false);
        connect(hideButton_, SIGNAL(clicked()), schannel_, SLOT(hideChatWindow()));
        layout->addWidget(hideButton_);

        QHBoxLayout *layout2 = new QHBoxLayout;
        notifyButton_ = new QPushButton("send notification:");
        connect(notifyButton_, SIGNAL(clicked()), SLOT(sendNotification()));
        layout2->addWidget(notifyButton_);
        notifyEdit_ = new QLineEdit();
        connect(notifyEdit_, SIGNAL(returnPressed()), SLOT(sendNotification()));
        layout2->addWidget(notifyEdit_);

        layout->addLayout(layout2);

        QPushButton *quitButton = new QPushButton("quit");
        connect(quitButton, SIGNAL(clicked()), qApp, SLOT(quit()));
        layout->addWidget(quitButton);

        setLayout(layout);
        setMinimumWidth(600);

        connect(schannel_, SIGNAL(serverDisconnected()), SLOT(serverDisconnected()));
        connect(schannel_, SIGNAL(chatWindowShown()), SLOT(showChatWindow()));
        connect(schannel_, SIGNAL(chatWindowHidden()), SLOT(hideChatWindow()));
        connect(
            schannel_, SIGNAL(notification(const QString &, const QString &, int)),
            SLOT(notification(const QString &, const QString &, int)));
    }

private:
    QCServerChannel *schannel_;
    QPushButton *showButton_;
    QPushButton *hideButton_;
    QPushButton *notifyButton_;
    QLineEdit *notifyEdit_;

private slots:
    void sendNotification()
    {
        schannel_->sendNotification(notifyEdit_->text());
    }

    void serverDisconnected()
    {
        showButton_->setEnabled(false);
        hideButton_->setEnabled(false);
        notifyButton_->setEnabled(false);
        notifyEdit_->setEnabled(false);
    }

    void showChatWindow()
    {
        showButton_->setEnabled(false);
        hideButton_->setEnabled(true);
    }

    void hideChatWindow()
    {
        showButton_->setEnabled(true);
        hideButton_->setEnabled(false);
    }

    void notification(const QString &text, const QString &user, int timestamp)
    {
        qDebug() << QString("notification (user %1, timestamp %2): %3")
            .arg(user).arg(timestamp).arg(text).toLatin1().data();
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
