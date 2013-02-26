#include "interactor.h"

Interactor::Interactor(const QString &user, const QString &chost, quint16 cport)
    : user_(user)
    , chost_(chost)
    , cport_(cport)
    , showingWindow_(false)
{
    cchannels_.reset(new QCLocalClientChannels);
    schannel_.reset(new QCTcpServerChannel);
    window_.reset(new ChatWindow);
}

bool Interactor::initialize()
{
    // initialize interaction with chat window
    connect(window_.data(), SIGNAL(windowShown()), SLOT(windowShown()));
    connect(window_.data(), SIGNAL(windowHidden()), SLOT(windowHidden()));
    connect(window_.data(), SIGNAL(chatMessage(const QString &, int)), SLOT(localChatMessage(const QString &, int)));
    connect(window_.data(), SIGNAL(channelSwitch(int)), SLOT(handleChannelSwitch(int)));
    connect(window_.data(), SIGNAL(fullNameChange(const QString &)), SLOT(handleFullNameChange(const QString &)));
    window_->setUser(user_);

    // initialize interaction with qcapps
    connect(cchannels_.data(), SIGNAL(serverFileChanged(const QString &)), SLOT(serverFileChanged(const QString &)));
    connect(cchannels_.data(), SIGNAL(clientConnected(qint64)), SLOT(clientConnected(qint64)));
    connect(cchannels_.data(), SIGNAL(showChatWindow()), SLOT(showChatWindow()));
    connect(cchannels_.data(), SIGNAL(hideChatWindow()), SLOT(hideChatWindow()));
    connect(
        cchannels_.data(), SIGNAL(notification(const QString &, const QString &, int, int)),
        SLOT(localNotification(const QString &, const QString &, int)));
    if (!cchannels_->listen()) {
        Logger::instance().logError(
            QString("failed to create listen path: %1").arg(cchannels_->lastError().toLatin1().data()));
        return false;
    }

    // initialize interaction with qccserver
    connect(schannel_.data(), SIGNAL(init(const QVariantMap &, qint64)), SLOT(init(const QVariantMap &)));
    connect(schannel_.data(), SIGNAL(serverDisconnected()), SLOT(serverDisconnected()));
    connect(
        schannel_.data(), SIGNAL(chatMessage(const QString &, const QString &, int, int)),
        SLOT(centralChatMessage(const QString &, const QString &, int, int)));
    connect(
        schannel_.data(), SIGNAL(notification(const QString &, const QString &, int, int)),
        SLOT(centralNotification(const QString &, const QString &, int, int)));
    connect(
        schannel_.data(), SIGNAL(channelSwitch(int, const QString &, const QString &, qint64)),
        SLOT(centralChannelSwitch(int, const QString &, const QString &)));
    connect(
        schannel_.data(), SIGNAL(fullNameChange(const QString &, const QString &, qint64)),
        SLOT(centralFullNameChange(const QString &, const QString &)));
    connect(
        schannel_.data(), SIGNAL(errorMessage(const QString &, qint64)),
        SLOT(centralErrorMessage(const QString &)));
    connect(
        schannel_.data(), SIGNAL(users(const QStringList &, const QStringList &, const QList<int> &)),
        SLOT(users(const QStringList &, const QStringList &, const QList<int> &)));
    if (!schannel_->connectToServer(chost_, cport_)) {
        Logger::instance().logError(
            QString("failed to connect to qccserver: connectToServer() failed: %1")
            .arg(schannel_->lastError().toLatin1().data()));
        return false;
    }
    QVariantMap msg;
    msg.insert("user", user_);
    bool ok;
    const QString ipaddr = getLocalIPAddress(&ok);
    if (!ok) {
        Logger::instance().logError(QString("failed to get local IP address: %1").arg(ipaddr.toLatin1().data()));
        return false;
    }
    msg.insert("ipaddr", ipaddr);
    schannel_->sendInit(msg);

    return true;
}

