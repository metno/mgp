#ifndef INTERACTOR_H
#define INTERACTOR_H

#include <QtGui> // ### TODO: include relevant headers only
#include "qcchat.h"
#include "qcglobal.h"
#include "window.h"

using namespace qclib;

class Interactor : public QObject
{
    Q_OBJECT
public:
    Interactor(const QString &, const QString &, quint16);
    bool initialize();
private:
    QString user_;
    QString chost_;
    quint16 cport_;
    QScopedPointer<QCLocalClientChannels> cchannels_; // qcapp channels
    QScopedPointer<QCTcpServerChannel> schannel_; // qccserver channel
    QScopedPointer<ChatMainWindow> mainWindow_;
    ChatWindow *window_;
    QStringList chatChannels_; //  a.k.a. chat rooms
    bool showingWindow_;
private slots:
    void init(const QVariantMap &);
    void serverDisconnected();
    void serverFileChanged(const QString &);
    void clientConnected(qint64);
    void localNotification(const QString &, const QString &, int);
    void centralChatMessage(const QString &, const QString &, int, int);
    void centralNotification(const QString &, const QString &, int, int);
    void centralChannelSwitch(int, const QString &, const QString &);
    void centralFullNameChange(const QString &, const QString &);
    void centralErrorMessage(const QString &);
    void users(const QStringList &, const QStringList &, const QList<int> &);
    void sendShowChatWindow();
    void sendHideChatWindow();
    void windowShown();
    void showChatWindow();
    void windowHidden();
    void hideChatWindow();
    void localChatMessage(const QString &, int);
    void handleChannelSwitch(int);
    void handleFullNameChange(const QString &);
};

#endif // INTERACTOR_H
