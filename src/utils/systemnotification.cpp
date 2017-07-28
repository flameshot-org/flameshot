#include "systemnotification.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QApplication>

SystemNotification::SystemNotification(QObject *parent) : QObject(parent) {
    m_interface = new QDBusInterface("org.freedesktop.Notifications",
                                     "/org/freedesktop/Notifications",
                                     "org.freedesktop.Notifications",
                                     QDBusConnection::sessionBus(),
                                     this);
}

void SystemNotification::sendMessage(
        const QString &text,
        const QString &title,
        const int timeout)
{
    QList<QVariant> args;
    args << (qAppName())                 //appname
         << static_cast<unsigned int>(0) //id
         << "flameshot.png"              //icon
         << title                        //summary
         << text                         //body
         << QStringList()                //actions
         << QVariantMap()                //hints
         << timeout;                     //timeout
    m_interface->callWithArgumentList(QDBus::AutoDetect, "Notify", args);
}
