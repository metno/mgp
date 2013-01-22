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
        , channelCBox_(0)
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
        QLabel *label2 = new QLabel("Send notification:");
        label2->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        layout2->addWidget(label2);
        channelCBox_ = new QComboBox;
        channelCBox_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
        layout2->addWidget(channelCBox_);
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
        connect(schannel_, SIGNAL(channels(const QStringList &)), SLOT(channels(const QStringList &)));
        connect(schannel_, SIGNAL(chatWindowShown()), SLOT(showChatWindow()));
        connect(schannel_, SIGNAL(chatWindowHidden()), SLOT(hideChatWindow()));
        connect(
            schannel_, SIGNAL(notification(const QString &, const QString &, int, int)),
            SLOT(notification(const QString &, const QString &, int, int)));
    }

private:
    QCServerChannel *schannel_;
    QPushButton *showButton_;
    QPushButton *hideButton_;
    QComboBox *channelCBox_;
    QLineEdit *notifyEdit_;
    QMap<QString, int> channelId_;

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
        }
    }

private slots:
    void sendNotification()
    {
        const int channelId = channelId_.value(channelCBox_->currentText());
        schannel_->sendNotification(notifyEdit_->text(), QString(), channelId);
    }

    void serverDisconnected()
    {
        showButton_->setEnabled(false);
        hideButton_->setEnabled(false);
        channelCBox_->setEnabled(false);
        notifyEdit_->setEnabled(false);
    }

    void channels(const QStringList &chatChannels)
    {
        setChannels(chatChannels);
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

    void notification(const QString &text, const QString &user, int channelId, int timestamp)
    {
        qDebug() << QString("notification (user %1, channel %2, timestamp %3): %4")
            .arg(user).arg(channelId).arg(timestamp).arg(text).toLatin1().data();
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
