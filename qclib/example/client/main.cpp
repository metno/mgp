// Example client code.

#include <QtGui> // ### TODO: include relevant headers only
#include "qc.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCServerChannel schannel;
    if (!schannel.connectToServer("localhost", 1104)) // for now
        qFatal("schannel.connectToServer() failed: %s", schannel.lastError().toLatin1().data());

    // connect schannel signals ... 2 B DONE!
    schannel.openChatWindow(); // typically done by a button
    schannel.closeChatWindow(); // typically done by a button
    schannel.sendNotification("this is a notification ..."); // typically done by a line edit

    return app.exec();
}

//#include "main.moc"