void Interactor::init(const QVariantMap &msg)
{
    window_->setServerSysInfo(
        msg.value("hostname").toString(), msg.value("domainname").toString(), msg.value("ipaddr").toString());
    chatChannels_ = msg.value("channels").toStringList();
    window_->setChannels(chatChannels_);
    window_->setFullNames(msg.value("fullnames").toStringList());
    window_->prependHistory(msg.value("history").toStringList());
}

void Interactor::serverDisconnected()
{
    Logger::instance().logWarning("central server disconnected");
    qApp->exit(1);
}

void Interactor::serverFileChanged(const QString &serverPath)
{
    Logger::instance().logError(
        QString("local server file modified or (re)moved: %1").arg(serverPath.toLatin1().data()));
    qApp->exit(1);
}

void Interactor::clientConnected(qint64 qcapp)
{
    if (!schannel_->isConnected()) {
        Logger::instance().logWarning("central server not connected; disconnecting client");
        cchannels_->close(qcapp);
        return;
    }

    // send init message to this qcapp only
    QVariantMap msg;
    msg.insert("chost", chost_);
    msg.insert("cport", cport_);
    msg.insert("channels", chatChannels_);
    msg.insert("windowshown", window_->isVisible());
    cchannels_->sendInit(msg, qcapp);
}

void Interactor::localNotification(const QString &text, const QString &, int channelId)
{
    schannel_->sendNotification(text, user_, channelId);
}

void Interactor::centralChatMessage(const QString &text, const QString &user, int channelId, int timestamp)
{
    window_->appendEvent(text, user, channelId, timestamp, CHATMESSAGE);
    window_->scrollToBottom();
}

void Interactor::centralNotification(const QString &text, const QString &user, int channelId, int timestamp)
{
    cchannels_->sendNotification(text, user, channelId, timestamp);
    window_->appendEvent(text, user, channelId, timestamp, NOTIFICATION);
    window_->scrollToBottom();
}

void Interactor::centralChannelSwitch(int channelId, const QString &user, const QString &ipaddr)
{
    window_->handleCentralChannelSwitch(user, ipaddr, channelId);
}

void Interactor::centralFullNameChange(const QString &fullName, const QString &user)
{
    window_->handleCentralFullNameChange(user, fullName);
}

void Interactor::centralErrorMessage(const QString &msg)
{
    window_->handleCentralErrorMessage(msg);
}

void Interactor::users(const QStringList &u, const QStringList &ipaddrs, const QList<int> &channelIds)
{
    window_->setUsers(u, ipaddrs, channelIds);
}

void Interactor::sendShowChatWindow()
{
    cchannels_->sendShowChatWindow();
}

void Interactor::sendHideChatWindow()
{
    cchannels_->sendHideChatWindow();
}

// invoked from window_->showEvent()
void Interactor::windowShown()
{
    // inform clients right after other events have been processed
    QTimer::singleShot(0, this, SLOT(sendShowChatWindow()));
    showingWindow_ = false;
}

// invoked from client request
void Interactor::showChatWindow()
{
    if (showingWindow_)
        return; // let the active operation complete
    showingWindow_ = true;

    if (window_->isVisible()) {
        window_->hide();
        qApp->processEvents();
        QTimer::singleShot(500, window_.data(), SLOT(show()));
    } else {
        window_->show();
    }
}

// invoked from window_->hideEvent()
void Interactor::windowHidden()
{
    // inform clients right after other events have been processed
    QTimer::singleShot(0, this, SLOT(sendHideChatWindow()));
}

// invoked from client request
void Interactor::hideChatWindow()
{
    window_->hide();
}

void Interactor::localChatMessage(const QString &text, int channelId)
{
    schannel_->sendChatMessage(text, user_, channelId);
}

void Interactor::handleChannelSwitch(int channelId)
{
    schannel_->sendChannelSwitch(channelId);
}

void Interactor::handleFullNameChange(const QString &fullName)
{
    schannel_->sendFullNameChange(fullName);
}
