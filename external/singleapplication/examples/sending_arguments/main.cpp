#include <singleapplication.h>
#include "messagereceiver.h"

int main(int argc, char *argv[])
{
    // Allow secondary instances
    SingleApplication app( argc, argv, true );

    MessageReceiver msgReceiver;

    // If this is a secondary instance
    if( app.isSecondary() ) {
        app.sendMessage( app.arguments().join(' ').toUtf8() );
        qDebug() << "App already running.";
        qDebug() << "Primary instance PID: " << app.primaryPid();
        qDebug() << "Primary instance user: " << app.primaryUser();
        return 0;
    } else {
        QObject::connect(
            &app,
            &SingleApplication::receivedMessage,
            &msgReceiver,
            &MessageReceiver::receivedMessage
        );
    }

    return app.exec();
}
