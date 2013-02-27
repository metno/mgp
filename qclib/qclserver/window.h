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
    void setUsers(const QStringList &, const QStringList &, const QList<bool> &, const QList<int> &);
    void handleCentralWindowVisibility(const QString &, const QString &, bool);
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
    QMap<QString, QString> serverSysInfo_;
    QMap<QString, QString> userFullName_;
    QMap<QPair<QString, QString>, bool> winVisible_; // window visibility for each (user, IP-address)-combination

    bool eventFilter(QObject *, QEvent *);
    void updateUserTree();
    void updateWindowTitle();
public slots:
    void openFullNameDialog();
private slots:
    void sendChatMessage();
    void handleChannelSwitch();
    void hover(QMouseEvent *);
    void resizeUserTreeColumns();
signals:
    void chatMessage(const QString &, int);
    void channelSwitch(int);
    void fullNameChange(const QString &);
};

class ChatMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    ChatMainWindow(ChatWindow *);
private:
    bool geometrySaveEnabled_;
    void saveGeometry();
    bool restoreGeometry();
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
private slots:
    void enableGeometrySave();
    void showAbout();
signals:
    void windowVisibility(bool);
};

#endif // WINDOW_H
