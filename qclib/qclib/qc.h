#ifndef QC_H
#define QC_H

#include <QtCore> // ### TODO: include relevant headers only
#include <QtNetwork> // ### TODO: include relevant headers only

// Returns a map of (option, value) pairs extracted from a flat list of command-line arguments.
QMap<QString, QString> getOptions(const QStringList &);

// ### Warning: This class is not reentrant!
class QCChannel : public QObject
{
    Q_OBJECT
public:
    QCChannel(QTcpSocket * = 0);
    virtual ~QCChannel();
    qint64 id() const { return id_; }
    QString peerInfo() const { return peerInfo_; }
    bool connectToServer(const QString &, const quint16);
    void close();
    bool isConnected() const;
    void sendMessage(const QVariantMap &);
    QString lastError() const;
private:
    qint64 id_;
    static qint64 nextId_;
    QByteArray msgbuf_;
    QTcpSocket *socket_;
    QString peerInfo_;
    QString lastError_;
    void initSocket();
    void setLastError(const QString &);
signals:
    void error(const QString &);
    void socketDisconnected();
    void messageArrived(qint64, const QVariantMap &);
private slots:
    void readyRead();
    void handleSocketError(QAbstractSocket::SocketError);
};

class QCChannelServer : public QObject
{
    Q_OBJECT
public:
    QCChannelServer();
    bool listen(const qint16 port);
    QString lastError() const;
private:
    QTcpServer server_;
    void setLastError(const QString &);
    QString lastError_;
private slots:
    void newConnection();
signals:
    void channelConnected(QCChannel *);
};

#endif // QC_H
