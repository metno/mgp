// Example server code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qc.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCClientChannels cchannels;
    if (!cchannels.listen(1104)) // for now
        qFatal("cchannels.listen() failed: %s", cchannels.lastError().toLatin1().data());

    // connect cchannels signals ... 2 B DONE!
    cchannels.notifyChatWindowOpened();
    cchannels.notifyChatWindowClosed();
    cchannels.sendNotification("this is a notification ...");

    return app.exec();
}

//#include "main.moc"
