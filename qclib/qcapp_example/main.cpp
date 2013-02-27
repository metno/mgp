// Example qcapp code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qcchat.h"
#include "qcglobal.h"

using namespace qclib;

class Window : public QWidget
{
    Q_OBJECT
#define TIMEOUT 3000 // server init timeout in milliseconds
public:
    Window(QCLocalServerChannel *schannel, const QString &chost, quint16 cport)
        : schannel_(schannel)
        , chost_(chost)
        , cport_(cport)
        , initialized_(false)
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
        connect(showButton_, SIGNAL(clicked()), schannel_, SLOT(sendShowWindow()));
        layout->addWidget(showButton_);

        hideButton_ = new QPushButton("hide chat window");
        hideButton_->setEnabled(false);
        connect(hideButton_, SIGNAL(clicked()), schannel_, SLOT(sendHideWindow()));
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

        // await init message or timeout
        connect(schannel, SIGNAL(init(const QVariantMap &, qint64)), SLOT(init(const QVariantMap &)));
        QTimer::singleShot(TIMEOUT, this, SLOT(serverInitTimeout()));
    }

private:
    QCLocalServerChannel *schannel_;
    QString chost_;
    quint16 cport_;
    bool initialized_;
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
    void serverInitTimeout()
    {
        if (!initialized_) {
            qDebug("ERROR: no response from server within %d milliseconds; terminating", TIMEOUT);
            qApp->exit(1);
        } else {
            // qDebug("already initialized");
        }
    }

    void init(const QVariantMap &msg)
    {
        initialized_ = true;

        if (msg.value("chost").toString() != chost_) {
            qDebug(
                "ERROR: central host mismatch: %s != %s", msg.value("chost").toString().toLatin1().data(),
                chost_.toLatin1().data());
            qApp->exit(1);
            return;
        }
        bool ok;
        const quint16 cport = msg.value("cport").toInt(&ok);
        if (!ok) {
            qDebug("ERROR: central port not an integer");
            qApp->exit(1);
            return;
        }
        if (cport != cport_) {
            qDebug("ERROR: central port mismatch: %d != %d",  cport, cport_);
            qApp->exit(1);
            return;
        }

        // chost and cport both match so, so enter normal operation
        setChannels(msg.value("channels").toStringList());
        windowVisibility(msg.value("windowvisible").toBool());
        connect(
            schannel_, SIGNAL(windowVisibility(bool, const QString &, const QString &, qint64)),
            SLOT(windowVisibility(bool)));
        connect(
            schannel_, SIGNAL(notification(const QString &, const QString &, int, int)),
            SLOT(notification(const QString &, const QString &, int, int)));
        connect(schannel_, SIGNAL(serverDisconnected()), SLOT(serverDisconnected()));
    }

    void sendNotification()
    {
        const int channelId = channelId_.value(channelCBox_->currentText());
        schannel_->sendNotification(notifyEdit_->text(), QString(), channelId);
    }

    void windowVisibility(bool visible)
    {
        showButton_->setEnabled(!visible);
        hideButton_->setEnabled(visible);
    }

    void notification(const QString &text, const QString &user, int channelId, int timestamp)
    {
        qDebug() << QString("notification (user %1, channel %2, timestamp %3): %4")
            .arg(user).arg(channelId).arg(timestamp).arg(text).toLatin1().data();
    }

    void serverDisconnected()
    {
        showButton_->setEnabled(false);
        hideButton_->setEnabled(false);
        channelCBox_->setEnabled(false);
        notifyEdit_->setEnabled(false);
    }
};

static void printUsage()
{
    qDebug() << QString(
        "usage: %1 --chost <central server host> --cport <central server port>")
        .arg(qApp->arguments().first()).toLatin1().data();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // extract command-line options
    const QMap<QString, QString> options = getOptions(app.arguments());
    const QString chost = options.value("chost");
    if (chost.isEmpty()) {
        qDebug("failed to extract central server host");
        printUsage();
        return 1;
    }
    bool ok;
    const quint16 cport = options.value("cport").toUInt(&ok);
    if (!ok) {
        qDebug("failed to extract central server port");
        printUsage();
        return 1;
    }

    // establish channel to qclserver
    QCLocalServerChannel schannel;
    if (!schannel.connectToServer(chost, cport)) {
        qDebug("schannel.connectToServer() failed: %s", schannel.lastError().toLatin1().data());
        return 1;
    }

    // create a dummy app window
    Window window(&schannel, chost, cport);
    window.show();
    return app.exec();
}

#include "main.moc"
