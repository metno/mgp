#ifndef WINDOW_H
#define WINDOW_H

#include <QtGui> // ### TODO: include relevant headers only

class HoverableTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    HoverableTextBrowser();
private:
    virtual void mouseMoveEvent(QMouseEvent *);
signals:
    void hover(QMouseEvent *);
};

class ChatWindow : public QWidget
{
    Q_OBJECT
public:
    ChatWindow();
    void appendEvent(const QString &, const QString &, int, int, int);
    void setChannels(const QStringList &);
    void setFullNames(const QStringList &);
    void setUser(const QString &);
    void prependHistory(const QStringList &);
    void setUsers(const QStringList &, const QStringList &, const QList<int> &);
    void handleCentralChannelSwitch(const QString &, const QString &, int);
    void handleCentralFullNameChange(const QString &, const QString &);
    void handleCentralErrorMessage(const QString &);
    void scrollToBottom();
    int currentChannelId() const;
    void setServerSysInfo(const QString &, const QString &, const QString &);
private:
    QComboBox *channelCBox_;
    QMap<QString, int> channelId_;
    QMap<int, QString> channelName_;
    QTreeWidget *userTree_;
    QMap<int, QTextBrowser *> log_;
    QStackedLayout logStack_;
    QLabel *userLabel_;
    QLineEdit *edit_;
    QMap<int, QString> html_;
    // (user, IP-address)-combinations associated with each chat channel:
    QMap<int, QSet<QPair<QString, QString> > *> channelUsers_;
    bool geometrySaveEnabled_;
    QMap<QString, QString> serverSysInfo_;
    QMap<QString, QString> userFullName_;

    void saveGeometry();
    bool restoreGeometry();
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
    bool eventFilter(QObject *, QEvent *);
    void updateUserTree();
    void updateWindowTitle();
private slots:
    void sendChatMessage();
    void handleChannelSwitch();
    void enableGeometrySave();
    void hover(QMouseEvent *);
    void resizeUserTreeColumns();
signals:
    void windowShown();
    void windowHidden();
    void chatMessage(const QString &, int);
    void channelSwitch(int);
    void fullNameChange(const QString &);
};

#endif // WINDOW_H
