#include "systemnotification.h"
#include "src/utils/confighandler.h"
#include <QApplication>

#ifndef Q_OS_WIN
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#else
#endif
#include "src/core/controller.h"

#ifdef Q_OS_LINUX
SystemNotification::SystemNotification(QObject *parent) : QObject(parent) {
    m_interface = new QDBusInterface(QStringLiteral("org.freedesktop.Notifications"),
                                     QStringLiteral("/org/freedesktop/Notifications"),
                                     QStringLiteral("org.freedesktop.Notifications"),
                                     QDBusConnection::sessionBus(),
                                     this);
}
#else
SystemNotification::SystemNotification(QObject *parent) : QObject(parent) {

}
#endif

void SystemNotification::sendMessage(
        const QString &text,
        const QString &title,
        const int timeout)
{
    if(!ConfigHandler().desktopNotificationValue()) {
        return;
    }

#ifndef Q_OS_WIN
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
#else
    auto c = Controller::getInstance();
    c->sendTrayNotification(text, title, timeout);
#endif
}
