#include <QtGui>
#include <QX11EmbedContainer>

QPushButton *startButton;
QPushButton *stopButton;
QLineEdit *message;


// Class to manage an external quasselclient process.
class QCManager : public QObject
{
Q_OBJECT

private:
    virtual void startProcess() = 0;
    virtual void stopProcess(bool processOnly = false) = 0;

protected:
    QProcess *process;

public:
    QCManager()
        : process(0)
    {
        qDebug() << "QCManager::QCManager() ...";
    }

    virtual ~QCManager()
    {
    }

    void init()
    {
        process = new QProcess;
        QObject::connect(process, SIGNAL(started()), this, SLOT(started()));
        QObject::connect(
            process, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(finished(int, QProcess::ExitStatus)));
        QObject::connect(
            process, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(error(QProcess::ProcessError)));
    }

private slots:
    void started() const
    {
        qDebug() << "process started, pid:" << process->pid();
        startButton->setEnabled(false);
        stopButton->setEnabled(true);
    }

    void finished(int exitCode, QProcess::ExitStatus exitStatus) const
    {
        qDebug() << "process finished, exit code:" << exitCode << ", exit status:" << exitStatus;
        qDebug() << "STDOUT:" << process->readAllStandardOutput();
        qDebug() << "STDERR:" << process->readAllStandardError();
        stopButton->setEnabled(false);
        startButton->setEnabled(true);
    }

    void error(QProcess::ProcessError error) const
    {
        qDebug() << "process failed, error code:" << error;
        qDebug() << "STDOUT:" << process->readAllStandardOutput();
        qDebug() << "STDERR:" << process->readAllStandardError();
        stopButton->setEnabled(false);
        startButton->setEnabled(true);
    }

public slots:
    void start()
    {
        if (!startButton->isEnabled()) {
            qDebug() << "already started!";
            return;
        } else {
            Q_ASSERT(!stopButton->isEnabled());
        }

        startProcess();
    }

    void stop()
    {
        if (!stopButton->isEnabled()) {
            qDebug() << "already stopped!";
            return;
        } else {
            Q_ASSERT(!startButton->isEnabled());
        }

        stopProcess();
    }

    void send()
    {
    }
};

class QCManager_separateWindow : public QCManager
{
private:
    ~QCManager_separateWindow()
    {
        if (process)
            stopProcess();
    }

protected:
    void startProcess()
    {
        qDebug() << "QCManager_separateWindow::startProcess() ...";
//        process->start("/home/joa/dev/joa/profet/quassel/quasselclient");
        process->start("/usr/bin/quasselclient");
    }

    void stopProcess(bool processOnly = false)
    {
        Q_UNUSED(processOnly);
        qDebug() << "QCManager_separateWindow::stopProcess() ...";
        process->terminate();
        // NOTE: In contrast to terminate(), the below functions both give
        // a QProcess::ExitStatus of 1 (but still an exit code of 0):
        //process->kill();
        //process->close();
    }
};

class QCManager_embedded : public QCManager
{
    Q_OBJECT
public:
    QCManager_embedded(QVBoxLayout *layout_)
        : layout(layout_)
        , container(new QX11EmbedContainer)
    {
        layout->addWidget(container);
        QObject::connect(container, SIGNAL(clientIsEmbedded()), this, SLOT(clientIsEmbedded()));
        QObject::connect(container, SIGNAL(clientClosed()), this, SLOT(clientClosed()));
    }

private:
    ~QCManager_embedded()
    {
        if (process)
            stopProcess(true);
    }

    QVBoxLayout *layout;
    QX11EmbedContainer *container;

private slots:
    void clientIsEmbedded() const
    {
        qDebug() << "clientIsEmbedded() ...";
    }
    void clientClosed() const
    {
        qDebug() << "clientClosed() ...";
    }


protected:
    void startProcess()
    {
        qDebug() << "QCManager_embedded::startProcess() ...";

        // Launch special version of quasselclient passing container window ID as argument.
        // Have quasselclient embed itself into the client using QX11EmbedWidget.embedInto(winID)
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("PECWINID", QString::number(container->winId()));
        process->setProcessEnvironment(env);
        process->start("/home/joa/dev/joa/profet/quassel/quasselclient");
//        process->start("xterm", QStringList() << "-into" << QString::number(container->winId()));
        container->show();

        // For now, embed an already existing quasselclient window
        // (got window ID from xwininfo):
        //container->embedClient(0x4e00003);

        // TODO: Modify quasselclient to turn it into a proper XEmbed widget
        // (otherwise focus handling doesn't work properly etc.)
        // (NOTE: Eventually we only bother to do this if we really want quasselclient to be embedded.
        // Also, we need to check if menus are available as expected for an embedded quasselclient).
    }

    void stopProcess(bool processOnly = false)
    {
        qDebug() << "QCManager_embedded::stopProcess() ...";

        if (!processOnly)
            container->hide();

        process->terminate();
        // // NOTE: In contrast to terminate(), the below functions both give
        // // a QProcess::ExitStatus of 1 (but still an exit code of 0):
        // //process->kill();
        // //process->close();
    }
};

int main(int argc, char *argv[])
{
    if (argc != 2) {
        qDebug() << "usage:" << argv[0] << "0|1 (0 = separate window, 1 = embedded)";
        exit(1);
    }

    const bool embedded = !QString("1").compare(argv[1]);

    QApplication app(argc, argv);

    QVBoxLayout *layout = new QVBoxLayout;

    QSharedPointer<QCManager> qcmgr = embedded
        ? QSharedPointer<QCManager>(new QCManager_embedded(layout))
        : QSharedPointer<QCManager>(new QCManager_separateWindow());
    qcmgr.data()->init();

    startButton = new QPushButton("start quasselclient");
    QObject::connect(startButton, SIGNAL(clicked()), qcmgr.data(), SLOT(start()));
    layout->addWidget(startButton);

    stopButton = new QPushButton("stop quasselclient");
    QObject::connect(stopButton, SIGNAL(clicked()), qcmgr.data(), SLOT(stop()));
    stopButton->setEnabled(false);
    layout->addWidget(stopButton);

    QPushButton *quitButton = new QPushButton("quit");
    QObject::connect(quitButton, SIGNAL(clicked()), &app, SLOT(quit()));
    layout->addWidget(quitButton);

    QHBoxLayout *layout2 = new QHBoxLayout;
    QPushButton *sendButton = new QPushButton("send to irc server");
    QObject::connect(sendButton, SIGNAL(clicked()), qcmgr.data(), SLOT(send()));
    layout2->addWidget(sendButton);
    message = new QLineEdit();
    layout2->addWidget(message);

    layout->addLayout(layout2);

    QWidget window;
    window.setLayout(layout);
    //window.resize(800, 200);
    window.show();

    return app.exec();
}

#include "main.moc"
