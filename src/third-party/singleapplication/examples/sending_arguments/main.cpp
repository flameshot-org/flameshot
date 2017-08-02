#include <SingleApplication.h>
#include "messagereceiver.h"

int main(int argc, char *argv[])
{
    // Allow secondary instances
    SingleApplication app( argc, argv, true );

    MessageReceiver msgReceiver;

    // If this is a secondary instance
    if( app.isSecondary() ) {
        app.sendMessage( app.arguments().join(' ').toUtf8() );
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
