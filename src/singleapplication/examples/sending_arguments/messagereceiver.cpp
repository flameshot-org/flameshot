#include <QDebug>
#include "messagereceiver.h"

MessageReceiver::MessageReceiver(QObject *parent) : QObject(parent)
{
}

void MessageReceiver::receivedMessage(int instanceId, QByteArray message)
{
    qDebug() << "Received message from instance: " << instanceId;
    qDebug() << "Message Text: " << message;
}
